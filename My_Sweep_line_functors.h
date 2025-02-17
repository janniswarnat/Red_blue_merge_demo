// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2/Sweep_line_functors.h $
// $Id: Sweep_line_functors.h 41124 2007-12-08 10:56:13Z efif $
//
//
// Author(s)     : Tali Zvi <talizvi@post.tau.ac.il>
//                 Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Ron Wein <wein@post.tau.ac.il>
//                 Efi Fogel <efif@post.tau.ac.il>

#ifndef MY_CGAL_SWEEP_LINE_FUNCTORS_H
#define MY_CGAL_SWEEP_LINE_FUNCTORS_H

/*! \file
 * Comparison functors used by the sweep-line algorithm.
 */

CGAL_BEGIN_NAMESPACE

/*! \class
 * A functor used to compare events and event points in an xy-lexicographic
 * order. Used to maintain the order of the event queue (the X-structure)
 * in the sweep-line algorithm.
 */
template<class Traits_, class Event_>
  class My_Compare_events
  {
  public:

    typedef Traits_ Traits_2;
    typedef Event_ Event;

typedef    typename Traits_2::Point_2 Point_2;
    typedef typename Traits_2::X_monotone_curve_2 X_monotone_curve_2;
    //typedef typename Traits_2::Boundary_category Boundary_category;

  private:

    // Data members:
    Traits_2 *m_traits; // A geometric-traits object.
    Kernel kernel;

    Arr_parameter_space m_ps_in_x; // Storing curve information when
    Arr_parameter_space m_ps_in_y; // comparing a curve end with
    Arr_curve_end m_index; // boundary conditions.

  public:

    /*! Cosntructor. */
    My_Compare_events (Traits_2 *traits) :
    m_traits (traits)
      {}

    /*!
     * Compare two existing events.
     * This operator is called by the multiset assertions only in
     * debug mode (to verify that event was inserted at the right place).
     */
    Comparison_result operator()(const Event* e1, const Event* e2) const
      {
        const bool on_boundary1 = e1->is_on_boundary();
        const bool on_boundary2 = e2->is_on_boundary();

        if (! on_boundary1 && ! on_boundary2)
          {
            // Both events do not have boundary conditions - just compare the points.
            return (m_traits->compare_xy_2_object()(e1->point(), e2->point()));
          }

        if (! on_boundary1)
          {
            // Compare the point associated with the first event with the second
            // boundary event.
            return ( this->operator()(e1->point(), e2) );
          }

        if (! on_boundary2)
          {
            // Compare the point associated with the second event with the first
            // boundary event.
            return (CGAL::opposite(this->operator()(e2->point(), e1)));
          }

        return (_compare_curve_end_with_event (e1->curve(), _curve_end(e1),
                e1->parameter_space_in_x(),
                e1->parameter_space_in_y(),
                e2));
      }

    /*!
     * Compare a point, which should be inserted into the event queue,
     * with an existing event point.
     */
   Comparison_result operator() (const Point_2& pt, const Event* e2) const
      {
        const bool on_boundary2 = e2->is_on_boundary();

        if (! on_boundary2)
          {
            // If e2 is a normal event, just compare pt and the event point.
            return (m_traits->compare_xy_2_object() (pt, e2->point()));
          }

        // Get the sign of the event's boundary condition in x. Note that a valid
        // point is always larger than any negative boundary event and smaller
        // than any positive boundary event.
        Arr_parameter_space ps_x2 = e2->parameter_space_in_x();
        if (ps_x2 == ARR_LEFT_BOUNDARY)
        return (LARGER);
        else if (ps_x2 == ARR_RIGHT_BOUNDARY)
        return (SMALLER);

        // Get the curve end that e2 represents, and compare the x-position of the
        // given point and this curve end.
        Arr_curve_end ind = _curve_end(e2);
        Comparison_result res =
        m_traits->compare_x_point_curve_end_2_object()(pt, e2->curve(), ind);

        if (res != EQUAL)
        return (res);

        // The event and the point has the same x-position. Get the sign of the
        // event's boundary condition in y. Note that a valid point is always
        // larger than any negative boundary event and smaller than any positive
        // boundary event.
        Arr_parameter_space ps_y2 = e2->parameter_space_in_y();

        CGAL_assertion (ps_y2 != ARR_INTERIOR);
        return (ps_y2 == ARR_BOTTOM_BOUNDARY) ? LARGER : SMALLER;
      }

    /*!
     * Compare a curve end, which should be inserted into the event queue,
     * with an existing event point.
     * Note that the index of the curve end as well as its boundary conditions
     * must be set beforehand using set_index() and set_parameter_space_in_x/y().
     */
    Comparison_result operator() (const X_monotone_curve_2& cv,
        const Event* e2) const
      {
        return _compare_curve_end_with_event(cv, m_index, m_ps_in_x, m_ps_in_y, e2);
      }

    /// \name Set the boundary conditions of a curve end we are about to compare.
    //@{
    void set_parameter_space_in_x (Arr_parameter_space bx)
      {
        m_ps_in_x = bx;
      }

    void set_parameter_space_in_y (Arr_parameter_space by)
      {
        m_ps_in_y = by;
      }

    void set_index (Arr_curve_end ind)
      {
        m_index = ind;
      }
    //@}

  private:

    /*!
     * Compare a given curve end with an event.
     * \param cv The curve.
     * \param ind The curve end.
     * \param ps_x The boundary condition of the curve end in x.
     * \param ps_y The boundary condition of the curve end in y.
     * \param e2 The event, which may have boundary conditions.
     * \return The comparison result of the curve end with the event.
     */
    Comparison_result
    _compare_curve_end_with_event (const X_monotone_curve_2& cv,
        Arr_curve_end ind,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y,
        const Event* e2) const
      {
        // Check if the curve end has a boundary condition in x.
        if (ps_x == ARR_LEFT_BOUNDARY)
          {
            if (e2->parameter_space_in_x() == ARR_LEFT_BOUNDARY)
              {
                // Both defined on the left boundary - compare them there.
                CGAL_assertion (ind == ARR_MIN_END);

                return (m_traits->compare_y_curve_ends_2_object() (cv, e2->curve(),
                        ind));
              }

            // The curve end is obviously smaller.
            return (SMALLER);
          }

        if (ps_x == ARR_RIGHT_BOUNDARY)
          {
            if (e2->parameter_space_in_x() == ARR_RIGHT_BOUNDARY)
              {
                // Both defined on the right boundary - compare them there.
                CGAL_assertion (ind == ARR_MAX_END);

                return (m_traits->compare_y_curve_ends_2_object() (cv, e2->curve(),
                        ind));
              }

            // The curve end is obviously larger.
            return (LARGER);
          }

        // Check if the event has a boundary condition in x. Note that if it
        // has a negative boundary condition, the curve end is larger than it,
        // and if it has a positive boundary condition, the curve end is smaller.
        if (e2->parameter_space_in_x() == ARR_LEFT_BOUNDARY)
        return (LARGER);

        if (e2->parameter_space_in_x() == ARR_RIGHT_BOUNDARY)
        return (SMALLER);

        CGAL_assertion (ps_y != ARR_INTERIOR);
        Comparison_result res;

        Arr_curve_end ind2 = _curve_end(e2);

        // Act according to the boundary sign of the event.
        if (e2->parameter_space_in_y() == ARR_BOTTOM_BOUNDARY)
          {

            // Compare the x-positions of the two entities.
            res = m_traits->compare_x_curve_ends_2_object() (cv, ind,
      						       e2->curve(), ind2);

            if (res != EQUAL)
            return (res);

            // In case of equal x-positions, the curve end is larger than the event,
            // which lies on the bottom boundary (unless it also lies on the bottom
            // boundary).
            if (ps_y == ARR_BOTTOM_BOUNDARY)
            return (EQUAL);

            return (LARGER);
          }

        if (e2->parameter_space_in_y() == ARR_TOP_BOUNDARY)
          {

            // Compare the x-positions of the two entities.
            res = m_traits->compare_x_curve_ends_2_object() (cv, ind,
      						       e2->curve(), ind2);

            if (res != EQUAL)
            return (res);

            // In case of equal x-positions, the curve end is smaller than the event,
            // which lies on the top boundary (unless it also lies on the top
            // boundary).
            if (ps_y == ARR_TOP_BOUNDARY)
            return (EQUAL);

            return (SMALLER);
          }

        // If we reached here, e2 is not a boundary event and is associated with
        // a valid point. We compare the x-position of this point with the curve
        // end.
        res = m_traits->compare_x_point_curve_end_2_object() (e2->point(), cv, ind);

        if (res != EQUAL)
        return (CGAL::opposite(res));

        // In case of equal x-positions, is the curve end has a negative boundary
        // sign, then it lies on the bottom boundary below the event. Otherwise,
        // it lies on the top aboundary above the event e2.
        return (ps_y == ARR_BOTTOM_BOUNDARY) ? SMALLER : LARGER;
      }

    /*! Detemine if the given event represents a left or a right curve end. */
    // adapted by Jannis Warnat
    inline Arr_curve_end _curve_end (const Event* e) const
      {
        if(e->has_red_left_curves() || e->has_blue_left_curves())
          {
            if(e->is_right_end())
              {
                return ARR_MAX_END;
              }
            else
              {
                return ARR_MIN_END;
              }
          }
        else
          {
            if(e->is_left_end())
              {
                return ARR_MIN_END;
              }
            else
              {
                return ARR_MAX_END;
              }
          }

        // return (e->has_left_curves()) ?
        //   ((e->is_right_end()) ? ARR_MAX_END : ARR_MIN_END) :
        //   ((e->is_left_end()) ? ARR_MIN_END : ARR_MAX_END);
      }
  };

// Forward declaration:
template<class Traits, class Subcurve>
  class My_Sweep_line_event;

// A functor used to compare scouts along the status line, by Jannis Warnat

template<class Traits_, class Subcurve_, class Scout_>
  class Scout_comparer
  {
  public:

    typedef Traits_ Traits_2;
    typedef Subcurve_ Subcurve;
    typedef Scout_ Scout;
    typedef My_Sweep_line_event<Traits_2, Subcurve> Event;
    typedef Arr_traits_basic_adaptor_2<Traits_2> Traits_adaptor_2;

  private:

    Traits_adaptor_2 *m_traits; // A geometric-traits object.
    Event **m_curr_event; // Points to the current event point.
    typename Subcurve::Compare_curves comp_cv; //functor to compare subcurves

  public:

    /*! Constructor. */
    template <class Sweep_event>
    Scout_comparer (Traits_adaptor_2 *t, Sweep_event** e_ptr) :
    m_traits(t),
    m_curr_event(reinterpret_cast<Event**>(e_ptr)),
    comp_cv(t,e_ptr)

      {
      }

    // Compare the vertical position of two Scouts along the status line.
    Comparison_result operator()(Scout *c1, Scout *c2) const
      {

        // if a dummy scout is involved
        if(c1->type() == Scout::LOWER_DUMMY && c2->type() == Scout::LOWER_DUMMY)
          {
            return EQUAL;
          }
        if(c1->type() == Scout::UPPER_DUMMY && c2->type() == Scout::UPPER_DUMMY)
          {
            return EQUAL;
          }
        if(c1->type() == Scout::LOWER_DUMMY || c2->type() == Scout::UPPER_DUMMY)
          {
            return SMALLER;
          }
        if(c2->type() == Scout::LOWER_DUMMY || c1->type() == Scout::UPPER_DUMMY)
          {
            return LARGER;
          }

        // compare the boundary curves
        Comparison_result res = comp_cv(c1->boundary_curve(),c2->boundary_curve());

        if (res == EQUAL)
          {
            if(c1->type() == c2->type())
              {
                return EQUAL;
              }
            else
              {
                //a lower scout is larger than an upper scout on the same curve
                if(c1->type() == Scout::LOWER)
                  {
                    return LARGER;
                  }
                else
                  {
                    return SMALLER;
                  }
              }
          }
        return res;
      }
  };

/*! \class
 * A functor used to compare curves and curve endpoints by their vertical
 * y-order. Used to maintain the order of the status line (the Y-structure)
 * in the sweep-line algorithm.
 */
template<class Traits_, class Subcurve_>
  class My_Curve_comparer
  {
  public:

    typedef Traits_ Traits_2;
    typedef Subcurve_ Subcurve;
    typedef Arr_traits_basic_adaptor_2<Traits_2> Traits_adaptor_2;

typedef    typename Traits_2::Base_point_2 Base_point_2;
    typedef typename Traits_adaptor_2::Point_2 Point_2;
    typedef My_Sweep_line_event<Traits_2, Subcurve> Event;

  private:

    Traits_adaptor_2 *m_traits; // A geometric-traits object.
    Event **m_curr_event; // Points to the current event point.


  public:

    /*! Constructor. */
    template <class Sweep_event>
    My_Curve_comparer (Traits_adaptor_2 *t, Sweep_event** e_ptr) :
    m_traits(t),
    m_curr_event(reinterpret_cast<Event**>(e_ptr))
      {
      }

    /*!
     * (Compare the vertical position of two subcurves in the status line.
     * This operator is called only in debug mode.)
     * Adapted by Jannis Warnat
     * This operator is needed to compare the vertical position of
     * scouts along the status line. The curves may intersect.
     */
    Comparison_result operator()(const Subcurve *c1, const Subcurve *c2) const
      {
        //MY_CGAL_SL_PRINT("My_Curve_comparer operator()");
        //MY_CGAL_SL_PRINT("c1->last_curve() = " << c1->last_curve());
        //MY_CGAL_SL_PRINT("c2->last_curve() = " << c2->last_curve());
        //MY_CGAL_SL_PRINT("(*m_curr_event)->point() = " << (*m_curr_event)->point());

        // every geometric curve is wrapped in a unique subcurve object, so it
        // suffices to compare the pointers
        if(c1 == c2)
          {
            return EQUAL;
          }

        // get the base points of the curve endpoints
        Base_point_2 c1_left_base = c1->last_curve().base().left();
        Base_point_2 c2_left_base = c2->last_curve().base().left();
        Base_point_2 c1_right_base = c1->last_curve().base().right();
        Base_point_2 c2_right_base = c2->last_curve().base().right();

        // left endpoints are equal, compare to the right
        if(c1_left_base == c2_left_base)
          {
            Comparison_result res = m_traits->base_traits()->compare_y_at_x_right_2_object()(c1->last_curve().base(),c2->last_curve().base(),c1_left_base);
            return res;
          }

        // right endpoints are equal, compare to the left
        if(c1_right_base == c2_right_base)
          {
            Comparison_result res = m_traits->base_traits()->compare_y_at_x_left_2_object()(c1->last_curve().base(),c2->last_curve().base(),c1_right_base);
            return res;
          }

        Comparison_result c1_left_to_c2;
        Comparison_result c1_right_to_c2;

        // we want to avoid computing an intersection as long as possible;
        // compare the relative position of the endpoints of c1 to c2
        if(c2->last_curve().base().is_vertical())
          {
            if (c1_left_base.x() == c2->last_curve().base().left().x())
              {
                c1_left_to_c2 = EQUAL;
              }
            else
              {
                if(c1_left_base.x() < c2->last_curve().base().left().x())
                  {
                    c1_left_to_c2 = SMALLER;
                  }
                else
                  {
                    c1_left_to_c2 = LARGER;
                  }
              }

            if (c1_right_base.x() == c2->last_curve().base().left().x())
              {
                c1_right_to_c2 = EQUAL;
              }
            else
              {
                if(c1_right_base.x() < c2->last_curve().base().left().x())
                  {
                    c1_right_to_c2 = SMALLER;
                  }
                else
                  {
                    c1_right_to_c2 = LARGER;
                  }
              }
          }
        else
          {
            c1_left_to_c2 = m_traits->kernel.compare_y_at_x_2_object()(c1_left_base,c2->last_curve().base().line());
            c1_right_to_c2 = m_traits->kernel.compare_y_at_x_2_object()(c1_right_base,c2->last_curve().base().line());
          }

        // c1 is below c2
        if(c1_left_to_c2 == SMALLER && c1_right_to_c2 == SMALLER)
          {
            return SMALLER;
          }
        // c2 is below c1
        if(c1_left_to_c2 == LARGER && c1_right_to_c2 == LARGER)
          {
            return LARGER;
          }

        // compare the relative position of the endpoints of c2 to c1
        Comparison_result c2_left_to_c1;
        Comparison_result c2_right_to_c1;
        if(c1->last_curve().base().is_vertical())
          {
            if (c2_left_base.x() == c1->last_curve().base().left().x())
              {
                c2_left_to_c1 = EQUAL;
              }
            else
              {
                if(c2_left_base.x() < c1->last_curve().base().left().x())
                  {
                    c2_left_to_c1 = SMALLER;
                  }
                else
                  {
                    c2_left_to_c1 = LARGER;
                  }
              }

            if (c2_right_base.x() == c1->last_curve().base().left().x())
              {
                c2_right_to_c1 = EQUAL;
              }
            else
              {
                if(c2_right_base.x() < c1->last_curve().base().left().x())
                  {
                    c2_right_to_c1 = SMALLER;
                  }
                else
                  {
                    c2_right_to_c1 = LARGER;
                  }
              }
          }
        else
          {
            c2_left_to_c1 = m_traits->kernel.compare_y_at_x_2_object()(c2_left_base,c1->last_curve().base().line());
            c2_right_to_c1 = m_traits->kernel.compare_y_at_x_2_object()(c2_right_base,c1->last_curve().base().line());
          }

        // c2 is below c1
        if(c2_left_to_c1 == SMALLER && c2_right_to_c1 == SMALLER)
          {
            return LARGER;
          }
        // c1 is below c2
        if(c2_left_to_c1 == LARGER && c2_right_to_c1 == LARGER)
          {
            return SMALLER;
          }

        // now we have to compute the intersection point ip after all
        std::vector<Object> obj_vec;
        m_traits->base_traits()->intersect_2_object()(c1->last_curve().base(),
            c2->last_curve().base(),
            std::back_inserter(obj_vec));
        std::pair<Base_point_2, unsigned int> base_ipt;
        base_ipt = object_cast<std::pair<Base_point_2, unsigned int> >(obj_vec.front());
        Base_point_2 ip = base_ipt.first;

        // we need to compare the intersection point to the current event point
        Base_point_2 event_point = (*m_curr_event)->point().base();

        // compare to the left
        if(event_point < ip)
          {
            Comparison_result res = m_traits->base_traits()->compare_y_at_x_left_2_object()(c1->last_curve().base(),c2->last_curve().base(),ip);
            return res;
          }
        // compare to the right
        if(event_point> ip)
          {
            Comparison_result res = m_traits->base_traits()->compare_y_at_x_right_2_object()(c1->last_curve().base(),c2->last_curve().base(),ip);
            return res;
          }
      }

    /*!
     * Compare the relative y-order of the given point and the given subcurve.
     */
    Comparison_result operator() (const Point_2& pt, const Subcurve *sc) const
      {
        return (m_traits->compare_y_at_x_2_object()(pt, sc->last_curve()));
      }

  };

CGAL_END_NAMESPACE

#endif

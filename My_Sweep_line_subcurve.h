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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2/Sweep_line_subcurve.h $
// $Id: Sweep_line_subcurve.h 41325 2007-12-26 15:39:40Z golubevs $
//
//
// Author(s)     : Tali Zvi <talizvi@post.tau.ac.il>,
//                 Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Ron Wein <wein@post.tau.ac.il>

#ifndef MY_CGAL_SWEEP_LINE_SUBCURVE_H
#define MY_CGAL_SWEEP_LINE_SUBCURVE_H

/*! \file
 * Defintion of the Sweep_line_subcurve class.
 */

CGAL_BEGIN_NAMESPACE

/*! \class Sweep_line_subcurve
 *
 * This is a wrapper class to X_monotone_curve_2 in the traits class, that contains
 * data that is used when applying the sweep algorithm on a set of x-monotone curves.
 *
 * The information contained in this class is:
 * - the remaining x-monotone curve that is to the right of the current sweep line.
 * - two event points which are associated with the left and right end of the curve.
 * - an iterator that points to the location of the subcurve at the status line.
 * - two pointers to subcurves that are the originating subcurves in case of an overlap,
 *   otherwise thay are both NULL.
 *
 */

template<class Arrangement_, class Traits_>
  class My_Sweep_line_subcurve;

  template<class Arrangement_, class Traits_>
    class Scout;

template<class Arrangement_,
    class Traits_,
    ////class Visitor_,
    class Subcurve_ = My_Sweep_line_subcurve<Arrangement_, Traits_> ,
    typename Event_ = My_Sweep_line_event<Traits_, Subcurve_> ,
    typename Allocator_ = CGAL_ALLOCATOR
    (int)>
  class My_Basic_sweep_line_2;

template<class Arrangement_,
    class Traits_,
    //class Visitor_,
    class Subcurve_ = My_Sweep_line_subcurve<Arrangement_, Traits_> ,
    class Event_ = My_Sweep_line_event<Traits_, Subcurve_> ,
    typename Allocator_ = CGAL_ALLOCATOR
    (int)>
  class My_Sweep_line_2;

template<class Arrangement_, class Traits_>
  class My_Sweep_line_subcurve
  {
  public:

    typedef Traits_ Traits_2;
    typedef Arrangement_ Arrangement_2;
    typedef typename Traits_2::X_monotone_curve_2 X_monotone_curve_2;
    typedef My_Sweep_line_subcurve<Arrangement_2, Traits_2> Self;
    typedef Scout<Arrangement_2, Traits_2> My_Scout;
    typedef My_Curve_comparer<Traits_2, Self> Compare_curves;

    typedef Multiset<Self*,
    Compare_curves,
    CGAL_ALLOCATOR(int)> Status_line;
    typedef typename Status_line::iterator Status_line_iterator;

    typedef My_Sweep_line_event<Traits_2, Self> Event;

  protected:

    // Data members:

    //upper scout on the curve
    My_Scout* upper;
    //lower scout on the curve
    My_Scout* lower;
    //upper guard guarding the curve
    My_Scout* upper_g;
    //lower guard guarding the curve
    My_Scout* lower_g;

    //if true, the wrapped curve has already been inserted into
    //the purple arrangement
    bool in_purple;

    //the left part of the wrapped curve must be inserted into the
    //purple arrangement after the curve has been split
    //in _handle_..._left_curves()
    bool ins_sub_cv1;

    X_monotone_curve_2 m_lastCurve; // The portion of the curve that lies to
    // the right of the last event point
    // that occured on the curve.

    Event *m_left_event; // The event associated with the left end.
    Event *m_right_event; // The event associated with the right end

    Status_line_iterator m_hint; // The location of the subcurve in the
    // status line (the Y-structure).

    Self *m_orig_subcurve1; // The overlapping hierarchy
    Self *m_orig_subcurve2; // (relevant only in case of overlaps).


  public:

    /*! Default constructor. */
    My_Sweep_line_subcurve () :
    m_orig_subcurve1 (NULL),
    m_orig_subcurve2 (NULL)
      {
        upper_g = 0;
        lower_g = 0;
        upper = 0;
        lower = 0;

        m_lastCurve = X_monotone_curve_2();

        in_purple = false;
        ins_sub_cv1 = false;
      }

    /*! Constructor given a curve. */
    My_Sweep_line_subcurve (const X_monotone_curve_2 &curve) :
    m_lastCurve (curve),
    m_orig_subcurve1 (NULL),
    m_orig_subcurve2 (NULL)
      {
        upper_g = 0;
        lower_g = 0;
        upper = 0;
        lower = 0;

        in_purple = false;
        ins_sub_cv1 = false;
      }

   //subcurve has been added to purple arrangement
    void set_in_purple(bool b)
      {
        in_purple = b;
      }

    //has subcurve been added to purple arrangement?
    bool is_in_purple()
      {
        return in_purple;
      }

    //special case for vertical segments
    void set_insert_sub_cv1(bool b)
      {
        ins_sub_cv1 = b;
      }

    //special case for vertical segments
    bool insert_sub_cv1()
      {
        return ins_sub_cv1;
      }

    void set_upper_guard(My_Scout* g)
      {
        upper_g = g;
      }

    void remove_upper_guard()
      {

        upper_g = 0;
      }

    void remove_upper_scout()
      {
        upper = 0;
      }

    void set_lower_guard(My_Scout* g)
      {
        lower_g = g;
      }

    void remove_lower_guard()
      {
        lower_g = 0;
      }

    void remove_lower_scout()
      {
        lower = 0;
      }


    const bool is_guarded_by_upper_scout() const
      {
        if(upper_g != 0)
          {
            return true;
          }
        else
          {
            return false;
          }
      }

    const bool is_guarded_by_lower_scout() const
      {
        if(lower_g != 0)
          {
            return true;
          }
        else
          {
            return false;
          }
      }

    const bool is_boundary() const
      {
        return (has_upper_scout() || has_lower_scout());
      }

    void set_upper_scout(My_Scout* u)
      {
        upper = u;
      }

    const bool has_upper_scout() const
      {
        if(upper != 0)
          {
            return true;
          }
        else
          {
            return false;
          }
      }

    typename Traits_2::Color color()
      {
        if(m_lastCurve.left_is_set() && m_lastCurve.right_is_set())
          {
            return m_lastCurve.color();
          }
        else
          {
            return Traits_2::DUMMY;
          }
      }

    void set_lower_scout(My_Scout* l)
      {
        lower = l;
      }

    const bool has_lower_scout() const
      {
        if(lower != 0)
          {
            return true;
          }
        else
          {
            return false;
          }
      }

    const bool is_lower_boundary() const
      {
        return has_lower_scout();
      }

    My_Scout* lower_scout()
      {
        return lower;
      }

    My_Scout* upper_scout()
      {
        return upper;
      }

    My_Scout* upper_guard()
      {
        return upper_g;
      }

    My_Scout* lower_guard()
      {
        return lower_g;
      }

    /*! Initialize the subcurves by setting the curve. */
    void init_subcurve (const X_monotone_curve_2 &curve)
      {
        m_lastCurve = curve;
      }

    /*! Destructor. */
    ~My_Sweep_line_subcurve()
      {
      }

    /*! Get the last intersecing curve so far (const version). */
    const X_monotone_curve_2& last_curve () const
      {

        return (m_lastCurve);
      }

    /*! Get the last intersecing curve so far (non-const version). */
    X_monotone_curve_2& last_curve()
      {
        return (m_lastCurve);
      }

    /*! Set the last intersecing curve so far. */
    void set_last_curve (const X_monotone_curve_2 &cv)
      {
        m_lastCurve = cv;
      }

    /*! Check if the given event is the matches the right-end event. */
    template<class SweepEvent>
    bool is_end_point (const SweepEvent* event) const
      {
        return (m_right_event == (Event*)event);
      }

    /*! Get the event that corresponds to the left end of the subcurve. */
    Event* left_event() const
      {
        return (m_left_event);
      }

    /*! Get the event that corresponds to the right end of the subcurve. */
    Event* right_event() const
      {
        return (m_right_event);
      }

    /*! Set the event that corresponds to the left end of the subcurve. */
    template<class SweepEvent>
    void set_left_event (SweepEvent* event)
      {
        m_left_event =(Event*)event;
      }

    /*! Set the event that corresponds to the right end of the subcurve. */
    template<class SweepEvent>
    void set_right_event(SweepEvent* event)
      {
        m_right_event = (Event*)event;
      }

    /*! Get the location of the subcurve in the status line .*/
    Status_line_iterator hint() const
      {
        return (m_hint);
      }

    /*! Set the location of the subcurve in the status line .*/
    void set_hint(Status_line_iterator hint)
      {
        m_hint = hint;
      }

    /*! Get the subcurves that originate an overlap. */
    Self* originating_subcurve1()
      {
        return (m_orig_subcurve1);
      }

    Self* originating_subcurve2()
      {
        return (m_orig_subcurve2);
      }

    /*! Set the subcurves that originate an overlap. */
    void set_originating_subcurve1 (Self* orig_subcurve1)
      {
        m_orig_subcurve1 = orig_subcurve1;
      }

    void set_originating_subcurve2 (Self* orig_subcurve2)
      {
        m_orig_subcurve2 = orig_subcurve2;
      }

    /*! Get all the leaf-nodes in the hierarchy of overlapping subcurves. */
    template <class OutputIterator>
    OutputIterator all_leaves (OutputIterator oi)
      {
        if (m_orig_subcurve1 == NULL)
          {
            *oi = this;
            ++oi;
            return (oi);
          }

        oi = m_orig_subcurve1->all_leaves (oi);
        oi = m_orig_subcurve2->all_leaves (oi);
        return (oi);
      }

    /*! Get all the nodes in the hierarchy of overlapping subcurves. */
    template <class OutputIterator>
    OutputIterator all_nodes (OutputIterator oi)
      {
        *oi = this;
        ++oi;

        if (m_orig_subcurve1 == NULL)
        return (oi);

        oi = m_orig_subcurve1->get_all_inner_nodes (oi);
        oi = m_orig_subcurve2->get_all_inner_nodes (oi);
        return (oi);
      }

    /*! Check if the given subcurve is a node in the overlapping hierarchy. */
    bool is_inner_node (Self *s)
      {
        if (this == s)
        return (true);

        if (m_orig_subcurve1 == NULL)
        return (false);

        return (m_orig_subcurve1->is_inner_node (s) ||
            m_orig_subcurve2->is_inner_node (s));
      }

    /*! Check if the given subcurve is a leaf in the overlapping hierarchy. */
    bool is_leaf (Self* s)
      {
        if (m_orig_subcurve1 == NULL)
        return (this == s);

        return (m_orig_subcurve1->is_leaf (s) ||
            m_orig_subcurve2->is_leaf (s));
      }

    /*! Check if the two hierarchies contain the same leaf nodes. */
    bool has_same_leaves (Self *s)
      {
        std::list<Self*> my_leaves;
        std::list<Self*> other_leaves;

        this->all_leaves (std::back_inserter (my_leaves));
        s->all_leaves (std::back_inserter (other_leaves));

        typename std::list<Self*>::iterator iter;

        for(iter = my_leaves.begin(); iter != my_leaves.end(); ++iter)
          {
            if (std::find(other_leaves.begin(), other_leaves.end(), *iter) ==
                other_leaves.end())
            return (false);
          }

        for(iter = other_leaves.begin(); iter != other_leaves.end(); ++iter)
          {
            if (std::find(my_leaves.begin(), my_leaves.end(), *iter) ==
                my_leaves.end())
            return (false);
          }

        return (true);
      }

    /*! Check if the two hierarchies contain a common leaf node. */
    bool has_common_leaf (Self *s)
      {
        std::list<Self*> my_leaves;
        std::list<Self*> other_leaves;

        this->all_leaves (std::back_inserter (my_leaves));
        s->all_leaves (std::back_inserter (other_leaves));

        typename std::list<Self*>::iterator iter;

        for (iter = my_leaves.begin(); iter != my_leaves.end(); ++iter)
          {
            if (std::find(other_leaves.begin(), other_leaves.end(), *iter) !=
                other_leaves.end())
            return (true);
          }
        return (false);
      }

    /*! Get all distinct nodes from the two hierarchies. */
    template <class OutputIterator>
    OutputIterator distinct_nodes(Self *s, OutputIterator oi)
      {
        if (m_orig_subcurve1 == NULL)
          {
            if (s->is_leaf(this))
              {
                *oi = this;
                ++oi;
              }
            return (oi);
          }

        if (! s->is_inner_node (m_orig_subcurve1))
          {
            *oi = m_orig_subcurve1;
            ++oi;
          }
        else
          {
            oi = m_orig_subcurve1->distinct_nodes (s, oi);
          }

        if (! s->is_inner_node (m_orig_subcurve2))
          {
            *oi = m_orig_subcurve2;
            ++oi;
          }
        else
          {
            oi = m_orig_subcurve2->distinct_nodes (s, oi);
          }

        return (oi);
      }

    /*! Get the depth of the overlap hierarchy. */
    unsigned int overlap_depth()
      {
        if (m_orig_subcurve1 == NULL)
        return (1);

        unsigned int depth1 = m_orig_subcurve1->overlap_depth();
        unsigned int depth2 = m_orig_subcurve2->overlap_depth();

        if (depth1> depth2)
        return (depth1 + 1);
        else
        return (depth2 + 1);
      }

    void Print();
  };

template<class Arrangement_, class Traits>
  void
  My_Sweep_line_subcurve<Arrangement_, Traits>::Print()
  {
    std::cout << "Curve " << this << " " << m_lastCurve << std::endl;
  }

CGAL_END_NAMESPACE

#endif

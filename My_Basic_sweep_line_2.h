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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Basic_sweep_line_2.h $
// $Id: Basic_sweep_line_2.h 41325 2007-12-26 15:39:40Z golubevs $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 (based on old version by Tali Zvi)

#ifndef MY_CGAL_BASIC_SWEEP_LINE_2_H
#define MY_CGAL_BASIC_SWEEP_LINE_2_H

/*! \file
 * Definition of the Basic_sweep_line_2 class.
 */

#include "My_Sweep_line_functors.h"
#include "My_Sweep_line_subcurve.h"
#include "Scout.h"
#include "My_Sweep_line_event.h"

CGAL_BEGIN_NAMESPACE

/*! \class Basic_Sweep_line_2
 * A class that implements the sweep line algorithm for general x-monotone
 * curves that are pairwise disjoint in their interiors (an additional set
 * of isolated points may also be supplied).
 * The x-monotone curve type and the point type are defined by the traits class
 * that is one of the template parameters.
 */

template<class Arrangement_, class Traits_, class Subcurve_, typename Event_,
    typename Allocator_>
  class My_Basic_sweep_line_2
  {
  public:

    typedef Traits_ Traits_2;
    typedef Event_ Event;
    typedef Subcurve_ Subcurve;
    typedef Allocator_ Allocator;
    typedef Arrangement_ Arrangement_2;

    typedef Arr_traits_basic_adaptor_2<Traits_2> Traits_adaptor_2;
typedef    typename Traits_adaptor_2::Point_2 Point_2;
    typedef typename Point_2::Base Base_point_2;

    typedef typename Traits_adaptor_2::X_monotone_curve_2 X_monotone_curve_2;
    typedef typename X_monotone_curve_2::Base Base_x_monotone_curve_2;

    //typedef typename Traits_adaptor_2::Boundary_category Boundary_category;
    typedef Scout<Arrangement_2,Traits_2> My_Scout;

    typedef CGAL::My_Compare_events<Traits_adaptor_2, Event> Compare_events;
    typedef Multiset<Event*, Compare_events, Allocator> Event_queue;
    typedef typename Event_queue::iterator Event_queue_iterator;

    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_handle Halfedge_handle;

    typedef typename Event::Subcurve_iterator
    Event_subcurve_iterator;

    typedef My_Sweep_line_event<Traits_2, Subcurve> Base_event;
    typedef typename Base_event::Attribute Attribute;

    typedef My_Sweep_line_subcurve<Arrangement_2, Traits_2> Base_subcurve;
    typedef class My_Curve_comparer<Traits_2, Base_subcurve> Compare_curves;
    typedef class Scout_comparer<Traits_2,Base_subcurve, My_Scout> Compare_scouts;
    typedef Multiset<Base_subcurve*,
    Compare_curves,
    Allocator> Status_line;
    typedef Multiset<My_Scout*,Compare_scouts,Allocator> Scout_set;
    typedef typename Status_line::iterator Status_line_iterator;
    typedef typename Scout_set::iterator Scout_set_iterator;
    typedef typename Scout_set::reverse_iterator Scout_set_reverse_iterator;

    typedef typename Allocator::template rebind<Event> Event_alloc_rebind;
    typedef typename Event_alloc_rebind::other Event_alloc;

    typedef typename Allocator::template rebind<Subcurve> Subcurve_alloc_rebind;
    typedef typename Subcurve_alloc_rebind::other Subcurve_alloc;

    //dummy subcurve associated with the dummy scouts
    Subcurve* dummy_sub;

    //dummy scout
    My_Scout* lower_dummy;
    My_Scout* upper_dummy;

  protected:

    /*! \struct
     * An auxiliary functor for comparing event pointers.
     */
    struct CompEventPtr
      {
        Comparison_result operator() (Event *e1, Event *e2) const
          {
            if (e1 < e2)
            return (SMALLER);
            if (e1> e2)
            return (LARGER);
            return (EQUAL);
          }
      };

    typedef Multiset<Event*, CompEventPtr> Allocated_events_set;
    typedef typename Allocated_events_set::iterator Allocated_events_iterator;

    // Data members:

    //the output arrangement
    Arrangement_2* m_purple;

    Traits_adaptor_2 *m_traits; // A traits-class object.
    bool m_traitsOwner; // Whether this object was allocated by
    // this class (and thus should be freed).

  public:
    Event *m_currentEvent; // The current event.
    Event_queue_iterator m_currentEventIter;
    Vertex_const_handle current_vertex;
    Point_2 current_event_point;
    typename Traits_2::Color current_event_point_color;

    // Comparison functor for the status line.
    Compare_curves m_statusLineCurveLess;

  protected:

    Compare_events m_queueEventLess; // Comparison functor for the event queue.
    Compare_scouts m_scoutLess; // Comparison functor for the scout set

    Event_queue *m_queue; // The event queue (the X-structure).
    Scout_set *m_scouts; // the scout set


    Subcurve *m_red_subCurves; // An array of the subcurves.
    Subcurve *m_blue_subCurves;

    Status_line m_red_statusLine; // The status line (the Y-structure).
    Status_line m_blue_statusLine;

    Allocated_events_set m_allocated_events;
    // The events that have been allocated
    // (and have not yet been deallocated).

    Status_line_iterator m_red_status_line_insert_hint;
    Status_line_iterator m_blue_status_line_insert_hint;
    // An iterator of the status line, which
    // is used as a hint for insertions.

    bool m_is_event_on_above;
    // Indicates if the current event is on
    // the interior of existing curve. This
    // may happen only with events that are
    // associated with isolated query points.

    bool d_to_n_default; //region starting can be delegated to next default event
    typename Traits_2::Color b_color; //the color of the curve below
    bool no_above; //is anything above?
    Subcurve* c_blue_below; //the current blue curve below
    Subcurve* c_red_below; //the current red curve below

    Event_alloc m_eventAlloc; // An allocator for the events objects.
    Subcurve_alloc m_subCurveAlloc; // An allocator for the subcurve objects.

    Event m_masterEvent; // A master Event (created once by the
    // constructor) for the allocator's usage.

    Subcurve m_masterSubcurve; // A master Subcurve (created once by the
    // constructor) for the allocator's usage.

    unsigned int m_num_of_red_subCurves; // Number of subcurves.
    unsigned int m_num_of_blue_subCurves;

  public:

    /*!
     * Constructor.
     */
    My_Basic_sweep_line_2 ();

    /*!
     * Constructor with a traits class.
     * \param traits A pointer to a sweep-line traits object.
     */

    /*! Destructor. */
    virtual ~My_Basic_sweep_line_2 ();

    template<class CurveInputIterator, class PointInputIterator>
    void sweep (CurveInputIterator red_curves_begin,
        CurveInputIterator red_curves_end,
        CurveInputIterator blue_curves_begin,
        CurveInputIterator blue_curves_end,
        PointInputIterator points_begin,
        PointInputIterator points_end)
      {
        _init_points(points_begin, points_end, Base_event::ACTION);
        _init_red_sweep(red_curves_begin, red_curves_end);
        _init_blue_sweep(blue_curves_begin, blue_curves_end);
        _init_curve_pairs();
        _sweep();
        _complete_sweep();
      }

    //delegate the regions starting etc. to the next default vertex above
    void delegate_to_next_default()
      {
        d_to_n_default = true;
      }

    //forget about the delegation
    void reset_delegation()
      {
        d_to_n_default = false;
      }

    //has the region starting etc. been delegated to the next default event?
    bool delegated_to_next_default()
      {
        return d_to_n_default;
      }

    //check the color and choose the correct data
    unsigned int number_of_right_curves()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->number_of_blue_right_curves();
          }
        else
          {
            return m_currentEvent->number_of_red_right_curves();
          }
      }

    //check the color and choose the correct data
    unsigned int number_of_left_curves()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->number_of_blue_left_curves();
          }
        else
          {
            return m_currentEvent->number_of_red_left_curves();
          }
      }

    //check the color and choose the correct data
    Event_subcurve_iterator left_curves_begin()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->blue_left_curves_begin();
          }
        else
          {
            return m_currentEvent->red_left_curves_begin();
          }
      }

    //check the color and choose the correct data
    Event_subcurve_iterator left_curves_end()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->blue_left_curves_end();
          }
        else
          {
            return m_currentEvent->red_left_curves_end();
          }
      }

    //check the color and choose the correct data
    Event_subcurve_iterator right_curves_begin()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->blue_right_curves_begin();
          }
        else
          {
            return m_currentEvent->red_right_curves_begin();
          }
      }

    //check the color and choose the correct data
    Event_subcurve_iterator right_curves_end()
      {
        if(current_event_point_color == Traits_2::BLUE)
          {
            return m_currentEvent->blue_right_curves_end();
          }
        else
          {
            return m_currentEvent->red_right_curves_end();
          }
      }

    //check the color and choose the correct data
    Status_line* different_colored_status_line()
      {
        if (current_event_point_color == Traits_2::BLUE)
          {
            return &m_red_statusLine;

          }
        else
          {
            return &m_blue_statusLine;
          }
      }

    //check the color and choose the correct data
    Status_line* same_colored_status_line()
      {
        if (current_event_point_color == Traits_2::BLUE)
          {
            return &m_blue_statusLine;

          }
        else
          {
            return &m_red_statusLine;
          }
      }

    //check the color and choose the correct data
    Status_line_iterator status_line_insert_hint()
      {
        if (current_event_point_color == Traits_2::BLUE)
          {
            return m_blue_status_line_insert_hint;

          }
        else
          {
            return m_red_status_line_insert_hint;
          }
      }


    //the color of the curve directly below
    void set_current_color_below(typename Traits_2::Color c)
      {
        b_color = c;
      }

    //remember if there is a curve above
    void next_region_is_unbounded_above(bool b)
      {
        no_above = b;
      }

    //what color is the current curve below
    typename Traits_2::Color current_color_below()
      {
        return b_color;
      }

    //if there is no curve above, the region to start will be unbounded
    bool is_next_region_unbounded_above()
      {
        return no_above;
      }

    //maintain the current red curve below
    void set_current_red_below(Subcurve* s)
      {
        c_red_below = s;
        if(s==0)
          {
            b_color = Traits_2::DUMMY;
          }
        else
          {
            b_color = Traits_2::RED;
          }
      }

    //maintain the current blue curve below
    void set_current_blue_below(Subcurve* s)
      {
        c_blue_below = s;
        if(s==0)
          {
            b_color = Traits_2::DUMMY;
          }
        else
          {
            b_color = Traits_2::BLUE;
          }
      }

    Subcurve* current_red_below()
      {
        return c_red_below;
      }

    Subcurve* current_blue_below()
      {
        return c_blue_below;
      }

    /*! Get an iterator for the first subcurve in the status line. */
    Status_line_iterator red_status_line_begin ()
      {
        return (m_red_statusLine.begin());
      }

    /*! Get a past-the-end iterator for the subcurves in the status line. */
    Status_line_iterator red_status_line_end()
      {
        return (m_red_statusLine.end());
      }

    /*! Get the status line size. */
    unsigned int red_status_line_size() const
      {
        return (m_red_statusLine.size());
      }

    /*! Check if the status line is empty. */
    bool is_red_status_line_empty() const
      {
        return (m_red_statusLine.empty());
      }

    /*! Get an iterator for the first subcurve in the status line. */
    Status_line_iterator blue_status_line_begin ()
      {
        return (m_blue_statusLine.begin());
      }

    /*! Get a past-the-end iterator for the subcurves in the status line. */
    Status_line_iterator blue_status_line_end()
      {
        return (m_blue_statusLine.end());
      }

    /*! Get the status line size. */
    unsigned int blue_status_line_size() const
      {
        return (m_blue_statusLine.size());
      }

    /*! Check if the status line is empty. */
    bool is_blue_status_line_empty() const
      {
        return (m_blue_statusLine.empty());
      }

    //is the next event point in the event queue a default vertex?
    bool has_vertex_above()
      {
        Event_queue_iterator next = m_currentEventIter;
        next++;
        bool is_d_a = false;
        if(next != this->event_queue_end())
          {
            Point_2 point = (*next)->point();
            if(current_event_point.base().x() == point.base().x())
              {
                is_d_a = true;
              }
          }
        return is_d_a;
      }

    /*! Get an iterator for the first event in event queue. */
    Event_queue_iterator event_queue_begin()
      {
        return (m_queue->begin());
      }

    /*! Get a past-the-end iterator for the events in the in event queue. */
    Event_queue_iterator event_queue_end()
      {
        return (m_queue->end());
      }

    /*! Get the event queue size. */
    unsigned int event_queue_size() const
      {
        return (m_queue->size());
      }

    /*! Check if the event queue is empty. */
    bool is_event_queue_empty() const
      {
        return (m_queue->empty());
      }

    /*!
     * Deallocate event object.
     * This method is made public to allow the visitor to manage the events
     * deallocation (as necessary).
     */
    void deallocate_event(Event* event);

    /*! Get the current event */
    Event* current_event()
      {
        return (m_currentEvent);
      }

    /*! Get the traits object */
    Traits_2* traits ()
      {
        return m_traits;
      }

    // get the purple arrangement
    Arrangement_2* get_purple()
      {
        return m_purple;
      }

    // get the set of scouts
    Scout_set* get_scouts()
      {
        return m_scouts;
      }

    // get the scout comparison functor
    Compare_scouts get_scoutLess()
      {
        return m_scoutLess;
      }

  protected:

    /*! Perform the main sweep-line loop. */
    void _sweep();

    //debug function
    void _validate_invariants();

    /*! Create an event object for each input point. */
    template <class PointInputIterator>
    void _init_points (PointInputIterator points_begin,
        PointInputIterator points_end,
        Attribute type)
      {
        PointInputIterator pit;
        for (pit = points_begin; pit != points_end; ++pit)
        _init_point (*pit, type);
        return;
      }

    /*! Create a Subcurve object and two Event objects for each curve. */
    template<class CurveInputIterator>
    void _init_red_curves (CurveInputIterator curves_begin,
        CurveInputIterator curves_end)
      {
        CurveInputIterator cit;
        unsigned int index = 0;

        for (cit = curves_begin; cit != curves_end; ++cit, ++index)
          {
            CGAL_assertion(cit->left_is_set());
            CGAL_assertion(cit->right_is_set());
            _init_red_curve (*cit, index);
          }

        return;
      }

    /*! Create a Subcurve object and two Event objects for each curve. */
    template<class CurveInputIterator>
    void _init_blue_curves (CurveInputIterator curves_begin,
        CurveInputIterator curves_end)
      {
        CurveInputIterator cit;
        unsigned int index = 0;

        for (cit = curves_begin; cit != curves_end; ++cit, ++index)
          {
            CGAL_assertion(cit->left_is_set());
            CGAL_assertion(cit->right_is_set());

            _init_blue_curve (*cit, index);
          }

        return;
      }

    /*! Initiliaze the sweep algorithm. */
    template<class CurveInputIterator>
    void _init_red_sweep (CurveInputIterator curves_begin,
        CurveInputIterator curves_end)
      {
        m_num_of_red_subCurves = std::distance (curves_begin, curves_end);

        _init_red_structures();

        // Initialize the curves.
        _init_red_curves (curves_begin, curves_end);
        return;
      }

    /*! Initiliaze the sweep algorithm. */
    template<class CurveInputIterator>
    void _init_blue_sweep (CurveInputIterator curves_begin,
        CurveInputIterator curves_end)
      {
        m_num_of_blue_subCurves = std::distance (curves_begin, curves_end);

        _init_blue_structures();

        // Initialize the curves.
        _init_blue_curves (curves_begin, curves_end);
        return;
      }

    /*! Initialize the data structures for the sweep-line algorithm. */
    virtual void _init_red_structures ();
    virtual void _init_blue_structures ();

  public:
    /*! Compete the sweep (compete data strcures) */
    virtual void _complete_sweep();

    virtual void _init_curve_pairs();

  protected:
    /*!
     * Initialize an event associated with a point.
     * \param p The given point.
     * \param type The event type.
     */
    void _init_point (const Point_2& pt, Attribute type);

    /*!
     * Initialize the events associated with an x-monotone curve.
     * \param curve The given x-monotone curve.
     * \param index Its unique index.
     */
    void _init_red_curve (const X_monotone_curve_2& curve, unsigned int index);
    void _init_blue_curve (const X_monotone_curve_2& curve, unsigned int index);

    /*!
     * Initialize an event associated with an x-monotone curve end.
     * \param cv The given x-monotone curve.
     * \param ind Its end (ARR_MIN_END or ARR_MAX_END).
     * \param sc The subcurve corresponding to cv.
     */
    void _init_red_curve_end (const X_monotone_curve_2& cv,
        Arr_curve_end ind,
        Subcurve* sc);

    void _init_blue_curve_end (const X_monotone_curve_2& cv,
        Arr_curve_end ind,
        Subcurve* sc);

    /*!
     * Handle the subcurves that are to the left of the event point (i.e.,
     * subcurves that we are done with).
     */
    virtual void _handle_red_left_curves();
    virtual void _handle_blue_left_curves();

    //some special cases involving red blue intersections
    virtual void prepare_red_blue_intersection();
    virtual void prepare_red_blue_vertical();

    /*!
     * Handle an event that does not have any incident left curves.
     * Such an event is usually the left endpoint of its incident right
     * subcurves, and we locate thei position in the status line.
     */
    void _handle_event_without_red_left_curves ();
    void _handle_event_without_blue_left_curves ();
    /*!
     * Sort the left subcurves of an event point according to their order in
     * their status line (no geometric comprasions are needed).
     */
    void _sort_red_left_curves ();
    void _sort_blue_left_curves ();

    /*! Handle the subcurves to the right of the current event point. */
    virtual void _handle_red_right_curves ();
    virtual void _handle_blue_right_curves ();

    /*!
     * Add a subcurve to the right of an event point.
     * \param event The event point.
     * \param curve The subcurve to add.
     * \return (true) if an overlap occured; (false) otherwise.
     */
    virtual bool _add_red_curve_to_right (Event* event, Subcurve* curve,
        bool overlap_exist = false);
    virtual bool _add_blue_curve_to_right (Event* event, Subcurve* curve,
        bool overlap_exist = false);

    /*! Remove a curve from the status line. */
    void _remove_curve_from_red_status_line (Subcurve *leftCurve);
    void _remove_curve_from_blue_status_line (Subcurve *leftCurve);

    /*!
     * Allocate an event object associated with a given point.
     * \param pt The point.
     * \param type The event type.
     * \param ps_x The boundary condition of the point in x.
     * \param ps_y The boundary condition of the point in y.
     * \pre Neither one of the boundary conditions is +/-oo.
     * \return The created event.
     */
    Event* _allocate_event (const Point_2& pt, Attribute type,
        Arr_parameter_space ps_x, Arr_parameter_space ps_y);

    /*!
     * Allocate an event at infinity, which is not associated with a valid point.
     * \param type The event type.
     * \param ps_x The boundary condition of the point in x.
     * \param ps_y The boundary condition of the point in y.
     * \param At least one of the boundary conditions is +/-oo.
     * \return The created event.
     */
    Event* _allocate_event_at_infinity (Attribute type,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y);

    /*!
     * Push a finite event point into the event queue.
     * \param pt The point associated with the event.
     * \param type The event type.
     * \param ps_x The boundary condition of the point in x.
     * \param ps_y The boundary condition of the point in y.
     * \param sc A subcurve that the new event represents on of its endpoints.
     * \return A pair that comprises a pointer to the event, and a flag
     *         indicating whether this is a new event (if false, the event
     *         was in the queue and we just updated it).
     */
    std::pair<Event*, bool> _push_red_event (const Point_2& pt,
        Attribute type,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y,
        Subcurve* sc = NULL);

    std::pair<Event*, bool> _push_blue_event (const Point_2& pt,
        Attribute type,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y,
        Subcurve* sc = NULL);

    /*!
     * Push an event point associated with a curve end into the event queue.
     * \param cv The x-monotone curve.
     * \param ind The relevant curve end.
     * \param type The event type.
     * \param ps_x The boundary condition of the point in x.
     * \param ps_y The boundary condition of the point in y.
     * \param sc A subcurve that the new event represents on of its endpoints.
     * \return A pair that comprises a pointer to the event, and a flag
     *         indicating whether this is a new event (if false, the event
     *         was in the queue and we just updated it).
     */
    std::pair<Event*, bool> _push_red_event (const X_monotone_curve_2& cv,
        Arr_curve_end ind,
        Attribute type,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y,
        Subcurve* sc = NULL);

    std::pair<Event*, bool> _push_blue_event (const X_monotone_curve_2& cv,
        Arr_curve_end ind,
        Attribute type,
        Arr_parameter_space ps_x,
        Arr_parameter_space ps_y,
        Subcurve* sc = NULL);

//    void _update_event_at_infinity(Event* e,
//        const X_monotone_curve_2& cv,
//        Arr_curve_end ind,
//        bool is_new)
//      {
//        _update_event_at_infinity(e, cv, ind, is_new, Boundary_category());
//      }

//    void _update_event_at_infinity(Event* e,
//        const X_monotone_curve_2& cv,
//        Arr_curve_end ind,
//        bool is_new,
//        Arr_has_boundary_tag)
//      {
//      }
//
//    void _update_event_at_infinity(Event* /* e */,
//        const X_monotone_curve_2& /* cv */,
//        Arr_curve_end /* ind */,
//        bool /* is_new */,
//        Arr_no_boundary_tag)
//      {
//        CGAL_error();
//      }

    virtual void proceed_according_to_vertex_type();

    void PrintEventQueue();
    void PrintRedSubCurves();
    void PrintBlueSubCurves();
    void PrintRedStatusLine();
    void PrintBlueStatusLine();
    void PrintInfinityType(Arr_parameter_space x, Arr_parameter_space y);
    void PrintEvent(const Event* e);

  };

#include "My_Sweep_line_2_debug.h"

CGAL_END_NAMESPACE

#include "My_Basic_sweep_line_2_impl.h"

#endif

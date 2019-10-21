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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2/Basic_sweep_line_2_impl.h $
// $Id: Basic_sweep_line_2_impl.h 46662 2008-11-04 14:49:44Z eric $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Efi Fogel <efif@post.tau.ac.il>
//                 (based on old version by Tali Zvi)

#ifndef MY_CGAL_BASIC_SWEEP_LINE_2_IMPL_H
#define MY_CGAL_BASIC_SWEEP_LINE_2_IMPL_H

/*! \file
 * Member-function definitions for the Basic_sweep_line_2 class.
 */

CGAL_BEGIN_NAMESPACE

//-----------------------------------------------------------------------------
// Constructor.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::My_Basic_sweep_line_2() :
    m_traits(new Traits_adaptor_2()), m_traitsOwner(true),
        m_statusLineCurveLess(m_traits, &m_currentEvent), m_queueEventLess(
            m_traits), m_scoutLess(m_traits, &m_currentEvent), m_queue(
            new Event_queue(m_queueEventLess)), m_scouts(new Scout_set(
            m_scoutLess)), m_red_statusLine(m_statusLineCurveLess),
        m_blue_statusLine(m_statusLineCurveLess),
        m_red_status_line_insert_hint(m_red_statusLine.begin()),
        m_blue_status_line_insert_hint(m_blue_statusLine.begin()),
        d_to_n_default(false), m_num_of_red_subCurves(0),
        m_num_of_blue_subCurves(0)
  {
    m_purple = new Arr();
    dummy_sub = new Subcv();
    lower_dummy = 0;
    upper_dummy = 0;
    c_red_below = 0;
    c_blue_below = 0;
    b_color = Traits_2::DUMMY;
  }

//-----------------------------------------------------------------------------
// Destructor.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::~My_Basic_sweep_line_2()
  {
    // Free the traits-class object, if we own it.

    if (m_traitsOwner)
      delete m_traits;

    // Free the event queue.
    delete m_queue;

    delete m_scouts;
    delete dummy_sub;

    if (lower_dummy != 0)
      {
        delete lower_dummy;
      }

    if (upper_dummy != 0)
      {
        delete upper_dummy;
      }

    // Free all the event that have not been de-allocated so far.
    Allocated_events_iterator iter;
    Event *p_event;

    for (iter = m_allocated_events.begin(); iter != m_allocated_events.end(); ++iter)
      {
        p_event = *iter;
        m_eventAlloc.destroy(p_event);
        m_eventAlloc.deallocate(p_event, 1);
      }
  }

//-----------------------------------------------------------------------------
// Deallocate event object..
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::deallocate_event(
      Event* event)
  {
    // Remove the event from the set of allocated events.
    m_allocated_events.erase(event);

    // Perfrom the actual deallocation.
    m_eventAlloc.destroy(event);
    m_eventAlloc.deallocate(event, 1);
    return;
  }

//-----------------------------------------------------------------------------
// Perform the main sweep-line loop.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_sweep()
  {
    MY_CGAL_SL_PRINT("begin _sweep()");

    // Looping over the events in the queue.
    Event_queue_iterator eventIter = m_queue->begin();

    bool first = true;
    while (eventIter != m_queue->end())
      {
        // Get the next event from the queue.
        m_currentEvent = *eventIter;
        m_currentEventIter = eventIter;

        //reset curves below if we are at a new x-value
        if (!first)
          {
            if (this->current_event_point.base().x()
                < this->m_currentEvent->point().base().x())
              {
                this->set_current_red_below(0);
                this->set_current_blue_below(0);
              }
          }
        first = false;

        current_event_point = m_currentEvent->point();

        // before the _handle_..._curves functions we might have to take care
        // of some special cases
        prepare_red_blue_intersection();
        prepare_red_blue_vertical();

        //handling done by the sweep line framework
        _handle_red_left_curves();
        _handle_blue_left_curves();
        _handle_red_right_curves();
        _handle_blue_right_curves();

        proceed_according_to_vertex_type();

        MY_CGAL_SL_DEBUG(this->_validate_invariants(););

        m_queue->erase(eventIter);
        eventIter = m_queue->begin();
      }
  }

//-----------------------------------------------------------------------------
// Initialize the data structures for the sweep-line algorithm.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_red_structures()
  {
    //  CGAL_assertion(m_queue->empty());
    CGAL_assertion((m_red_statusLine.size() == 0));

    // Allocate all of the Subcurve objects as one block.
    m_red_subCurves = m_subCurveAlloc.allocate(m_num_of_red_subCurves);
    return;
  }

//-----------------------------------------------------------------------------
// Initialize the data structures for the sweep-line algorithm.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_blue_structures()
  {
    //CGAL_assertion(m_queue->empty());
    CGAL_assertion((m_blue_statusLine.size() == 0));

    // Allocate all of the Subcurve objects as one block.
    m_blue_subCurves = m_subCurveAlloc.allocate(m_num_of_blue_subCurves);
    return;
  }

//-----------------------------------------------------------------------------
// Complete the sweep (complete the data structures).
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_complete_sweep()
  {
    CGAL_assertion(m_queue->empty());
    CGAL_assertion((m_red_statusLine.size() == 0) && (m_blue_statusLine.size()
        == 0));

    // Free all subcurve objects.
    unsigned int i;
    for (i = 0; i < m_num_of_red_subCurves; ++i)
      m_subCurveAlloc.destroy(m_red_subCurves + i);

    for (i = 0; i < m_num_of_blue_subCurves; ++i)
      m_subCurveAlloc.destroy(m_blue_subCurves + i);

    if (m_num_of_red_subCurves > 0)
      m_subCurveAlloc.deallocate(m_red_subCurves, m_num_of_red_subCurves);

    if (m_num_of_blue_subCurves > 0)
      m_subCurveAlloc.deallocate(m_blue_subCurves, m_num_of_blue_subCurves);

    return;
  }

//-----------------------------------------------------------------------------
// Complete the sweep (complete the data structures).
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_curve_pairs()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Initialize an event associated with a point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_point(
      const Point_2& pt, Attribute type)
  {

    // Create the event, or obtain an existing event in the queue.
    // Note that an isolated point does not have any boundary conditions.

    const std::pair<Event*, bool>& pair_res = _push_red_event(pt, type,
        ARR_INTERIOR, ARR_INTERIOR);//subcurve == NULL, darum egal, ob push_red oder push_blue

    return;
  }

//-----------------------------------------------------------------------------
// Initialize the events associated with an x-monotone curve.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_red_curve(
      const X_monotone_curve_2& curve, unsigned int index)
  {

    CGAL_assertion(curve.left_is_set());
    CGAL_assertion(curve.right_is_set());
    // Construct an initialize a subcurve object.
    m_subCurveAlloc.construct(m_red_subCurves + index, m_masterSubcurve);

    (m_red_subCurves + index)->init_subcurve(curve);

    // Create two events associated with the curve ends.
    _init_red_curve_end(curve, ARR_MAX_END, m_red_subCurves + index);
    _init_red_curve_end(curve, ARR_MIN_END, m_red_subCurves + index);

    return;
  }

//-----------------------------------------------------------------------------
// Initialize the events associated with an x-monotone curve.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_blue_curve(
      const X_monotone_curve_2& curve, unsigned int index)
  {
    CGAL_assertion(curve.left_is_set());
    CGAL_assertion(curve.right_is_set());

    // Construct an initialize a subcurve object.
    m_subCurveAlloc.construct(m_blue_subCurves + index, m_masterSubcurve);

    (m_blue_subCurves + index)->init_subcurve(curve);

    // Create two events associated with the curve ends.
    _init_blue_curve_end(curve, ARR_MAX_END, m_blue_subCurves + index);
    _init_blue_curve_end(curve, ARR_MIN_END, m_blue_subCurves + index);

    return;
  }

//-----------------------------------------------------------------------------
// Initialize an event associated with an x-monotone curve end.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_red_curve_end(
      const X_monotone_curve_2& cv, Arr_curve_end ind, Subcurve* sc)
  {
    // Get the boundary conditions of the curve end.
    const Attribute end_attr = (ind == ARR_MIN_END) ? Base_event::LEFT_END
        : Base_event::RIGHT_END;

    Arr_parameter_space ps_x = m_traits->parameter_space_in_x_2_object()(cv,
        ind);
    Arr_parameter_space ps_y = m_traits->parameter_space_in_y_2_object()(cv,
        ind);

    // Create the corresponding event an push it into the event queue.
    std::pair<Event*, bool> pair_res;

    if (/*m_traits->is_bounded_2_object()(cv, ind)*/true)
      {
        // The curve end is bounded and associated with a valid endpoint.
        const Point_2
            & pt =
                (ind == ARR_MIN_END) ? m_traits->construct_min_vertex_2_object()(
                    cv)
                    : m_traits->construct_max_vertex_2_object()(cv);

        if (ps_x == ARR_INTERIOR && ps_y == ARR_INTERIOR)
          {
            pair_res = _push_red_event(pt, end_attr, ps_x, ps_y, sc);
          }
        else
          {
            pair_res = _push_red_event(cv, ind, end_attr, ps_x, ps_y, sc);
          }

        // Inform the visitor in case we updated an existing event.
        Event *e = pair_res.first;

        CGAL_assertion(e->is_finite());
        //m_visitor->update_event (e, pt, cv, ind, pair_res.second);
      }
    else
      {
        // The curve end is unbounded, insert it into the event queue.
        pair_res = _push_red_event(cv, ind, end_attr, ps_x, ps_y, sc);

        // Inform the visitor in case we updated an existing event.
        Event *e = pair_res.first;

        CGAL_assertion(!e->is_finite());
       // _update_event_at_infinity(e, cv, ind, pair_res.second);
      }

    return;
  }

//-----------------------------------------------------------------------------
// Initialize an event associated with an x-monotone curve end.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_blue_curve_end(
      const X_monotone_curve_2& cv, Arr_curve_end ind, Subcurve* sc)
  {
    // Get the boundary conditions of the curve end.
    const Attribute end_attr = (ind == ARR_MIN_END) ? Base_event::LEFT_END
        : Base_event::RIGHT_END;

    Arr_parameter_space ps_x = m_traits->parameter_space_in_x_2_object()(cv,
        ind);
    Arr_parameter_space ps_y = m_traits->parameter_space_in_y_2_object()(cv,
        ind);

    // Create the corresponding event an push it into the event queue.
    std::pair<Event*, bool> pair_res;

    if (/*m_traits->is_bounded_2_object()(cv, ind)*/true)
      {
        // The curve end is bounded and associated with a valid endpoint.
        const Point_2
            & pt =
                (ind == ARR_MIN_END) ? m_traits->construct_min_vertex_2_object()(
                    cv)
                    : m_traits->construct_max_vertex_2_object()(cv);

        if (ps_x == ARR_INTERIOR && ps_y == ARR_INTERIOR)
          {
            pair_res = _push_blue_event(pt, end_attr, ps_x, ps_y, sc);
          }
        else
          {
            pair_res = _push_blue_event(cv, ind, end_attr, ps_x, ps_y, sc);
          }

        // Inform the visitor in case we updated an existing event.
        Event *e = pair_res.first;

        CGAL_assertion(e->is_finite());
        //m_visitor->update_event (e, pt, cv, ind, pair_res.second);
      }
    else
      {
        // The curve end is unbounded, insert it into the event queue.
        pair_res = _push_blue_event(cv, ind, end_attr, ps_x, ps_y, sc);

        // Inform the visitor in case we updated an existing event.
        Event *e = pair_res.first;

        CGAL_assertion(!e->is_finite());
        //_update_event_at_infinity(e, cv, ind, pair_res.second);
      }

    return;
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::prepare_red_blue_intersection()
  {
    //virtual
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::prepare_red_blue_vertical()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the left of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_red_left_curves()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the left of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_blue_left_curves()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Handle an event that does not have any incident left curves.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_event_without_red_left_curves()
  {
    // Check if the event is a boundary event or not.
    const Arr_parameter_space ps_x = m_currentEvent->parameter_space_in_x();
    const Arr_parameter_space ps_y = m_currentEvent->parameter_space_in_y();

    if (ps_x == ARR_INTERIOR && ps_y == ARR_INTERIOR)
      {
        // The event is associated with a valid point - locate the position of
        // this point on the status line (note this point may be located on a
        // subcurve in the status line).
        const std::pair<Status_line_iterator, bool>& pair_res =
            m_red_statusLine.find_lower(m_currentEvent->point(),
                m_statusLineCurveLess);

        m_red_status_line_insert_hint = pair_res.first;
        m_is_event_on_above = pair_res.second;

        return;
      }

    // We have a boundary event, so we can easily locate a plave for it in the
    // status line.

    if (ps_x == ARR_LEFT_BOUNDARY)
      {
        // We are still sweeping the left boundary, so by the way we have ordered
        // the events in the queue, we know that the new event should be placed
        // above all other subcurves in the status line.
        m_red_status_line_insert_hint = m_red_statusLine.end();
      }
    else
      {
        // Note that an event with a positive boundary condition at x can only
        // represent a right end of a curve.
        CGAL_assertion(ps_x != ARR_RIGHT_BOUNDARY);

        // If the sign of the boundary in y is negative, the event should be
        // inserted below all other subcurves; if it is possitive, the event is
        // above all other subcurves.
        if (ps_y == ARR_BOTTOM_BOUNDARY)
          {
            m_red_status_line_insert_hint = m_red_statusLine.begin();
          }
        else
          {
            CGAL_assertion(ps_y == ARR_TOP_BOUNDARY);
            m_red_status_line_insert_hint = m_red_statusLine.end();
          }
      }

    return;
  }

//-----------------------------------------------------------------------------
// Handle an event that does not have any incident left curves.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_event_without_blue_left_curves()
  {
    // Check if the event is a boundary event or not.
    const Arr_parameter_space ps_x = m_currentEvent->parameter_space_in_x();
    const Arr_parameter_space ps_y = m_currentEvent->parameter_space_in_y();

    if (ps_x == ARR_INTERIOR && ps_y == ARR_INTERIOR)
      {
        // The event is associated with a valid point - locate the position of
        // this point on the status line (note this point may be located on a
        // subcurve in the status line).
        const std::pair<Status_line_iterator, bool>& pair_res =
            m_blue_statusLine.find_lower(m_currentEvent->point(),
                m_statusLineCurveLess);

        m_blue_status_line_insert_hint = pair_res.first;
        m_is_event_on_above = pair_res.second;

        return;
      }

    // We have a boundary event, so we can easily locate a plave for it in the
    // status line.

    if (ps_x == ARR_LEFT_BOUNDARY)
      {
        // We are still sweeping the left boundary, so by the way we have ordered
        // the events in the queue, we know that the new event should be placed
        // above all other subcurves in the status line.
        m_blue_status_line_insert_hint = m_blue_statusLine.end();
      }
    else
      {
        // Note that an event with a positive boundary condition at x can only
        // represent a right end of a curve.
        CGAL_assertion(ps_x != ARR_RIGHT_BOUNDARY);

        // If the sign of the boundary in y is negative, the event should be
        // inserted below all other subcurves; if it is possitive, the event is
        // above all other subcurves.
        if (ps_y == ARR_BOTTOM_BOUNDARY)
          {
            m_blue_status_line_insert_hint = m_blue_statusLine.begin();
          }
        else
          {
            CGAL_assertion(ps_y == ARR_TOP_BOUNDARY);
            m_blue_status_line_insert_hint = m_blue_statusLine.end();
          }
      }

    return;
  }

//-----------------------------------------------------------------------------
// Sort the left subcurves of an event point according to their order in
// their status line (no geometric comprasions are needed).
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_sort_red_left_curves()
  {
    CGAL_assertion(m_currentEvent->has_red_left_curves());

    // Get the first curve associated with the event and its position on the
    // status line. We proceed from this position up the status line until
    // we encounter a subcurve that is not associated with the current event.
    Subcurve *curve = *(m_currentEvent->red_left_curves_begin());
    Status_line_iterator sl_iter = curve->hint();

    CGAL_assertion(*sl_iter == curve);
    sl_iter++;

    for (/*++sl_iter*/; sl_iter != m_red_statusLine.end(); ++sl_iter)
      {
        if (std::find(m_currentEvent->red_left_curves_begin(),
            m_currentEvent->red_left_curves_end(), *sl_iter)
            == m_currentEvent->red_left_curves_end())
          {
            break;
          }
      }

    Status_line_iterator end = sl_iter;

    sl_iter = curve->hint();
    if (sl_iter == m_red_statusLine.begin())
      {
        // In case the lowest subcurve in the status line is associated with the
        // current event, we have the range of (sorted) subcurves ready. We
        // associate this range with the event, so the curves are now sorted
        // according to their vertical positions immediately to the left of the
        // event.
        m_currentEvent->replace_red_left_curves(sl_iter, end);
        return;
      }

    // Go down the status line until we encounter a subcurve that is not
    // associated with the current event.
    --sl_iter;
    for (; sl_iter != m_red_statusLine.begin(); --sl_iter)
      {
        if (std::find(m_currentEvent->red_left_curves_begin(),
            m_currentEvent->red_left_curves_end(), *sl_iter)
            == m_currentEvent->red_left_curves_end())
          {
            // Associate the sorted range of subcurves with the event.
            m_currentEvent->replace_red_left_curves(++sl_iter, end);
            return;
          }
      }

    // Check if the subcurve at the current iterator position should be
    // associated with the current event, and select the (sorted) range of
    // subcurves accordingly.
    if (std::find(m_currentEvent->red_left_curves_begin(),
        m_currentEvent->red_left_curves_end(), *sl_iter)
        == m_currentEvent->red_left_curves_end())
      {
        m_currentEvent->replace_red_left_curves(++sl_iter, end);;
      }
    else
      {
        m_currentEvent->replace_red_left_curves(sl_iter, end);
      }

    return;
  }

//-----------------------------------------------------------------------------
// Sort the left subcurves of an event point according to their order in
// their status line (no geometric comprasions are needed).
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_sort_blue_left_curves()
  {
    CGAL_assertion(m_currentEvent->has_blue_left_curves());

    // Get the first curve associated with the event and its position on the
    // status line. We proceed from this position up the status line until
    // we encounter a subcurve that is not associated with the current event.
    Subcurve *curve = *(m_currentEvent->blue_left_curves_begin());
    Status_line_iterator sl_iter = curve->hint();
    CGAL_assertion(*sl_iter == curve);

    for (++sl_iter; sl_iter != m_blue_statusLine.end(); ++sl_iter)
      {
        if (std::find(m_currentEvent->blue_left_curves_begin(),
            m_currentEvent->blue_left_curves_end(), *sl_iter)
            == m_currentEvent->blue_left_curves_end())
          break;
      }
    Status_line_iterator end = sl_iter;

    sl_iter = curve->hint();
    if (sl_iter == m_blue_statusLine.begin())
      {
        // In case the lowest subcurve in the status line is associated with the
        // current event, we have the range of (sorted) subcurves ready. We
        // associate this range with the event, so the curves are now sorted
        // according to their vertical positions immediately to the left of the
        // event.
        m_currentEvent->replace_blue_left_curves(sl_iter, end);
        return;
      }

    // Go down the status line until we encounter a subcurve that is not
    // associated with the current event.
    --sl_iter;
    for (; sl_iter != m_blue_statusLine.begin(); --sl_iter)
      {
        if (std::find(m_currentEvent->blue_left_curves_begin(),
            m_currentEvent->blue_left_curves_end(), *sl_iter)
            == m_currentEvent->blue_left_curves_end())
          {
            // Associate the sorted range of subcurves with the event.
            m_currentEvent->replace_blue_left_curves(++sl_iter, end);
            return;
          }
      }

    // Check if the subcurve at the current iterator position should be
    // associated with the current event, and select the (sorted) range of
    // subcurves accordingly.
    if (std::find(m_currentEvent->blue_left_curves_begin(),
        m_currentEvent->blue_left_curves_end(), *sl_iter)
        == m_currentEvent->blue_left_curves_end())
      {
        m_currentEvent->replace_blue_left_curves(++sl_iter, end);;
      }
    else
      {
        m_currentEvent->replace_blue_left_curves(sl_iter, end);
      }

    return;
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the right of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_red_right_curves()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the right of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_blue_right_curves()
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Add a subcurve to the right of an event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  bool
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_add_red_curve_to_right(
      Event* event, Subcurve* curve, bool /* overlap_exist */)
  {
    //virtual
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  bool
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_add_blue_curve_to_right(
      Event* event, Subcurve* curve, bool /* overlap_exist */)
  {
    //virtual
  }

//-----------------------------------------------------------------------------
// Remove a curve from the status line.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_remove_curve_from_red_status_line(
      Subcurve *sc)
  {
    // Get the position of the subcurve on the status line.
    Status_line_iterator sl_iter = sc->hint();
    CGAL_assertion(sl_iter != m_red_statusLine.end());

    // The position of the next event can be right after the deleted subcurve.
    m_red_status_line_insert_hint = sl_iter;
    ++m_red_status_line_insert_hint;

    // Erase the subcurve from the status line.
    m_red_statusLine.erase(sl_iter);
    ////MY_CGAL_PRINT      ("remove_curve_from_red_status_line Done\n";)
    return;
  }

//-----------------------------------------------------------------------------
// Remove a curve from the status line.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_remove_curve_from_blue_status_line(
      Subcurve *sc)
  {
    // Get the position of the subcurve on the status line.
    Status_line_iterator sl_iter = sc->hint();
    CGAL_assertion(sl_iter != m_blue_statusLine.end());

    // The position of the next event can be right after the deleted subcurve.
    m_blue_status_line_insert_hint = sl_iter;
    ++m_blue_status_line_insert_hint;

    // Erase the subcurve from the status line.
    m_blue_statusLine.erase(sl_iter);
    ////MY_CGAL_PRINT  ("remove_curve_from_blue_status_line Done\n";)
    return;
  }

//-----------------------------------------------------------------------------
// Allocate an event object associated with a valid point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_allocate_event(
      const Point_2& pt, Attribute type, Arr_parameter_space ps_x,
      Arr_parameter_space ps_y)
  {
    // Allocate the event.
    Event *e = m_eventAlloc.allocate(1);
    m_eventAlloc.construct(e, m_masterEvent);
    e->init(pt, type, ps_x, ps_y);

    // Insert it to the set of allocated events.
    m_allocated_events.insert(e);
    return (e);
  }

//-----------------------------------------------------------------------------
// Allocate an event at infinity, which is not associated with a valid point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_allocate_event_at_infinity(
      Attribute type, Arr_parameter_space ps_x, Arr_parameter_space ps_y)
  {
    Event *e = m_eventAlloc.allocate(1);
    m_eventAlloc.construct(e, m_masterEvent);
    e->init_at_infinity(type, ps_x, ps_y);

    m_allocated_events.insert(e);
    return (e);
  }

//-----------------------------------------------------------------------------
// Push a finite event point into the event queue.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  std::pair<
      typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*, bool>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_push_red_event(
      const Point_2& pt, Attribute type, Arr_parameter_space ps_x,
      Arr_parameter_space ps_y, Subcurve* sc)
  {

    // Look for the point in the event queue.
    Event* e;
    m_queueEventLess.set_parameter_space_in_x(ps_x);
    m_queueEventLess.set_parameter_space_in_y(ps_y);

    const std::pair<Event_queue_iterator, bool>& pair_res =
        m_queue->find_lower(pt, m_queueEventLess);

    const bool exist = pair_res.second;

    if (!exist)
      {
        // The point is not found in the event queue - create a new event and
        // insert it into the queue.
        e = _allocate_event(pt, type, ps_x, ps_y);
      }
    else
      {
        // The event associated with the given point already exists in the queue,
        // so we just have to update it.
        e = *(pair_res.first);
        CGAL_assertion(e->is_finite());

        e->set_attribute(type);
      }

    // If we are given a subcurve that the event represents one of its
    // endpoints, update the event and the subcurve records accordingly.
    // Note that this must be done before we actually insert the new event
    // into the event queue.
    if (sc != NULL)
      {
        if (type == Base_event::LEFT_END)
          {
            sc->set_left_event(e);
            _add_red_curve_to_right(e, sc);
          }
        else
          {
            CGAL_assertion(type == Base_event::RIGHT_END);
            sc->set_right_event(e);
            e->add_red_curve_to_left(sc);
          }
      }

    if (!exist)
      {
        // Insert the new event into the queue using the hint we got when we
        // looked for it.
        m_queue->insert_before(pair_res.first, e);
      }
    ////MY_CGAL_PRINT_NEW_EVENT(pt, e);

    // Return the resulting event and a flag indicating whether we have created
    // a new event.
    return (std::make_pair(e, !exist));
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  std::pair<
      typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*, bool>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_push_blue_event(
      const Point_2& pt, Attribute type, Arr_parameter_space ps_x,
      Arr_parameter_space ps_y, Subcurve* sc)
  {
    // Look for the point in the event queue.
    Event* e;
    m_queueEventLess.set_parameter_space_in_x(ps_x);
    m_queueEventLess.set_parameter_space_in_y(ps_y);

    const std::pair<Event_queue_iterator, bool>& pair_res =
        m_queue->find_lower(pt, m_queueEventLess);
    const bool exist = pair_res.second;

    if (!exist)
      {
        // The point is not found in the event queue - create a new event and
        // insert it into the queue.
        e = _allocate_event(pt, type, ps_x, ps_y);
      }
    else
      {
        // The event associated with the given point already exists in the queue,
        // so we just have to update it.
        e = *(pair_res.first);
        CGAL_assertion(e->is_finite());

        e->set_attribute(type);
      }

    // If we are given a subcurve that the event represents one of its
    // endpoints, update the event and the subcurve records accordingly.
    // Note that this must be done before we actually insert the new event
    // into the event queue.
    if (sc != NULL)
      {
        if (type == Base_event::LEFT_END)
          {
            sc->set_left_event(e);
            _add_blue_curve_to_right(e, sc);
          }
        else
          {
            CGAL_assertion(type == Base_event::RIGHT_END);
            sc->set_right_event(e);
            e->add_blue_curve_to_left(sc);
          }
      }

    if (!exist)
      {
        // Insert the new event into the queue using the hint we got when we
        // looked for it.
        m_queue->insert_before(pair_res.first, e);
      }
    ////MY_CGAL_PRINT_NEW_EVENT(pt, e);

    // Return the resulting event and a flag indicating whether we have created
    // a new event.
    return (std::make_pair(e, !exist));
  }

//-----------------------------------------------------------------------------
// Push an event point associated with a curve end into the event queue.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  std::pair<
      typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*, bool>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_push_red_event(
      const X_monotone_curve_2& cv, Arr_curve_end ind, Attribute type,
      Arr_parameter_space ps_x, Arr_parameter_space ps_y, Subcurve* sc)
  {
    // Look for the curve end in the event queue.
    Event* e;

    m_queueEventLess.set_parameter_space_in_x(ps_x);
    m_queueEventLess.set_parameter_space_in_y(ps_y);
    m_queueEventLess.set_index(ind);

    const std::pair<Event_queue_iterator, bool>& pair_res =
        m_queue->find_lower(cv, m_queueEventLess);
    const bool exist = pair_res.second;

    if (!exist)
      {
        // The curve end is not found in the event queue - create a new event and
        // insert it into the queue.
        if (/*m_traits->is_bounded_2_object()(cv, ind)*/true)
          {
            // The curve end is not unbounded, so it is associated with a valid
            // point.
            const Point_2
                & pt =
                    (ind == ARR_MIN_END) ? m_traits->construct_min_vertex_2_object()(
                        cv)
                        : m_traits->construct_max_vertex_2_object()(cv);

            e = _allocate_event(pt, type, ps_x, ps_y);
          }
        else
          {
            // The curve end is unbounded, so we create an event at infinity.
            e = _allocate_event_at_infinity(type, ps_x, ps_y);
          }
      }
    else
      {
        // The event associated with the given curve end already exists in the
        // queue, so we just have to update it.
        e = *(pair_res.first);
        CGAL_assertion(e->parameter_space_in_x() == ps_x
            && e->parameter_space_in_y() == ps_y);

        e->set_attribute(type);
      }

    // If we are given a subcurve that the event represents one of its
    // endpoints, update the event and the subcurve records accordingly.
    // Note that this must be done before we actually insert the new event
    // into the event queue.
    if (sc != NULL)
      {
        if (type == Base_event::LEFT_END)
          {
            sc->set_left_event(e);
            _add_red_curve_to_right(e, sc);
          }
        else
          {
            CGAL_assertion(type == Base_event::RIGHT_END);
            sc->set_right_event(e);
            e->add_red_curve_to_left(sc);
          }
      }

    if (!exist)
      {
        // Insert the new event into the queue using the hint we got when we
        // looked for it.
        m_queue->insert_before(pair_res.first, e);
      }

    return (std::make_pair(e, !exist));
  }

//-----------------------------------------------------------------------------
// Push an event point associated with a curve end into the event queue.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  std::pair<
      typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::Event*, bool>
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_push_blue_event(
      const X_monotone_curve_2& cv, Arr_curve_end ind, Attribute type,
      Arr_parameter_space ps_x, Arr_parameter_space ps_y, Subcurve* sc)
  {
    // Look for the curve end in the event queue.
    Event* e;

    m_queueEventLess.set_parameter_space_in_x(ps_x);
    m_queueEventLess.set_parameter_space_in_y(ps_y);
    m_queueEventLess.set_index(ind);

    const std::pair<Event_queue_iterator, bool>& pair_res =
        m_queue->find_lower(cv, m_queueEventLess);
    const bool exist = pair_res.second;

    if (!exist)
      {
        // The curve end is not found in the event queue - create a new event and
        // insert it into the queue.
        if (/*m_traits->is_bounded_2_object()(cv, ind)*/true)
          {
            // The curve end is not unbounded, so it is associated with a valid
            // point.
            const Point_2
                & pt =
                    (ind == ARR_MIN_END) ? m_traits->construct_min_vertex_2_object()(
                        cv)
                        : m_traits->construct_max_vertex_2_object()(cv);

            e = _allocate_event(pt, type, ps_x, ps_y);
          }
        else
          {
            // The curve end is unbounded, so we create an event at infinity.
            e = _allocate_event_at_infinity(type, ps_x, ps_y);
          }
      }
    else
      {
        // The event associated with the given curve end already exists in the
        // queue, so we just have to update it.
        e = *(pair_res.first);
        CGAL_assertion(e->parameter_space_in_x() == ps_x
            && e->parameter_space_in_y() == ps_y);

        e->set_attribute(type);
      }

    // If we are given a subcurve that the event represents one of its
    // endpoints, update the event and the subcurve records accordingly.
    // Note that this must be done before we actually insert the new event
    // into the event queue.
    if (sc != NULL)
      {
        if (type == Base_event::LEFT_END)
          {
            sc->set_left_event(e);
            _add_blue_curve_to_right(e, sc);
          }
        else
          {
            CGAL_assertion(type == Base_event::RIGHT_END);
            sc->set_right_event(e);
            e->add_blue_curve_to_left(sc);
          }
      }

    if (!exist)
      {
        // Insert the new event into the queue using the hint we got when we
        // looked for it.
        m_queue->insert_before(pair_res.first, e);
      }

    return (std::make_pair(e, !exist));
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::proceed_according_to_vertex_type()
  {
    //virtual
  }

CGAL_END_NAMESPACE

#endif

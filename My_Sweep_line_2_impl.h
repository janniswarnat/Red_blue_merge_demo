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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2/Sweep_line_2_impl.h $
// $Id: Sweep_line_2_impl.h 41272 2007-12-20 12:50:29Z efif $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Efi Fogel       <efif@post.tau.ac.il>
//                 (based on old version by Tali Zvi)

#ifndef MY_CGAL_SWEEP_LINE_2_IMPL_H
#define MY_CGAL_SWEEP_LINE_2_IMPL_H

/*! \file
 * Member-function definitions of the Sweep_line_2 class-template.
 */

CGAL_BEGIN_NAMESPACE

////-----------------------------------------------------------------------------
//// Initialize the data structures for the sweep-line algorithm.
////
//template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
//  void
//  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_structures()
//  {
//    // Initailize the structures maintained by the base sweep-line class.
//    Base::_init_structures();
//
//    // Resize the hash to be O(2*n), where n is the number of input curves.
//    m_curves_pair_set.resize (2 * this->m_num_of_subCurves);
//  }

//-----------------------------------------------------------------------------
// Initialize the data structures for the sweep-line algorithm.
// adapted by Jannis Warnat
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_init_curve_pairs()
  {
    // Initailize the structures maintained by the base sweep-line class.
    //Base::_init_blue_structures();

    // Resize the hash to be O(2*n), where n is the number of input curves.
        m_curves_pair_set.resize(2 * (this->m_num_of_red_subCurves
            + this->m_num_of_blue_subCurves));

  }

//-----------------------------------------------------------------------------
// Complete the sweep (complete the data structures).
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_complete_sweep()
  {
    // Complete the sweep process using base sweep-line class.
    Base::_complete_sweep();

    // Clean the set of curve pairs for which we have computed intersections.
    m_curves_pair_set.clear();


    //  // Free all overlapping subcurves we have created.
    //  Subcurve_iterator   itr;
    //  for (itr = m_overlap_subCurves.begin();
    //       itr != m_overlap_subCurves.end();
    //       ++itr)
    //  {
    //    this->m_subCurveAlloc.destroy(*itr);
    //    this->m_subCurveAlloc.deallocate(*iArr, Tr, 1);
    //  }
    //
    //  m_overlap_subCurves.clear();
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the left of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_red_left_curves()
  {
    ////MY_CGAL_PRINT("Handling red left curve" << std::endl;);

    this->m_is_event_on_above = false;

    if (!this->m_currentEvent->has_red_left_curves())
      {
        // In case the current event has no left subcurves incident to it, we have
        // to locate a place for it in the status line.
        ////MY_CGAL_PRINT(" - handling special case " << std::endl;);
        this->_handle_event_without_red_left_curves();

        Status_line_iterator sl_pos = this->m_red_status_line_insert_hint;

        if (this->m_is_event_on_above)
          {
            // The current event point starts at the interior of a subcurve that
            // already exists in the status line (this may also indicate an overlap).
            if (!this->m_currentEvent->has_red_right_curves())
              {
                // The event is an isolated point.
                if (this->m_currentEvent->is_query())
                  {
                    // In case of a query point, just notify the visitor about it.
                    this->m_is_event_on_above = true;
                    //this->m_visitor->before_handle_event (this->m_currentEvent);
                    return;
                  }

                // In case of an isolated action point, mark the point at a "weak"
                // intersection.
                CGAL_assertion(this->m_currentEvent->is_action());
                this->m_currentEvent->set_weak_intersection();
              }

            // Obtain the subcurve that contains the current event, and add it to
            // the left curves incident to the event.
            Subcurve
                *sc =
                    static_cast<Subcurve*> (*(this-> m_red_status_line_insert_hint));
            const X_monotone_curve_2& last_curve = sc->last_curve();

            this->m_currentEvent->set_weak_intersection();
            //this->m_visitor->update_event(this->m_currentEvent, sc);
            this->m_currentEvent->add_red_curve_to_left(sc);

            // If necessary, add the subcurves as a right incident curve as well.
            // We also check for overlaps.
            bool is_overlap = _add_red_curve_to_right(this->m_currentEvent, sc);

            //this->m_traits->split_2_object() (last_curve,
            this->m_traits->split_2_object()(last_curve,
                this->m_currentEvent->point(), sub_cv1, sub_cv2);

            ++(this->m_red_status_line_insert_hint);

            if (is_overlap)
              {
                // Handle overlaps.
                //this->m_visitor->before_handle_event (this->m_currentEvent);
                //this->m_visitor->add_subcurve (sub_cv1, sc);
                this->m_red_statusLine.erase(sl_pos);
                return;
              }

          }
        else
          {
            // The event is not located on any subcurve.
            //this->m_visitor->before_handle_event(this->m_currentEvent);
            return;
          }
      }
    //
    //    //MY_CGAL_PRINT("left curves before sorting: "<<"\n";);
    //    MY_CGAL_SL_DEBUG(if (this->m_currentEvent->red_left_curves_begin() !=
    //            this->m_currentEvent->red_left_curves_end() )
    //          {
    //            this->m_currentEvent->Print();
    //          });

    _fix_overlap_subcurves_from_red();
    this->_sort_red_left_curves();

    //this->m_visitor->before_handle_event(this->m_currentEvent);

    //    //MY_CGAL_PRINT("left curves after sorting: "<<"\n";);
    //    MY_CGAL_SL_DEBUG(if (this->m_currentEvent->red_left_curves_begin() !=
    //            this->m_currentEvent->red_left_curves_end() )
    //          {
    //            this->m_currentEvent->Print();
    //          });

    // Check if the curve should be removed for good.
    bool remove_for_good = false;

    Event_subcurve_iterator left_iter =
        this->m_currentEvent->red_left_curves_begin();

    while (left_iter != this->m_currentEvent->red_left_curves_end())
      {
        Subcurve *leftCurve = *left_iter;

        if ((Event*) leftCurve->right_event() == this->m_currentEvent)
          {
            // we are done with that subcurve (current event point is his right
            // end point) so we remove it from the status line for good.
            remove_for_good = true;
            //this->m_visitor->add_subcurve(leftCurve->last_curve(), leftCurve);
          }
        else
          {
            // current event splits the subcurve.
            // following is adapted by Jannis Warnat
            // preserve the left curve
            const X_monotone_curve_2 &lastCurve = leftCurve->last_curve();
            Point_2 left = leftCurve->last_curve().left();
            Point_2 right = leftCurve->last_curve().right();

            this->m_traits->split_2_object()(lastCurve,
                this->m_currentEvent->point(), sub_cv1, sub_cv2);

            sub_cv2.set_right_ex_point(right);
            sub_cv2.set_left_ex_point(this->m_currentEvent->point());

            leftCurve->set_last_curve(sub_cv2);
            leftCurve->set_left_event(this->m_currentEvent);

            //insert the left part of the splitted curve
            if (leftCurve->is_boundary() || leftCurve->insert_sub_cv1())
              {
                //extract left vertex
                Vertex_const_handle purple_left;
                try
                  {
                    purple_left = object_cast<Vertex_const_handle> (
                        left.purple_object());
                  }
                catch (Bad_object_cast)
                  {
                    CGAL_assertion(false);
                  }
                Vertex_handle non_const_left =
                    this->get_purple()->non_const_handle(purple_left);

                //extract right vertex
                Vertex_const_handle purple_right;
                try
                  {
                    purple_right = object_cast<Vertex_const_handle> (
                        this->m_currentEvent->point().purple_object());
                  }
                catch (Bad_object_cast)
                  {
                    CGAL_assertion(false);
                  }
                Vertex_handle non_const_right =
                    this->get_purple()->non_const_handle(purple_right);

                //insert the curve into the purple arrangement
                Halfedge_handle he = this->get_purple()->insert_at_vertices(
                    sub_cv1.base(), non_const_left, non_const_right);
                CGAL_assertion(he->direction() == ARR_LEFT_TO_RIGHT);
                leftCurve->set_insert_sub_cv1(false);
              }
          }
        ++left_iter;

        //remove curve from the status line (also checks intersection
        //between the neighbouring curves,only if the curve is removed for good)
        _remove_curve_from_red_status_line(leftCurve, remove_for_good);
      }
    return;
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_blue_left_curves()
  {
    ////MY_CGAL_PRINT("Handling blue left curve" << std::endl;);

    this->m_is_event_on_above = false;

    if (!this->m_currentEvent->has_blue_left_curves())
      {
        // In case the current event has no left subcurves incident to it, we have
        // to locate a place for it in the status line.
        ////MY_CGAL_PRINT(" - handling special case " << std::endl;);
        this->_handle_event_without_blue_left_curves();

        Status_line_iterator sl_pos = this->m_blue_status_line_insert_hint;

        if (this->m_is_event_on_above)
          {
            // The current event point starts at the interior of a subcurve that
            // already exists in the status line (this may also indicate an overlap).
            if (!this->m_currentEvent->has_blue_right_curves())
              {
                // The event is an isolated point.
                if (this->m_currentEvent->is_query())
                  {
                    // In case of a query point, just notify the visitor about it.
                    this->m_is_event_on_above = true;
                    //this->m_visitor->before_handle_event (this->m_currentEvent);
                    return;
                  }

                // In case of an isolated action point, mark the point at a "weak"
                // intersection.
                CGAL_assertion(this->m_currentEvent->is_action());
                this->m_currentEvent->set_weak_intersection();
              }

            // Obtain the subcurve that contains the current event, and add it to
            // the left curves incident to the event.
            Subcurve
                *sc =
                    static_cast<Subcurve*> (*(this-> m_blue_status_line_insert_hint));
            const X_monotone_curve_2& last_curve = sc->last_curve();

            this->m_currentEvent->set_weak_intersection();
            //this->m_visitor->update_event(this->m_currentEvent, sc);
            this->m_currentEvent->add_blue_curve_to_left(sc);

            // If necessary, add the subcurves as a right incident curve as well.
            // We also check for overlaps.
            bool is_overlap =
                _add_blue_curve_to_right(this->m_currentEvent, sc);

            this->m_traits->split_2_object()(last_curve,

            this->m_currentEvent->point(), sub_cv1, sub_cv2);

            ++(this->m_blue_status_line_insert_hint);

            if (is_overlap)
              {
                // Handle overlaps.
                //this->m_visitor->before_handle_event (this->m_currentEvent);
                //this->m_visitor->add_subcurve (sub_cv1, sc);
                this->m_blue_statusLine.erase(sl_pos);
                return;
              }

          }
        else
          {
            // The event is not located on any subcurve.
            //this->m_visitor->before_handle_event(this->m_currentEvent);
            return;
          }
      }

    //  //MY_CGAL_PRINT("left curves before sorting: "<<"\n";);
    //  MY_CGAL_SL_DEBUG(if (this->m_currentEvent->left_curves_begin() !=
    //                    this->m_currentEvent->left_curves_end() )
    //                {
    //                  this->m_currentEvent->Print();
    //                });
    _fix_overlap_subcurves_from_blue();
    this->_sort_blue_left_curves();
    //this->m_visitor->before_handle_event(this->m_currentEvent);

    //  //MY_CGAL_PRINT("left curves after sorting: "<<"\n";);
    //  MY_CGAL_SL_DEBUG(if (this->m_currentEvent->left_curves_begin() !=
    //                    this->m_currentEvent->left_curves_end() )
    //                {
    //                  this->m_currentEvent->Print();
    //                });

    // Check if the curve should be removed for good.
    bool remove_for_good = false;

    Event_subcurve_iterator left_iter =
        this->m_currentEvent->blue_left_curves_begin();

    while (left_iter != this->m_currentEvent->blue_left_curves_end())
      {
        Subcurve *leftCurve = *left_iter;

        if ((Event*) leftCurve->right_event() == this->m_currentEvent)
          {
            // we are done with that subcurve (current event point is his right
            // end point) so we remove it from the status line for good.
            remove_for_good = true;
            //this->m_visitor->add_subcurve(leftCurve->last_curve(), leftCurve);
          }
        else
          {
            // current event splits the subcurve.
            // following is adapted by Jannis Warnat
            // preserve the left curve
            const X_monotone_curve_2 &lastCurve = leftCurve->last_curve();
            Point_2 left = leftCurve->last_curve().left();
            Point_2 right = leftCurve->last_curve().right();

            this->m_traits->split_2_object()(lastCurve,
                this->m_currentEvent->point(), sub_cv1, sub_cv2);

            sub_cv2.set_right_ex_point(right);
            sub_cv2.set_left_ex_point(this->m_currentEvent->point());

            leftCurve->set_last_curve(sub_cv2);
            leftCurve->set_left_event(this->m_currentEvent);

            //insert the left part of the splitted curve
            if (leftCurve->is_boundary() || leftCurve->insert_sub_cv1())
              {
                //extract left vertex
                Vertex_const_handle purple_left;
                try
                  {
                    purple_left = object_cast<Vertex_const_handle> (
                        left.purple_object());
                  }
                catch (Bad_object_cast)
                  {
                    CGAL_assertion(false);
                  }
                Vertex_handle non_const_left =
                    this->get_purple()->non_const_handle(purple_left);

                //extract right vertex
                Vertex_const_handle purple_right;
                try
                  {
                    purple_right = object_cast<Vertex_const_handle> (
                        this->m_currentEvent->point().purple_object());
                  }
                catch (Bad_object_cast)
                  {
                    CGAL_assertion(false);
                  }
                Vertex_handle non_const_right =
                    this->get_purple()->non_const_handle(purple_right);

                //insert the curve into the purple arrangement
                Halfedge_handle he = this->get_purple()->insert_at_vertices(
                    sub_cv1.base(), non_const_left, non_const_right);
                CGAL_assertion(he->direction() == ARR_LEFT_TO_RIGHT);
                leftCurve->set_insert_sub_cv1(false);
              }
          }
        ++left_iter;

        //remove curve from the status line (also checks intersection
        //between the neighbouring curves,only if the curve is removed for good)
        _remove_curve_from_blue_status_line(leftCurve, remove_for_good);

      }
    ////MY_CGAL_PRINT( "Handling blue left curve END" << std::endl;);

    return;
  }

//-----------------------------------------------------------------------------
// Handle the subcurves to the right of the current event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_red_right_curves()
  {
    ////MY_CGAL_PRINT("Handling red_right curves (" ;);
    ////MY_CGAL_PRINT(")\n";);

    if (!this->m_currentEvent->has_red_right_curves())
      return;

    // Loop over the curves to the right of the status line and handle them:
    // - If we are at the beginning of the curve, we insert it to the status
    //   line, then we look if it intersects any of its neighbors.
    // - If we are at an intersection point between two curves, we add them
    //   to the status line and attempt to intersect them with their neighbors
    // - We also check to see if the two intersect again to the right of the
    //   point.

    Event_subcurve_iterator currentOne =
        this->m_currentEvent->red_right_curves_begin();
    Event_subcurve_iterator rightCurveEnd =
        this->m_currentEvent->red_right_curves_end();

    ////MY_CGAL_PRINT_INSERT(*currentOne);

    Status_line_iterator slIter = this->m_red_statusLine.insert_before(
        this->m_red_status_line_insert_hint, *currentOne);
    ((Subcurve*) (*currentOne))->set_hint(slIter);

    //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););
    if (slIter != this->m_red_statusLine.begin())
      {
        //  get the previous curve in the y-str
        Status_line_iterator prev = slIter;
        --prev;
        //_intersect(static_cast<Subcurve*>(*prev),
        //           static_cast<Subcurve*>(*slIter));
      }

    Event_subcurve_iterator prevOne = currentOne;
    ++currentOne;
    while (currentOne != rightCurveEnd)
      {
        ////MY_CGAL_PRINT_INSERT(*currentOne);
        slIter = this->m_red_statusLine.insert_before(
            this->m_red_status_line_insert_hint, *currentOne);
        ((Subcurve*) (*currentOne))->set_hint(slIter);

        //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););

        // If the two curves used to be neighbours before, we do not need to
        // intersect them again.
        //    if (!this->m_currentEvent->are_left_neighbours
        //        (static_cast<Subcurve*>(*currentOne),
        //         static_cast<Subcurve*>(*prevOne)))
        //    {
        //      _intersect(*prevOne, *currentOne);
        //    }

        prevOne = currentOne;
        ++currentOne;
      }

    //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););

    //the next Subcurve at the status line
    ++slIter;
    //  if ( slIter != this->m_statusLine.end() )
    //    _intersect( static_cast<Subcurve*>(*prevOne),
    //                static_cast<Subcurve*>(*slIter));
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_blue_right_curves()
  {
    ////MY_CGAL_PRINT("Handling blue_right curves (" ;);
    //MY_CGAL_SL_DEBUG(this->PrintEvent(this->m_currentEvent););
    ////MY_CGAL_PRINT(")\n";);
    ////MY_CGAL_PRINT("begin _handle_blue_right_curves()");
    if (!this->m_currentEvent->has_blue_right_curves())
      return;

    // Loop over the curves to the right of the status line and handle them:
    // - If we are at the beginning of the curve, we insert it to the status
    //   line, then we look if it intersects any of its neighbors.
    // - If we are at an intersection point between two curves, we add them
    //   to the status line and attempt to intersect them with their neighbors
    // - We also check to see if the two intersect again to the right of the
    //   point.

    Event_subcurve_iterator currentOne =
        this->m_currentEvent->blue_right_curves_begin();
    Event_subcurve_iterator rightCurveEnd =
        this->m_currentEvent->blue_right_curves_end();

    ////MY_CGAL_PRINT_INSERT(*currentOne);

    Status_line_iterator slIter = this->m_blue_statusLine.insert_before(
        this->m_blue_status_line_insert_hint, *currentOne);
    ((Subcurve*) (*currentOne))->set_hint(slIter);

    //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););
    if (slIter != this->m_blue_statusLine.begin())
      {
        //  get the previous curve in the y-str
        Status_line_iterator prev = slIter;
        --prev;
        //_intersect(static_cast<Subcurve*>(*prev),
        //           static_cast<Subcurve*>(*slIter));
      }

    Event_subcurve_iterator prevOne = currentOne;
    ++currentOne;
    while (currentOne != rightCurveEnd)
      {
        ////MY_CGAL_PRINT_INSERT(*currentOne);
        slIter = this->m_blue_statusLine.insert_before(
            this->m_blue_status_line_insert_hint, *currentOne);
        ((Subcurve*) (*currentOne))->set_hint(slIter);

        //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););

        // If the two curves used to be neighbours before, we do not need to
        // intersect them again.
        //    if (!this->m_currentEvent->are_left_neighbours
        //        (static_cast<Subcurve*>(*currentOne),
        //         static_cast<Subcurve*>(*prevOne)))
        //    {
        //      _intersect(*prevOne, *currentOne);
        //    }

        prevOne = currentOne;
        ++currentOne;
      }

    //MY_CGAL_SL_DEBUG(this->PrintStatusLine(););

    //the next Subcurve at the status line
    ++slIter;
    //  if ( slIter != this->m_statusLine.end() )
    //    _intersect( static_cast<Subcurve*>(*prevOne),
    //                static_cast<Subcurve*>(*slIter));
  }

//-----------------------------------------------------------------------------
// Add a subcurve to the right of an event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  bool
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_add_red_curve_to_right(
      Event* event, Subcurve* curve, bool overlap_exist)
  {
    Event_subcurve_iterator iter;

    for (iter = event->red_right_curves_begin(); iter
        != event->red_right_curves_end(); ++iter)
      {
        if ((curve == *iter) || (*iter)->is_inner_node(curve))
          {
            return false;
          }
        if ((curve)->is_inner_node(*iter))
          {
            *iter = curve;
            return false;
          }

        if ((curve)->has_common_leaf(*iter))
          {
            std::list<Base_subcurve*> list_of_sc;
            curve->distinct_nodes(*iter, std::back_inserter(list_of_sc));

            typename std::list<Base_subcurve*>::iterator sc_iter;
            for (sc_iter = list_of_sc.begin(); sc_iter != list_of_sc.end(); ++sc_iter)
              {
                _add_red_curve_to_right(event,
                    static_cast<Subcurve*> (*sc_iter));
              }
            return true;
          }
      }
    std::pair<bool, Event_subcurve_iterator> pair_res =
        event->add_red_curve_to_right(curve, this->m_traits);

    if (!pair_res.first)
      // No overlap occurs:
      return (false);

    _handle_overlap_from_red(event, curve, pair_res.second, overlap_exist);

    // Inidicate that an overlap has occured:
    return (true);
  }

//-----------------------------------------------------------------------------
// Add a subcurve to the right of an event point.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  bool
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_add_blue_curve_to_right(
      Event* event, Subcurve* curve, bool overlap_exist)
  {
    Event_subcurve_iterator iter;

    for (iter = event->blue_right_curves_begin(); iter
        != event->blue_right_curves_end(); ++iter)
      {
        if ((curve == *iter) || (*iter)->is_inner_node(curve))
          {
            return false;
          }
        if ((curve)->is_inner_node(*iter))
          {
            *iter = curve;
            return false;
          }

        if ((curve)->has_common_leaf(*iter))
          {
            std::list<Base_subcurve*> list_of_sc;
            curve->distinct_nodes(*iter, std::back_inserter(list_of_sc));

            typename std::list<Base_subcurve*>::iterator sc_iter;
            for (sc_iter = list_of_sc.begin(); sc_iter != list_of_sc.end(); ++sc_iter)
              {
                _add_blue_curve_to_right(event,
                    static_cast<Subcurve*> (*sc_iter));
              }
            return true;
          }
      }
    std::pair<bool, Event_subcurve_iterator> pair_res =
        event->add_blue_curve_to_right(curve, this->m_traits);

    if (!pair_res.first)
      // No overlap occurs:
      return (false);

    _handle_overlap_from_blue(event, curve, pair_res.second, overlap_exist);

    // Inidicate that an overlap has occured:
    return (true);
  }

//-----------------------------------------------------------------------------
// Remove a curve from the status line.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_remove_curve_from_red_status_line(
      Subcurve *leftCurve, bool remove_for_good)

  {
    //  //MY_CGAL_PRINT("remove_curve_from_status_line\n";);
    //  MY_CGAL_SL_DEBUG(this->PrintStatusLine(););
    //  MY_CGAL_SL_DEBUG(leftCurve->Print(););

    Status_line_iterator sliter = leftCurve->hint();
    this->m_red_status_line_insert_hint = sliter;
    ++(this->m_red_status_line_insert_hint);

    if (!remove_for_good)
      {
        // the subcurve is not removed for good, so we dont need to intersect
        // his neighbours after its removal.
        this->m_red_statusLine.erase(sliter);
        ////MY_CGAL_PRINT("remove_curve_from_red_status_line Done\n";)
        return;
      }

    // the subcurve will be removed for good from the stauts line, we need
    // to check for intersection between his two neighbours (below and above him)
    // but we need to make sure that its not the first or last subcurve
    // at the status line.
    CGAL_assertion(sliter != this->m_red_statusLine.end());
    Status_line_iterator lastOne = this->m_red_statusLine.end();
    --lastOne;

    if (sliter != this->m_red_statusLine.begin() && sliter != lastOne)
      {
        Status_line_iterator prev = sliter;
        --prev;
        Status_line_iterator next = sliter;
        ++next;

        // intersect *next with  *prev
        //    _intersect(static_cast<Subcurve*>(*prev),
        //               static_cast<Subcurve*>(*next));
      }
    this->m_red_statusLine.erase(sliter);
    ////MY_CGAL_PRINT("remove_curve_from_red_status_line Done\n";)
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_remove_curve_from_blue_status_line(
      Subcurve *leftCurve, bool remove_for_good)

  {
    //  //MY_CGAL_PRINT("remove_curve_from_status_line\n";);
    //  MY_CGAL_SL_DEBUG(this->PrintStatusLine(););
    //  MY_CGAL_SL_DEBUG(leftCurve->Print(););

    Status_line_iterator sliter = leftCurve->hint();
    this->m_blue_status_line_insert_hint = sliter;
    ++(this->m_blue_status_line_insert_hint);

    if (!remove_for_good)
      {
        // the subcurve is not removed for good, so we dont need to intersect
        // his neighbours after its removal.
        this->m_blue_statusLine.erase(sliter);
        ////MY_CGAL_PRINT("remove_curve_from_blue_status_line Done\n";)
        return;
      }

    // the subcurve will be removed for good from the stauts line, we need
    // to check for intersection between his two neighbours (below and above him)
    // but we need to make sure that its not the first or last subcurve
    // at the status line.
    CGAL_assertion(sliter != this->m_blue_statusLine.end());
    Status_line_iterator lastOne = this->m_blue_statusLine.end();
    --lastOne;

    if (sliter != this->m_blue_statusLine.begin() && sliter != lastOne)
      {
        Status_line_iterator prev = sliter;
        --prev;
        Status_line_iterator next = sliter;
        ++next;

        // intersect *next with  *prev
        //    _intersect(static_cast<Subcurve*>(*prev),
        //               static_cast<Subcurve*>(*next));
      }
    this->m_blue_statusLine.erase(sliter);
    ////MY_CGAL_PRINT("remove_curve_from_blue_status_line Done\n";)
  }

//-----------------------------------------------------------------------------
// Compute intersections between the two given curves.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_intersect_from_red(
      Subcurve *c1, Subcurve *c2)
  {
    MY_CGAL_SL_PRINT("begin _intersect_from_red()");

    if (c2 == 0)
      {
        return;
      }

    // look up for (c1,c2) in the table and insert if doesnt exist
    Curve_pair
    cv_pair(c1, c2);
    if (!(m_curves_pair_set.insert(cv_pair)).second)
      {
        //MY_CGAL_SL_PRINT("already computed intersection");
        return; //the curves have already been checked for intersection
      }
    else
      {
        //MY_CGAL_SL_PRINT("compute new intersection");
      }

    float load_factor = static_cast<float> (m_curves_pair_set.size())
        / m_curves_pair_set.bucket_count();
    // after lot of benchemarks, keeping load_factor<=6 is optimal
    if (load_factor > 6.0f)
      m_curves_pair_set.resize(m_curves_pair_set.size() * 6);

    vector_inserter
    vi(m_x_objects);
    vector_inserter
    vi_end(m_x_objects);
    vi_end = this->m_traits->intersect_2_object()(c1->last_curve(),
        c2->last_curve(), vi);

    if (vi == vi_end)
      {
        ////MY_CGAL_PRINT("no intersection...\n";);
        return; // no intersection at all
      }

    // The two subCurves may start at the same point, in that case we ignore the
    // first intersection point (if we got to that stage, they cannot  overlap).
    if (reinterpret_cast<Event*> (c1->left_event()) == this->m_currentEvent
        && reinterpret_cast<Event*> (c2->left_event()) == this->m_currentEvent)
      {
        ////MY_CGAL_PRINT(" [Skipping common left endpoint...]\n";);
        ++vi;
      }
    else
      {
        // In case both left curve-ends have boundary conditions and are not
        // unbounded, check whether the left endpoints are the same. If they are,
        // skip the first intersection point.
        const Arr_parameter_space ps_x1 =
            this->m_traits->parameter_space_in_x_2_object()(c1->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_y1 =
            this->m_traits->parameter_space_in_y_2_object()(c1->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_x2 =
            this->m_traits->parameter_space_in_x_2_object()(c2->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_y2 =
            this->m_traits->parameter_space_in_y_2_object()(c2->last_curve(),
                ARR_MIN_END);

        if ((ps_x1 == ps_x2) && (ps_y1 == ps_y2) && ((ps_x1 != ARR_INTERIOR)
            || (ps_y2 != ARR_INTERIOR))
            && /*this->m_traits->is_bounded_2_object()(c1->last_curve(),
                ARR_MIN_END)*/true && /*this->m_traits->is_bounded_2_object()(
            c2->last_curve(), ARR_MIN_END)*/true)
          {
            if (this->m_traits->equal_2_object()(
                this->m_traits->construct_min_vertex_2_object()(
                    c1->last_curve()),
                this->m_traits->construct_min_vertex_2_object()(
                    c2->last_curve())))
              {
                ////MY_CGAL_PRINT(" [Skipping common left endpoint on boundary ...]\n";);
                ++vi;
              }
          }
      }

    // If the two subcurves have a common right-event, and the last intersection
    // object is a point, we can ignore last intersection (note that in case of
    // an overlap that ends at the common endpoint, we definately want to keep
    // the intersection object).
    if (reinterpret_cast<Event*> (c1->right_event())
        == reinterpret_cast<Event*> (c2->right_event()))
      {
        vector_inserter vi_last = vi_end;

        --vi_last;
        if (object_cast<std::pair<Point_2, unsigned int> > (&(*vi_last))
            != NULL)
          {
            ////MY_CGAL_PRINT(" [Skipping common right endpoint...]\n";);
            --vi_end;
          }
      }
    else
      {
        // In case both right curve-ends have boundary conditions and are not
        // unbounded, check whether the right endpoints are the same. If they are,
        // skip the last intersection point.
        const Arr_parameter_space ps_x1 =
            this->m_traits->parameter_space_in_x_2_object()(c1->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_y1 =
            this->m_traits->parameter_space_in_y_2_object()(c1->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_x2 =
            this->m_traits->parameter_space_in_x_2_object()(c2->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_y2 =
            this->m_traits->parameter_space_in_y_2_object()(c2->last_curve(),
                ARR_MAX_END);

        if ((ps_x1 == ps_x2) && (ps_y1 == ps_y2) && ((ps_x1 != ARR_INTERIOR)
            || (ps_y2 != ARR_INTERIOR))
            && /*this->m_traits->is_bounded_2_object()(c1->last_curve(),
                ARR_MAX_END)*/true && /*this->m_traits->is_bounded_2_object()(
            c2->last_curve(), ARR_MAX_END)*/true)
          {
            if (this->m_traits->equal_2_object()(
                this->m_traits->construct_max_vertex_2_object()(
                    c1->last_curve()),
                this->m_traits->construct_max_vertex_2_object()(
                    c2->last_curve())))
              {
                vector_inserter vi_last = vi_end;

                --vi_last;
                if (object_cast<std::pair<Point_2, unsigned int> > (&(*vi_last))
                    != NULL)
                  {
                    ////MY_CGAL_PRINT(" [Skipping common right endpoint on boundary...]\n";);
                    --vi_end;
                  }
              }
          }
      }

    const std::pair<Point_2, unsigned int> *xp_point;

    if (vi != vi_end)
      {
        xp_point = object_cast<std::pair<Point_2, unsigned int> > (&(*vi));
        if (xp_point != NULL)
          {
            // Skip the intersection point if it is not larger than the current
            // event.
            if (this->m_queueEventLess(xp_point->first, this->m_currentEvent)
                != LARGER)
              {
                ++vi;
              }
          }
      }

    for (; vi != vi_end; ++vi)
      {
        const X_monotone_curve_2 *icv;
        Point_2 xp;
        unsigned int multiplicity = 0;

        xp_point = object_cast<std::pair<Point_2, unsigned int> > (&(*vi));

        if (xp_point != NULL)
          {
            xp = xp_point->first;
            multiplicity = xp_point->second;
            ////MY_CGAL_PRINT("found an intersection point: " << xp << " \n";);
            _create_intersection_point_from_red(xp, multiplicity, c1, c2);
          }
        else
          {
            std::cout << "commented in _intersect_from_red by Jannis Warnat"
                << std::endl;
            //      icv = object_cast<X_monotone_curve_2> (&(*vi));
            //      CGAL_assertion (icv != NULL);
            //
            //      Point_2 left_xp = this->m_traits->construct_min_vertex_2_object()(*icv);
            //      xp = this->m_traits->construct_max_vertex_2_object()(*icv);
            //
            //      sub_cv1 = *icv;
            //      _create_intersection_point(xp, 0 , c1 , c2);
            //      _create_intersection_point(left_xp, 0 , c1 ,c2, true);
          }
      }
  }

//-----------------------------------------------------------------------------
// Compute intersections between the two given curves.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_intersect_from_blue(
      Subcurve *c1, Subcurve *c2)
  {
    MY_CGAL_SL_PRINT("begin _intersect_from_blue()");

    if (c2 == 0)
      {
        return;
      }

    // look up for (c1,c2) in the table and insert if doesnt exist
    Curve_pair
    cv_pair(c1, c2);
    if (!(m_curves_pair_set.insert(cv_pair)).second)
      {
        //MY_CGAL_SL_PRINT("already computed intersection");
        return; //the curves have already been checked for intersection
      }
    else
      {
        //MY_CGAL_SL_PRINT("compute new intersection");
      }


    float load_factor = static_cast<float> (m_curves_pair_set.size())
        / m_curves_pair_set.bucket_count();
    // after lot of benchemarks, keeping load_factor<=6 is optimal
    if (load_factor > 6.0f)
      m_curves_pair_set.resize(m_curves_pair_set.size() * 6);

    vector_inserter
    vi(m_x_objects);
    vector_inserter
    vi_end(m_x_objects);
    vi_end = this->m_traits->intersect_2_object()(c1->last_curve(),

    c2->last_curve(), vi);

    if (vi == vi_end)
      {
        ////MY_CGAL_PRINT("no intersection...\n";);
        return; // no intersection at all
      }

    // The two subCurves may start at the same point, in that case we ignore the
    // first intersection point (if we got to that stage, they cannot  overlap).
    if (reinterpret_cast<Event*> (c1->left_event()) == this->m_currentEvent
        && reinterpret_cast<Event*> (c2->left_event()) == this->m_currentEvent)
      {
        ////MY_CGAL_PRINT(" [Skipping common left endpoint...]\n";);
        ++vi;
      }
    else
      {
        // In case both left curve-ends have boundary conditions and are not
        // unbounded, check whether the left endpoints are the same. If they are,
        // skip the first intersection point.
        const Arr_parameter_space ps_x1 =
            this->m_traits->parameter_space_in_x_2_object()(c1->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_y1 =
            this->m_traits->parameter_space_in_y_2_object()(c1->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_x2 =
            this->m_traits->parameter_space_in_x_2_object()(c2->last_curve(),
                ARR_MIN_END);
        const Arr_parameter_space ps_y2 =
            this->m_traits->parameter_space_in_y_2_object()(c2->last_curve(),
                ARR_MIN_END);

        if ((ps_x1 == ps_x2) && (ps_y1 == ps_y2) && ((ps_x1 != ARR_INTERIOR)
            || (ps_y2 != ARR_INTERIOR))
            && /*this->m_traits->is_bounded_2_object()(c1->last_curve(),
                ARR_MIN_END)*/true && /*this->m_traits->is_bounded_2_object()(
            c2->last_curve(), ARR_MIN_END)*/true)
          {
            if (this->m_traits->equal_2_object()(
                this->m_traits->construct_min_vertex_2_object()(
                    c1->last_curve()),
                this->m_traits->construct_min_vertex_2_object()(
                    c2->last_curve())))
              {
                ////MY_CGAL_PRINT(" [Skipping common left endpoint on boundary ...]\n";);
                ++vi;
              }
          }
      }

    // If the two subcurves have a common right-event, and the last intersection
    // object is a point, we can ignore last intersection (note that in case of
    // an overlap that ends at the common endpoint, we definately want to keep
    // the intersection object).
    if (reinterpret_cast<Event*> (c1->right_event())
        == reinterpret_cast<Event*> (c2->right_event()))
      {
        vector_inserter vi_last = vi_end;

        --vi_last;
        if (object_cast<std::pair<Point_2, unsigned int> > (&(*vi_last))
            != NULL)
          {
            ////MY_CGAL_PRINT(" [Skipping common right endpoint...]\n";);
            --vi_end;
          }
      }
    else
      {
        // In case both right curve-ends have boundary conditions and are not
        // unbounded, check whether the right endpoints are the same. If they are,
        // skip the last intersection point.
        const Arr_parameter_space ps_x1 =
            this->m_traits->parameter_space_in_x_2_object()(c1->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_y1 =
            this->m_traits->parameter_space_in_y_2_object()(c1->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_x2 =
            this->m_traits->parameter_space_in_x_2_object()(c2->last_curve(),
                ARR_MAX_END);
        const Arr_parameter_space ps_y2 =
            this->m_traits->parameter_space_in_y_2_object()(c2->last_curve(),
                ARR_MAX_END);

        if ((ps_x1 == ps_x2) && (ps_y1 == ps_y2) && ((ps_x1 != ARR_INTERIOR)
            || (ps_y2 != ARR_INTERIOR))
            && /*this->m_traits->is_bounded_2_object()(c1->last_curve(),
                ARR_MAX_END)*/true && /*this->m_traits->is_bounded_2_object()(
            c2->last_curve(), ARR_MAX_END)*/true)
          {
            if (this->m_traits->equal_2_object()(
                this->m_traits->construct_max_vertex_2_object()(
                    c1->last_curve()),
                this->m_traits->construct_max_vertex_2_object()(
                    c2->last_curve())))
              {
                vector_inserter vi_last = vi_end;

                --vi_last;
                if (object_cast<std::pair<Point_2, unsigned int> > (&(*vi_last))
                    != NULL)
                  {
                    ////MY_CGAL_PRINT(" [Skipping common right endpoint on boundary...]\n";);
                    --vi_end;
                  }
              }
          }
      }

    const std::pair<Point_2, unsigned int> *xp_point;

    if (vi != vi_end)
      {
        xp_point = object_cast<std::pair<Point_2, unsigned int> > (&(*vi));
        if (xp_point != NULL)
          {
            // Skip the intersection point if it is not larger than the current
            // event.
            if (this->m_queueEventLess(xp_point->first, this->m_currentEvent)
                != LARGER)
              {
                ++vi;
              }
          }
      }

    for (; vi != vi_end; ++vi)
      {
        const X_monotone_curve_2 *icv;
        Point_2 xp;
        unsigned int multiplicity = 0;

        xp_point = object_cast<std::pair<Point_2, unsigned int> > (&(*vi));

        if (xp_point != NULL)
          {
            xp = xp_point->first;
            multiplicity = xp_point->second;
            _create_intersection_point_from_blue(xp, multiplicity, c1, c2);
          }
        else
          {
            std::cout << "commented in _intersect_from_red by Jannis Warnat"
                << std::endl;
            //      icv = object_cast<X_monotone_curve_2> (&(*vi));
            //      CGAL_assertion (icv != NULL);
            //
            //      Point_2 left_xp = this->m_traits->construct_min_vertex_2_object()(*icv);
            //      xp = this->m_traits->construct_max_vertex_2_object()(*icv);
            //
            //      sub_cv1 = *icv;
            //      _create_intersection_point(xp, 0 , c1 , c2);
            //      _create_intersection_point(left_xp, 0 , c1 ,c2, true);
          }
      }
  }

//-----------------------------------------------------------------------------
// Create an intersection-point event between two curves.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_create_intersection_point_from_red(
      Point_2 xp, unsigned int multiplicity, Subcurve* c1, Subcurve* c2,
      bool is_overlap)
  {

    // insert the event and check if an event at this point already exists.
    const std::pair<Event*, bool>& pair_res = this->_push_red_event(xp,
        Base_event::DEFAULT, ARR_INTERIOR, ARR_INTERIOR);

    Event *e = pair_res.first;
    if (pair_res.second)//new event
      {
        //following adapted by Jannis Warnat
        //insert the red blue intersection in the arrangement and store the vertex with the point
        Vertex_const_handle vh = this->get_purple()->insert_in_face_interior(
            e->point().base(), this->get_purple()->unbounded_face());
        e->point().set_purple_object(make_object(vh));
        Vertex_handle non_const_vh = this->get_purple()->non_const_handle(vh);
        non_const_vh->set_data(DEFAULT);

        // a new event is creatd , which inidicates
        // that the intersection point cannot be one
        //of the end-points of two curves

        e->set_intersection();

        //this->m_visitor ->update_event(e, c1, c2, true);
        e->push_back_red_curve_to_left(c1);
        e->push_back_blue_curve_to_left(c2);

        //         Act according to the multiplicity:
        if (multiplicity == 0)
          {
            // The multiplicity of the intersection point is unkown or undefined:
            _add_red_curve_to_right(e, c1, is_overlap);
            _add_blue_curve_to_right(e, c2, is_overlap);
            if (!is_overlap)
              {
                //                if (e->is_right_curve_bigger_from_red(c1, c2))
                //                  std::swap(c1, c2);
              }
          }
        else
          {
            if ((multiplicity % 2) == 1)
              {
                //         The mutiplicity of the intersection point is odd: Swap their
                //         order to the right of this point.
                //std::swap(c1, c2);
                e->add_curve_pair_to_right_from_red(c1, c2);
              }
            else
              {
                // The mutiplicity of the intersection point is even, so they
                // maintain their order to the right of this point.
                CGAL_assertion((multiplicity % 2) == 0);
                e->add_curve_pair_to_right_from_red(c1, c2);
              }
          }
      }
    else // the event already exists, so we need to update it accordingly
      {
        ////MY_CGAL_PRINT("event already exists,updating..1 (" << xp <<")\n";);
        if (e == this->m_currentEvent)
          {
            // This can happen when c1 starts at the interior of c2 (or vice versa).
            return;
          }

        e->add_red_curve_to_left(c1);
        e->add_blue_curve_to_left(c2);

        if (!c1->is_end_point(e) && !c2->is_end_point(e))
          {
            //strong intersection
            _add_red_curve_to_right(e, c1, is_overlap);
            _add_blue_curve_to_right(e, c2, is_overlap);
            e->set_intersection();
            //this->m_visitor ->update_event(e, c1, c2, false);
          }
        else
          {
            //weak intersection
            if (!c1->is_end_point(e) && c2->is_end_point(e))
              {
                _add_red_curve_to_right(e, c1, is_overlap);
                e->set_weak_intersection();
                //e->set_intersection();
                //this->m_visitor ->update_event(e, c1);
              }
            else
              {
                if (c1->is_end_point(e) && !c2->is_end_point(e))
                  {
                    _add_blue_curve_to_right(e, c2, is_overlap);
                    e->set_weak_intersection();
                    //e->set_intersection();
                    //this->m_visitor ->update_event(e, c2);
                  }
              }
          }
        if (!is_overlap)
          {
            //            if (e->is_right_curve_bigger_from_red(c1, c2))
            //              std::swap(c1, c2);
          }

        //MY_CGAL_SL_DEBUG      (e->Print();)
      }
  }

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_create_intersection_point_from_blue(
      Point_2 xp, unsigned int multiplicity, Subcurve* c1, Subcurve* c2,
      bool is_overlap)
  {
    // insert the event and check if an event at this point already exists.
    const std::pair<Event*, bool>& pair_res = this->_push_red_event(xp,
        Base_event::DEFAULT, ARR_INTERIOR, ARR_INTERIOR);

    Event *e = pair_res.first;
    if (pair_res.second)
      {
        //following adapted by Jannis Warnat
        //insert the red blue intersection in the arrangement and store the vertex with the point
        Vertex_const_handle vh = this->get_purple()->insert_in_face_interior(
            e->point().base(), this->get_purple()->unbounded_face());
        e->point().set_purple_object(make_object(vh));
        Vertex_handle non_const_vh = this->get_purple()->non_const_handle(vh);
        non_const_vh->set_data(DEFAULT);

        // a new event is creatd , which inidicates
        // that the intersection point cannot be one
        //of the end-points of two curves

        e->set_intersection();

        //this->m_visitor ->update_event(e, c1, c2, true);
        e->push_back_blue_curve_to_left(c1);
        e->push_back_red_curve_to_left(c2);

        // Act according to the multiplicity:
        if (multiplicity == 0)
          {
            // The multiplicity of the intersection point is unkown or undefined:
            _add_blue_curve_to_right(e, c1, is_overlap);
            _add_red_curve_to_right(e, c2, is_overlap);
            if (!is_overlap)
              {
                //                if (e->is_right_curve_bigger_from_blue(c1, c2))
                //                  std::swap(c1, c2);
              }
          }
        else
          {
            if ((multiplicity % 2) == 1)
              {
                // The mutiplicity of the intersection point is odd: Swap their
                // order to the right of this point.
                //std::swap(c1,c2);
                e->add_curve_pair_to_right_from_blue(c1, c2);

              }
            else
              {
                // The mutiplicity of the intersection point is even, so they
                // maintain their order to the right of this point.
                CGAL_assertion((multiplicity % 2) == 0);
                e->add_curve_pair_to_right_from_blue(c1, c2);
              }
          }
      }
    else // the event already exists, so we need to update it accordingly
      {
        ////MY_CGAL_PRINT("event already exists,updating..2 (" << xp <<")\n";);
        if (e == this->m_currentEvent)
          {
            // This can happen when c1 starts at the interior of c2 (or vice versa).
            return;
          }

        e->add_blue_curve_to_left(c1);
        e->add_red_curve_to_left(c2);

        if (!c1->is_end_point(e) && !c2->is_end_point(e))
          {
            //strong intersection
            _add_blue_curve_to_right(e, c1, is_overlap);
            _add_red_curve_to_right(e, c2, is_overlap);
            e->set_intersection();
            //this->m_visitor ->update_event(e, c1, c2, false);
          }
        else
          {
            //weak intersection
            if (!c1->is_end_point(e) && c2->is_end_point(e))
              {
                _add_blue_curve_to_right(e, c1, is_overlap);
                e->set_weak_intersection();
                //e->set_intersection();
                //this->m_visitor ->update_event(e, c1);
              }
            else
              {
                if (c1->is_end_point(e) && !c2->is_end_point(e))
                  {
                    _add_red_curve_to_right(e, c2, is_overlap);
                    e->set_weak_intersection();
                    //e->set_intersection();
                    //this->m_visitor ->update_event(e, c2);
                  }
              }
          }
        if (!is_overlap)
          {
            //            if (e->is_right_curve_bigger_from_blue(c1, c2))
            //              std::swap(c1, c2);
          }

        //MY_CGAL_SL_DEBUG      (e->Print();)
      }
  }

//-----------------------------------------------------------------------------
// Fix overlap Subcurves before handling the current event.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_fix_overlap_subcurves_from_red()
  {
    CGAL_assertion(this->m_currentEvent->has_red_left_curves());

    Event_subcurve_iterator leftCurveIter =
        this->m_currentEvent->red_left_curves_begin();

    //special treatment for Subcuves that store overlaps
    while (leftCurveIter != this->m_currentEvent->red_left_curves_end())
      {
        Subcurve *leftCurve = *leftCurveIter;

        // we check if the subcurve store overlap and current event is its
        // right end point.
        if ((Event*) leftCurve->right_event() == this->m_currentEvent)
          {
            if (leftCurve->originating_subcurve1() != NULL)
              {
                Subcurve* orig_sc_1 =
                    (Subcurve*) leftCurve->originating_subcurve1();
                Subcurve* orig_sc_2 =
                    (Subcurve*) leftCurve->originating_subcurve2();

                _fix_finished_overlap_subcurve(orig_sc_1);
                _fix_finished_overlap_subcurve(orig_sc_2);
              }
          }
        ++leftCurveIter;
      }
  }

//-----------------------------------------------------------------------------
// Fix overlap Subcurves before handling the current event.
//
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_fix_overlap_subcurves_from_blue()
  {
    CGAL_assertion(this->m_currentEvent->has_blue_left_curves());

    Event_subcurve_iterator leftCurveIter =
        this->m_currentEvent->blue_left_curves_begin();

    //special treatment for Subcuves that store overlaps
    while (leftCurveIter != this->m_currentEvent->blue_left_curves_end())
      {
        Subcurve *leftCurve = *leftCurveIter;

        // we check if the subcurve store overlap and current event is its
        // right end point.
        if ((Event*) leftCurve->right_event() == this->m_currentEvent)
          {
            if (leftCurve->originating_subcurve1() != NULL)
              {
                Subcurve* orig_sc_1 =
                    (Subcurve*) leftCurve->originating_subcurve1();
                Subcurve* orig_sc_2 =
                    (Subcurve*) leftCurve->originating_subcurve2();

                _fix_finished_overlap_subcurve(orig_sc_1);
                _fix_finished_overlap_subcurve(orig_sc_2);
              }
          }
        ++leftCurveIter;
      }
  }

//-----------------------------------------------------------------------------
// Handle overlap at right insertion to event.
// event - the event where that overlap starts (the left
// end point of the overlap).
// curve - the subcurve that its insertion to the list of right subcurves of
// 'event' causes the overlap (with *iter).
// iter - the existing subcurve at the right subcurves of 'event'
// overlap_exist - a flag indicates if the overlap X_monotone_curve_2 was
// computed already (is true than its stored at sub_cv1 data member).
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_overlap_from_red(
      Event* event, Subcurve* curve, Event_subcurve_iterator iter,
      bool overlap_exist)
  {
    // An overlap occurs:
    ////MY_CGAL_PRINT("Overlap detected at right insertion...\n";);

    X_monotone_curve_2 overlap_cv;
    if (overlap_exist)
      overlap_cv = sub_cv1;
    else
      {
        // compute the overlap.
        std::vector<Object> obj_vec;
        vector_inserter
        vit(obj_vec);
        this->m_traits->intersect_2_object()(curve->last_curve(),
            (*iter)->last_curve(), vit);

        if (obj_vec.empty())
          return;

        overlap_cv = object_cast<X_monotone_curve_2> (obj_vec.front());
      }

    // Get the right end of overlap_cv (if it is bounded from the right).
    Event *right_end;
    Arr_parameter_space
        ps_x_r = this->m_traits->parameter_space_in_x_2_object()(overlap_cv,
            ARR_MAX_END);
    Arr_parameter_space
        ps_y_r = this->m_traits->parameter_space_in_y_2_object()(overlap_cv,
            ARR_MAX_END);

    CGAL_assertion(ps_x_r != ARR_LEFT_BOUNDARY);
    if (ps_x_r != ARR_INTERIOR || ps_y_r != ARR_INTERIOR)
      {
        // The overlapping subcurve is either unbounded from the right, or
        // touches the boundary of the surface. In either case, the curves that
        // are involved in the overlap must also be unbounded or defined at the
        // boundary, so the event associated with their right ends already exists,
        // and we set it as the overlapping subcurve's right event.
        CGAL_assertion((*iter)->right_event() == curve->right_event());
        right_end = (Event*) (curve->right_event());
      }
    else
      {
        // The overlapping subcurve has a valid right endpoint.
        // Find the event associated with this point (or create a new event).
        Point_2 end_overlap = this->m_traits->construct_max_vertex_2_object()(
            overlap_cv);

        const std::pair<Event*, bool>& pair_res = this->_push_red_event(end_overlap,
            Base_event::OVERLAP, ps_x_r, ps_y_r);

        right_end = pair_res.first;
      }

    // Get the left end of overlap_cv (if it is bounded from the left).
    Arr_parameter_space
        ps_x_l = this->m_traits->parameter_space_in_x_2_object()(overlap_cv,
            ARR_MIN_END);
    Arr_parameter_space
        ps_y_l = this->m_traits->parameter_space_in_y_2_object()(overlap_cv,
            ARR_MIN_END);

    CGAL_assertion(ps_x_l != ARR_RIGHT_BOUNDARY);
    if (ps_x_l == ARR_INTERIOR && ps_y_l == ARR_INTERIOR)
      {
        // The left end of the overlapping subcurve is regular point, so in case
        // the event is also associated with a regular point (not incident to the
        // surface boundaries), we make sure that the overlapping subcurve does
        // not start to the left of this event.
        if (!event->is_on_boundary())
          {
            // If the left endpoint of the overlapping curve is to the left of the
            // event, split the overlapping subcurve so its left endpoint equals
            // the event point.
            const Point_2& begin_overlap =
                this->m_traits->construct_min_vertex_2_object()(overlap_cv);
            Comparison_result res = this->m_traits->compare_xy_2_object()(
                event->point(), begin_overlap);

            CGAL_assertion(res != SMALLER);
            if (res == LARGER)
              {
                this->m_traits->split_2_object()(overlap_cv, event->point(),
                    sub_cv1, sub_cv2);
                overlap_cv = sub_cv2;
              }
          }
      }
    else
      {
        // The left end of the overlapping subcurve is either unbounded, or
        // incident to the surface boundaries. In case the current event is
        // associated with a regular point, it must lie to the right of this
        // curve-end, so we clip the overlapping subcurve accordingly.
        if (!event->is_on_boundary())
          {
            this->m_traits->split_2_object()(overlap_cv, event->point(),
                sub_cv1, sub_cv2);
            overlap_cv = sub_cv2;
          }
      }

    // Alocate a new Subcure for the overlap
    Subcurve *overlap_sc = this->m_subCurveAlloc.allocate(1);
    this->m_subCurveAlloc.construct(overlap_sc, this->m_masterSubcurve);
    CGAL_assertion(false);//Jannis Warnat
    overlap_sc->init_subcurve(overlap_cv);
    overlap_sc->set_left_event(event);
    overlap_sc->set_right_event(right_end);
    //m_overlap_subCurves.push_back(overlap_sc);

    ////MY_CGAL_PRINT(curve << " + " << *iter << " => " << overlap_sc << "\n");
    // Set the two events' attribute to overlap.
    event -> set_overlap();
    //right_end -> set_overlap();

    // Remove curve, *iter from the left curves of end_overlap event
    right_end->remove_red_curve_from_left(curve);
    right_end->remove_red_curve_from_left(*iter);

    // Add overlap_sc to the left curves
    right_end->add_red_curve_to_left(overlap_sc);

    // sets the two originating subcurves of overlap_sc
    overlap_sc -> set_originating_subcurve1(*iter);
    overlap_sc -> set_originating_subcurve2(curve);

    // If one of the originating subcurves (or both), does not end
    // at the right end of the overlap, add them to the right subcurves
    // of the event associated with the right end of the overlap.
    if ((Event*) curve->right_event() != right_end)
      {
        _add_red_curve_to_right(right_end, curve);
      }

    if ((Event*) (*iter)->right_event() != right_end)
      {
        _add_red_curve_to_right(right_end, (*iter));
      }

    //this->m_visitor->found_overlap(curve, *iter, overlap_sc);

    // Replace current sub-curve (*iter) with the new sub-curve
    (*iter) = overlap_sc;
  }

//-----------------------------------------------------------------------------
// Handle overlap at right insertion to event.
// event - the event where that overlap starts (the left
// end point of the overlap).
// curve - the subcurve that its insertion to the list of right subcurves of
// 'event' causes the overlap (with *iter).
// iter - the existing subcurve at the right subcurves of 'event'
// overlap_exist - a flag indicates if the overlap X_monotone_curve_2 was
// computed already (is true than its stored at sub_cv1 data member).
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_handle_overlap_from_blue(
      Event* event, Subcurve* curve, Event_subcurve_iterator iter,
      bool overlap_exist)
  {
    // An overlap occurs:
    ////MY_CGAL_PRINT("Overlap detected at right insertion...\n";);

    X_monotone_curve_2 overlap_cv;
    if (overlap_exist)
      overlap_cv = sub_cv1;
    else
      {
        // compute the overlap.
        std::vector<Object> obj_vec;
        vector_inserter
        vit(obj_vec);
        this->m_traits->intersect_2_object()(curve->last_curve(),
            (*iter)->last_curve(), vit);

        if (obj_vec.empty())
          return;

        overlap_cv = object_cast<X_monotone_curve_2> (obj_vec.front());
      }

    // Get the right end of overlap_cv (if it is bounded from the right).
    Event *right_end;
    Arr_parameter_space
        ps_x_r = this->m_traits->parameter_space_in_x_2_object()(overlap_cv,
            ARR_MAX_END);
    Arr_parameter_space
        ps_y_r = this->m_traits->parameter_space_in_y_2_object()(overlap_cv,
            ARR_MAX_END);

    CGAL_assertion(ps_x_r != ARR_LEFT_BOUNDARY);
    if (ps_x_r != ARR_INTERIOR || ps_y_r != ARR_INTERIOR)
      {
        // The overlapping subcurve is either unbounded from the right, or
        // touches the boundary of the surface. In either case, the curves that
        // are involved in the overlap must also be unbounded or defined at the
        // boundary, so the event associated with their right ends already exists,
        // and we set it as the overlapping subcurve's right event.
        CGAL_assertion((*iter)->right_event() == curve->right_event());
        right_end = (Event*) (curve->right_event());
      }
    else
      {
        // The overlapping subcurve has a valid right endpoint.
        // Find the event associated with this point (or create a new event).
        Point_2 end_overlap = this->m_traits->construct_max_vertex_2_object()(
            overlap_cv);

        const std::pair<Event*, bool>& pair_res = this->_push_red_event(end_overlap,
            Base_event::OVERLAP, ps_x_r, ps_y_r);

        right_end = pair_res.first;
      }

    // Get the left end of overlap_cv (if it is bounded from the left).
    Arr_parameter_space
        ps_x_l = this->m_traits->parameter_space_in_x_2_object()(overlap_cv,
            ARR_MIN_END);
    Arr_parameter_space
        ps_y_l = this->m_traits->parameter_space_in_y_2_object()(overlap_cv,
            ARR_MIN_END);

    CGAL_assertion(ps_x_l != ARR_RIGHT_BOUNDARY);
    if (ps_x_l == ARR_INTERIOR && ps_y_l == ARR_INTERIOR)
      {
        // The left end of the overlapping subcurve is regular point, so in case
        // the event is also associated with a regular point (not incident to the
        // surface boundaries), we make sure that the overlapping subcurve does
        // not start to the left of this event.
        if (!event->is_on_boundary())
          {
            // If the left endpoint of the overlapping curve is to the left of the
            // event, split the overlapping subcurve so its left endpoint equals
            // the event point.
            const Point_2& begin_overlap =
                this->m_traits->construct_min_vertex_2_object()(overlap_cv);
            Comparison_result res = this->m_traits->compare_xy_2_object()(
                event->point(), begin_overlap);

            CGAL_assertion(res != SMALLER);
            if (res == LARGER)
              {
                this->m_traits->split_2_object()(overlap_cv, event->point(),
                    sub_cv1, sub_cv2);
                overlap_cv = sub_cv2;
              }
          }
      }
    else
      {
        // The left end of the overlapping subcurve is either unbounded, or
        // incident to the surface boundaries. In case the current event is
        // associated with a regular point, it must lie to the right of this
        // curve-end, so we clip the overlapping subcurve accordingly.
        if (!event->is_on_boundary())
          {
            this->m_traits->split_2_object()(overlap_cv, event->point(),
                sub_cv1, sub_cv2);
            overlap_cv = sub_cv2;
          }
      }

    // Alocate a new Subcure for the overlap
    Subcurve *overlap_sc = this->m_subCurveAlloc.allocate(1);
    this->m_subCurveAlloc.construct(overlap_sc, this->m_masterSubcurve);
    CGAL_assertion(false);//Jannis Warnat
    overlap_sc->init_subcurve(overlap_cv);
    overlap_sc->set_left_event(event);
    overlap_sc->set_right_event(right_end);
    //m_overlap_subCurves.push_back(overlap_sc);

    ////MY_CGAL_PRINT(curve << " + " << *iter << " => " << overlap_sc << "\n");
    // Set the two events' attribute to overlap.
    event -> set_overlap();
    //right_end -> set_overlap();

    // Remove curve, *iter from the left curves of end_overlap event
    right_end->remove_blue_curve_from_left(curve);
    right_end->remove_blue_curve_from_left(*iter);

    // Add overlap_sc to the left curves
    right_end->add_blue_curve_to_left(overlap_sc);

    // sets the two originating subcurves of overlap_sc
    overlap_sc -> set_originating_subcurve1(*iter);
    overlap_sc -> set_originating_subcurve2(curve);

    // If one of the originating subcurves (or both), does not end
    // at the right end of the overlap, add them to the right subcurves
    // of the event associated with the right end of the overlap.
    if ((Event*) curve->right_event() != right_end)
      {
        _add_blue_curve_to_right(right_end, curve);
      }

    if ((Event*) (*iter)->right_event() != right_end)
      {
        _add_blue_curve_to_right(right_end, (*iter));
      }

    //this->m_visitor->found_overlap(curve, *iter, overlap_sc);

    // Replace current sub-curve (*iter) with the new sub-curve
    (*iter) = overlap_sc;
  }

//-----------------------------------------------------------------------------
// Fix a subcurve that represents an overlap.
// sc - some originating subcurve of a aubcurve that stores an overlap
// notice thah this function is recursive since an originating subcurve of
// an overlap can be itself a subcurve that stores overlap and so on.
template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_fix_finished_overlap_subcurve(
      Subcurve* sc)
  {
    //
    CGAL_assertion(sc != NULL);

    // split 'sc' if necessary and update to event as weak intersection
    if ((Event*) sc->right_event() != this->m_currentEvent)
      {
        this->m_traits->split_2_object()(sc->last_curve(),
            this->m_currentEvent->point(), sub_cv1, sub_cv2);
        sc->set_last_curve(sub_cv2);

        this->m_currentEvent->set_weak_intersection();
        //this->m_visitor ->update_event(this->m_currentEvent,(Subcurve*)sc);
        return;
      }

    if (!sc->originating_subcurve1())
      // sc does not store an overlap, we are done
      return;

    // sc is a subcurve that stores overlap, we have to continue with the
    // recursion and deal with his two originating subcurves recursively.
    Subcurve* orig_sc_1 = (Subcurve*) sc->originating_subcurve1();
    Subcurve* orig_sc_2 = (Subcurve*) sc->originating_subcurve2();

    _fix_finished_overlap_subcurve(orig_sc_1);
    _fix_finished_overlap_subcurve(orig_sc_2);
  }

CGAL_END_NAMESPACE

#endif

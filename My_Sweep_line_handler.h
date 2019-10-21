#ifndef MY_SWEEP_LINE_HANDLER_H
#define MY_SWEEP_LINE_HANDLER_H

/*! \file
 * Member-function definitions for the My_Sweep_line_2 class specifically needed
 * for the red blue merge algorithm
 * by Jannis Warnat
 */

CGAL_BEGIN_NAMESPACE

// ---------------------------------------------------------------------------
// Checks the type of the current event/point/vertex and calls the appropriate
// function

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::proceed_according_to_vertex_type()
  {
    MY_CGAL_SL_PRINT("begin proceed_according_to_vertex_type()");

    //red-blue intersection, no red or blue arrangement vertex
    //associated with the event
    if (this->m_currentEvent->is_intersection())
      {
        MY_CGAL_SL_PRINT("red blue intersection " << this->current_event_point);
        handle_red_blue_intersection();
        return;
      }

    //red or blue
    this->current_event_point_color = this->current_event_point.color();

    //extract the arrangement vertex from the current event point
    if (this->current_event_point_color == Traits_2::RED)
      {
        try
          {
            this->current_vertex = object_cast<Vertex_const_handle> (
                this->current_event_point.red_object());
          }
        catch (Bad_object_cast)
          {
            CGAL_assertion(false);
          }
      }
    else
      {
        try
          {
            this->current_vertex = object_cast<Vertex_const_handle> (
                this->current_event_point.blue_object());
          }
        catch (Bad_object_cast)
          {
            CGAL_assertion(false);
          }
      }

    Vertex_type p_type = this->current_vertex->data().type();

    //point is nothing of the types below
    if (p_type == DEFAULT)
      {
        handle_default();
      }

    //if delegate_to_next_default was set it is not needed
    //if the event is not of type default
    this->reset_delegation();

    if (p_type == INTERNAL_ENDPOINT)
      {
        //endpoint is either left or right endpoint
        if (this->m_currentEvent->has_left_curves())
          {
            handle_right_endpoint();
          }
        else
          {
            handle_left_endpoint();
          }
      }

    //happens only once per sweep
    if (p_type == POINT_X)
      {
        handle_point_x();
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_default(), the scout jumps from the left
// curve to the correct right curve

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::change_boundary_of_upper_scout(
      Subcv* left_curve)
  {
    MY_CGAL_SL_PRINT("begin change_boundary_of_upper_scout()");

    //get lower curve at current event
    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Subcurve* right_curve = *right_begin;

    if (left_curve->has_upper_scout())
      {
        //left_curve is part of the boundary, so insert into
        //purple arrangement
        left_curve->upper_scout()->insert_into_purple_arr();

        //move the scout
        right_curve->set_upper_scout(left_curve->upper_scout());
        right_curve->upper_scout()->set_boundary_curve(right_curve);

        // scout sits on a new boundary curve so we have to check
        // for intersection with the guarded curve
        if (left_curve->upper_scout()->is_active())
          {
            right_curve->upper_scout()->check_individual_for_intersection();
          }

        left_curve->remove_upper_scout();

        // scout sits on a new boundary curve so we have to check
        // for intersection with the partner
        right_curve->upper_scout()->check_partners_for_intersection();
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_default(), the scout jumps from the
// left curve to the correct right curve
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::change_boundary_of_lower_scout(
      Subcv* left_curve)
  {
    MY_CGAL_SL_PRINT("begin change_boundary_of_lower_scout()");

    //get upper curve at current event
    Event_subcurve_iterator right_end = this->right_curves_end();
    right_end--;
    Subcurve* right_curve = *right_end;

    if (left_curve->has_lower_scout())
      {
        //left_curve is part of the boundary, so insert into
        //purple arrangement
        left_curve->lower_scout()->insert_into_purple_arr();

        //move the scout
        right_curve->set_lower_scout(left_curve->lower_scout());
        right_curve->lower_scout()->set_boundary_curve(right_curve);

        // scout sits on a new boundary curve so we have to check
        // for intersection with the guarded curve
        if (left_curve->lower_scout()->is_active())
          {
            right_curve->lower_scout()->check_individual_for_intersection();
          }

        left_curve->remove_lower_scout();

        // scout sits on a new boundary curve so we have to check
        // for intersection with the partner
        right_curve->lower_scout()->check_partners_for_intersection();
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_default() and handle_right_endpoint(),
// possible guards need to be informed about changes in their guarded curve
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::change_assignment_of_upper_guard(
      Subcv* left_curve)
  {
    MY_CGAL_SL_PRINT("begin change_assignment_of_upper_guard()");

    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Event_subcurve_iterator right_end = this->right_curves_end();
    Status_line* status_line = this->same_colored_status_line();

    if (left_curve->is_guarded_by_upper_scout())
      {
        //there is a right curve so just jump over to it;
        //we are at a default event
        if (right_begin != right_end)
          {
            Subcurve* right_curve = *right_begin;
            left_curve->upper_guard()->set_guarded_curve(right_curve);

            //the guarded curve changed, so check for intersection
            (right_curve)->upper_guard()->check_individual_for_intersection();
          }
        else //we are at a right endpoint
          {

            //left curve ends here (right endpoint), guard closest curve above
            std::pair<Status_line_iterator, bool> pair =
                status_line->find_lower(this->current_event_point,
                    this->m_statusLineCurveLess);
            Status_line_iterator above = pair.first;

            //there is a curve above to guard
            if (above != status_line->end())
              {
                left_curve->upper_guard()->set_idled();
                left_curve->upper_guard()->set_guarded_curve(*above);

                //the following includes an intersection test if necessary
                left_curve->upper_guard()->start_upper_inform_upper_above();
              }
            else
              {
                //there is no curve above
                left_curve->upper_guard()->set_guarded_curve(0);
              }
          }
        left_curve->remove_upper_guard();
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_default() and handle_right_endpoint(),
// possible guards need to be informed about changes in their guarded curve
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::change_assignment_of_lower_guard(
      Subcv* left_curve)
  {
    MY_CGAL_SL_PRINT("begin change_assignment_of_lower_guard()");

    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Event_subcurve_iterator right_end = this->right_curves_end();

    if (left_curve->is_guarded_by_lower_scout())
      {
        //there is a right curve so just jump over to it;
        //we are at a default event
        if (right_begin != right_end)
          {
            right_end--;
            Subcurve* right_curve = *right_end;
            left_curve->lower_guard()->set_guarded_curve(right_curve);

            //the guarded curve changed, so check for intersection
            (right_curve)->lower_guard()->check_individual_for_intersection();
          }
        else //we are at a right endpoint
          {
            //left_curve curve ends here (right endpoint), guard closest curve below;
            //no geometric search on the status line necessary
            Subcurve* below_curve;

            if (this->current_event_point_color == Traits_2::RED)
              {
                below_curve = this->current_red_below();
              }
            else
              {
                below_curve = this->current_blue_below();
              }
            //there is no curve below
            if (below_curve == 0)
              {
                left_curve->lower_guard()->set_guarded_curve(0);
              }
            else
              {
                //there is a curve below to guard
                left_curve->lower_guard()->set_idled();
                left_curve->lower_guard()->set_guarded_curve(below_curve);

                //the following includes an intersection test if necessary
                left_curve->lower_guard()->start_lower_inform_lower_below();
              }
          }
        left_curve->remove_lower_guard();
      }
  }

// ---------------------------------------------------------------------------
// Handles events/vertices/points of type DEFAULT

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::handle_default()
  {
    MY_CGAL_SL_PRINT("begin handle_default");

    //one or two left curves and one or two right curves of the same color
    Event_subcurve_iterator left_begin = this->left_curves_begin();
    Event_subcurve_iterator left_end = this->left_curves_end();
    Event_subcurve_iterator right_begin = this->right_curves_begin();
    int number_of_right_curves = this->number_of_right_curves();
    int number_of_left_curves = this->number_of_left_curves();

    //we have up to two left curves, call them "left_lower" and "left_upper"
    if (number_of_left_curves != 0)
      {
        Subcurve* left_lower = *left_begin;
        Event_subcurve_iterator left_second = left_begin;
        left_second++;
        Subcurve* left_upper;

        if (left_second != left_end)
          {
            //there is a second left curve
            left_upper = *left_second;

            //end of a region
            if (left_upper->has_upper_scout() && left_lower->has_lower_scout())
              {
                end_region(left_lower->lower_scout(), left_upper->upper_scout());
              }
          }

        //only the lowest can be guarded by an upper scout
        change_assignment_of_upper_guard(left_lower);

        //move a possible upper scout
        change_boundary_of_upper_scout(left_lower);

        //left upper, can be the same as left lower
        left_end--;
        left_upper = *left_end;

        //only the highest can be guarded by a lower scout
        change_assignment_of_lower_guard(left_upper);

        //move a possible lower scout
        change_boundary_of_lower_scout(left_upper);
      }

    //we may have to start a new region
    if (this->delegated_to_next_default())
      {
        start_region();
      }

    //the curves currently below may change, store them with the sweep line
    if (number_of_right_curves != 0)
      {
        //the highest right curve becomes the current curve below
        if (number_of_right_curves == 2)
          {
            right_begin++;
          }

        Subcurve* right_upper = *right_begin;

        //ignore vertical segments
        bool is_vertical = right_upper->last_curve().base().is_vertical();
        Subcurve* right_upper_non_vertical;
        if (is_vertical && number_of_right_curves == 2)
          {
            right_begin--;
            right_upper_non_vertical = *right_begin;
          }
        else
          {
            right_upper_non_vertical = right_upper;
          }

        //update current curves below if appropriate
        if (!is_vertical || (is_vertical && number_of_right_curves == 2))
          {
            if (this->current_event_point_color == Traits_2::RED)
              {
                this->set_current_red_below(right_upper_non_vertical);
              }
            else
              {
                this->set_current_blue_below(right_upper_non_vertical);
              }
          }
      }
  }

// ---------------------------------------------------------------------------
// Function to end a region between two scouts, "standard" end of a region;

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::end_region(My_Scout* lower,
      My_Scout* upper)
  {
    MY_CGAL_SL_PRINT("begin end_region()");

    //inform affected scouts
    upper->end_upper_inform_upper_below();
    lower->end_lower_inform_lower_above();

    // make sure the boundaries of the region will be inserted into
    // the purple arrangement

    //the lower scout lies on a vertical segment that is not split yet
    if (this->m_currentEvent->is_weak_intersection())
      {
        lower->boundary_curve()->set_insert_sub_cv1(true);
        upper->insert_into_purple_arr();
      }
    else
      {
        //the region ends in a red blue intersection, the curves
        //are not split yet
        if (this->m_currentEvent->is_intersection())
          {
            upper->boundary_curve()->set_insert_sub_cv1(true);
            lower->boundary_curve()->set_insert_sub_cv1(true);
          }
        else
          {
            //"standard case", insert the boundaries of the region into
            //the purple arrangement
            upper->insert_into_purple_arr();
            lower->insert_into_purple_arr();
          }
      }

    lower->delete_lower_scout();
    delete lower;
    upper->delete_upper_scout();
    delete upper;
  }

// ---------------------------------------------------------------------------
// Function to end a region between two scouts, special case of an end of a region

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::end_lower_region(My_Scout* lower,
      My_Scout* upper)
  {
    MY_CGAL_SL_PRINT("begin end_lower_region()");

    //inform the affected scouts
    upper->end_upper_inform_upper_below();
    lower->end_lower_inform_lower_above();

    //lower scout has already moved on, so do not insert its boundary;
    //if type DEFAULT, the upper scout has already move on too
    if (this->current_vertex->data().type() != DEFAULT)
      {
        upper->insert_into_purple_arr();
      }

    lower->delete_lower_scout();
    delete lower;
    upper->delete_upper_scout();
    delete upper;
  }

// ---------------------------------------------------------------------------
// Handles events/vertices/points of type INTERNAL_ENDPOINT,
// specifically right endpoints

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::handle_right_endpoint()
  {
    MY_CGAL_SL_PRINT("begin handle_right_endpoint");

    //one left curve, no right curves
    Event_subcurve_iterator left_begin = this->left_curves_begin();
    Subcurve* left_curve = *left_begin;

    //inform possible guards
    change_assignment_of_upper_guard(left_curve);
    change_assignment_of_lower_guard(left_curve);

    //end the region above the left curve
    if (left_curve->has_lower_scout())
      {
        end_region(left_curve->lower_scout(),
            left_curve->lower_scout()->partner());
      }

    //end the region below the left curve
    if (left_curve->has_upper_scout())
      {
        end_lower_region(left_curve->upper_scout()->partner(),
            left_curve->upper_scout());
      }

    //only start the region if point is red and blue face containing point_x
    if (!this->is_in_face_x())
      {
        return;
      }

    // from now on the event can be handled exactly as if it was
    // of type X
    handle_point_x();
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_left_endpoint(),
// since a new curve starts it may have to be guarded from below

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::inform_guard_below()
  {
    MY_CGAL_SL_PRINT("begin inform_guard_below()");

    //the right curve may be guarded from below
    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Status_line* status_line = this->different_colored_status_line();
    Subcurve* right_curve = *right_begin;

    //find_lower finds the closest curve above of the opposite color...
    std::pair<Status_line_iterator, bool> pair = status_line->find_lower(
        this->current_event_point, this->m_statusLineCurveLess);
    Status_line_iterator below = pair.first;

    //...so decrement if possible
    bool there_is_below = false;
    if (below != status_line->begin())
      {
        below--;
        there_is_below = true;
      }

    //go down the status line to find the highest upper scout
    while (below != status_line->begin() && !((*below)->has_upper_scout()))
      {
        below--;
      }

    //we found an upper scout
    if (there_is_below && below != status_line->end()
        && (*below)->has_upper_scout())
      {
        //if this upper scout guards nothing or a curve above right_curve
        //it must guard right_curve from now on
        Subcurve* temp = (*below)->upper_scout()->guarded_curve();
        if (temp == 0 || this->m_statusLineCurveLess(right_curve, temp) == -1)
          {
            (*below)->upper_scout()->set_guarded_curve(right_curve);
            (*below)->upper_scout()->set_active();
            (*below)->upper_scout()->check_individual_for_intersection();
          }
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from handle_left_endpoint(),
// since a new curve starts it might have to be guarded from above
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::inform_guard_above()
  {
    MY_CGAL_SL_PRINT("begin inform_guard_above()");

    //the right curve may be guarded from above
    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Status_line* status_line = this->different_colored_status_line();
    Subcurve* right_curve = *right_begin;

    //find the next curve above of the opposite color
    std::pair<Status_line_iterator, bool> pair = status_line->find_lower(
        this->current_event_point, this->m_statusLineCurveLess);
    Status_line_iterator above = pair.first;

    //go up the status line to find the lowest lower scout
    while (above != status_line->end() && !((*above)->has_lower_scout()))
      {
        above++;
      }

    //we found a lower scout
    if (above != status_line->end() && (*above)->has_lower_scout())
      {
        //if this lower scout guards nothing or a curve below right_curve
        //it must guard right_curve from now on
        Subcurve* temp = (*above)->lower_scout()->guarded_curve();
        if (temp == 0 || this->m_statusLineCurveLess(right_curve, temp) == 1)
          {
            (*above)->lower_scout()->set_guarded_curve(right_curve);
            (*above)->lower_scout()->set_active();
            (*above)->lower_scout()->check_individual_for_intersection();
          }
      }
  }

// ---------------------------------------------------------------------------
// checks if an event point is contained both in the red face and the blue face
// containing point_x; if not, we do not have to start a region

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  bool
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::is_in_face_x()
  {
    MY_CGAL_SL_PRINT("begin is_in_face_x");
    bool is_in_f_x = true;

    //only if a point is represented by both a red and a blue vertex
    //a region needs to be started
    Vertex_const_handle ext;
    if (this->current_event_point_color == Traits_2::RED)
      {
        try
          {
            ext = object_cast<Vertex_const_handle> (
                this->current_event_point.blue_object());
          }
        catch (Bad_object_cast)
          {
            MY_CGAL_SL_PRINT("Bad object cast");
            is_in_f_x = false;
          }
      }
    else
      {
        try
          {
            ext = object_cast<Vertex_const_handle> (
                this->current_event_point.red_object());
          }
        catch (Bad_object_cast)
          {
            MY_CGAL_SL_PRINT("Bad object cast");
            is_in_f_x = false;
          }
      }
    return is_in_f_x;
  }

// ---------------------------------------------------------------------------
// Handles events/vertices/points of type INTERNAL_ENDPOINT,
// specifically left endpoints

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::handle_left_endpoint()
  {
    MY_CGAL_SL_PRINT("begin handle_left_endpoint");

    //no left curves, one right curve
    Event_subcurve_iterator right_begin = this->right_curves_begin();
    Subcurve* right_curve = *right_begin;
    bool is_vertical = right_curve->last_curve().base().is_vertical();

    //a new curve starts here, which may have to be guarded from above...
    inform_guard_above();
    //...and / or below
    inform_guard_below();

    //check if we have to start a region...
    if (!this->is_in_face_x())
      {
        if (!is_vertical)
          {
            //...if not, do not start a region but update the current curves below
            if (this->current_event_point_color == Traits_2::RED)
              {
                this->set_current_red_below(right_curve);
              }
            else
              {
                this->set_current_blue_below(right_curve);
              }
          }
        return;
      }

    this->next_region_is_unbounded_above(false);

    //...if so, start the region below the right curve...
    start_region();

    //...and order the region above the right curve to be started
    //at the next event of type DEFAULT if there is one
    bool has_vertex_above = this->has_vertex_above();
    this->next_region_is_unbounded_above(!has_vertex_above);

    // if the right curve is not vertical, it becomes the current curve below
    if (!is_vertical)
      {
        this->delegate_to_next_default();

        if (this->current_event_point_color == Traits_2::RED)
          {
            this->set_current_red_below(right_curve);
          }
        else
          {
            this->set_current_blue_below(right_curve);
          }
      }

    //if there is nothing above, start the region above the right curve here...
    if (this->is_next_region_unbounded_above())
      {
        //...but only if the right curve is not vertical
        if (!is_vertical)
          {
            start_region();
          }
        else
          {
            this->reset_delegation();
          }
      }
  }

// ---------------------------------------------------------------------------
// Handles the event/vertex/point of type POINT_X
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::handle_point_x()
  {
    MY_CGAL_SL_PRINT("begin handle_point_x");

    //look if there is a vertex above...
    bool has_vertex_above = this->has_vertex_above();

    //...and store this information at the sweep line
    this->next_region_is_unbounded_above(!has_vertex_above);

    //if there is no vertex above, start the region here...
    if (this->is_next_region_unbounded_above())
      {
        start_region();
      }
    else
      {
        //...else start the region at the next DEFAULT vertex
        this->delegate_to_next_default();
      }
  }

// ---------------------------------------------------------------------------
// Start one new region
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::start_region()
  {
    MY_CGAL_SL_PRINT("begin start_region()");
    this->reset_delegation();

    //start two new partner scouts
    My_Scout* upper_partner;
    My_Scout* lower_partner;
    lower_partner = start_lower_scout();
    Scout_set_iterator low_it = lower_partner->scout_set_position();
    low_it++;
    upper_partner = start_upper_scout(low_it);
    lower_partner->set_partner(upper_partner);
    upper_partner->set_partner(lower_partner);

    //inform the interested scouts about the two new scouts
    lower_partner->start_lower_inform_lower_below();
    lower_partner->start_lower_inform_lower_above();
    upper_partner->start_upper_inform_upper_below();
    upper_partner->start_upper_inform_upper_above();

    //check if the two boundaries intersect
    lower_partner->check_partners_for_intersection();
  }

// ---------------------------------------------------------------------------
// Start a new upper scout
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::My_Scout*
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::start_upper_scout(
      Scout_set_iterator lower)
  {
    MY_CGAL_SL_PRINT("begin start_upper_scout()");

    Status_line* status_line = this->different_colored_status_line();

    //the scout might be a dummy...
    if (this->is_next_region_unbounded_above())
      {
        My_Scout* upper = new My_Scout(this->dummy_sub, My_Scout::UPPER_DUMMY, this);
        this->upper_dummy = upper;
        return upper;
      }

    //...otherwise its boundary is the lower right curve at the current event
    My_Scout* upper = new My_Scout(*(this->right_curves_begin()), My_Scout::UPPER, this,
        lower);

    //find the curve to guard...
    const std::pair<Status_line_iterator, bool> & other_pair =
        status_line->find_lower(this->current_event_point,
            this->m_statusLineCurveLess);
    Status_line_iterator other_above = other_pair.first;

    //...and assign the scout to guard this curve
    if (other_above != status_line->end())
      {
        upper->set_guarded_curve(*other_above);
      }
    return upper;
  }

// ---------------------------------------------------------------------------
// Start a new lower scout
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  typename My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::My_Scout*
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::start_lower_scout()
  {
    MY_CGAL_SL_PRINT("begin start_lower_scout()");

    //the scout might be a dummy
    if (this->current_color_below() == Tr::DUMMY)
      {
        //end the region if necessary
        if (this->lower_dummy != 0)
          {
            end_lower_region(this->lower_dummy, this->lower_dummy->partner());
          }

        //start a new dummy and return it
        My_Scout* lower = new My_Scout(this->dummy_sub, My_Scout::LOWER_DUMMY, this);
        this->lower_dummy = lower;
        return lower;
      }

    //we need to find the boundary for the scout to move on
    Subcurve* below_curve;

    // if there is nothing above and we are at a left endpoint,
    // the boundary of the lower scout will
    // be the right curve at the current event
    if (this->current_vertex->data().type() == INTERNAL_ENDPOINT
        && this->is_next_region_unbounded_above()
        && this->m_currentEvent->has_right_curves())
      {
        Event_subcurve_iterator right_end = this->right_curves_end();
        right_end--;
        below_curve = *right_end;
      }
    else
      {
        //the lower boundary will be the current curve below
        if (this->current_color_below() == Tr::RED)
          {
            below_curve = this->current_red_below();
          }
        else
          {
            below_curve = this->current_blue_below();
          }
      }

    //end the region below
    if (below_curve->has_lower_scout())
      {
        end_lower_region(below_curve->lower_scout(),
            below_curve->lower_scout()->partner());
      }

    //initialize the scout
    My_Scout* lower = new My_Scout(below_curve, My_Scout::LOWER, this);

    //the curve to guard is the curve below of the other color
    Subcurve* other_below_curve;

    if (this->current_color_below() == Tr::RED)
      {
        other_below_curve = this->current_blue_below();
      }
    else
      {
        other_below_curve = this->current_red_below();
      }

    //assign the scout to guard this curve
    if (other_below_curve != 0)
      {
        lower->set_guarded_curve(other_below_curve);
      }
    return lower;
  }

// ---------------------------------------------------------------------------
// Special treatment of red blue intersections before call of the
// handle_..._curves functions

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::prepare_red_blue_intersection()
  {
    MY_CGAL_SL_PRINT("begin prepare_red_blue_intersection()");

    if (this->m_currentEvent->is_intersection())
      {
        Subcurve* blue_left = *this->m_currentEvent->blue_left_curves_begin();
        Subcurve* red_left = *this->m_currentEvent->red_left_curves_begin();

        //as an exception we are working left of the event point
        this->m_currentEvent->set_use_compare_y_at_x_left(true);

        //end region
        if (blue_left->has_upper_scout() && red_left->has_lower_scout()
            && blue_left->upper_scout()->partner() == red_left->lower_scout())
          {
            end_region(red_left->lower_scout(), blue_left->upper_scout());
          }

        //end region
        if (blue_left->has_lower_scout() && red_left->has_upper_scout()
            && blue_left->lower_scout()->partner() == red_left->upper_scout())
          {
            end_region(blue_left->lower_scout(), red_left->upper_scout());
          }

        //temporarily erase the affected scouts from the scout set
        //because their order will change
        Scout_set* m_scouts = this->get_scouts();
        if (red_left->has_upper_scout())
          {
            m_scouts->erase(red_left->upper_scout()->scout_set_position());
          }
        if (red_left->has_lower_scout())
          {
            m_scouts->erase(red_left->lower_scout()->scout_set_position());
          }
        if (blue_left->has_upper_scout())
          {
            m_scouts->erase(blue_left->upper_scout()->scout_set_position());
          }

        if (blue_left->has_lower_scout())
          {
            m_scouts->erase(blue_left->lower_scout()->scout_set_position());
          }

        //end of the exception
        this->m_currentEvent->set_use_compare_y_at_x_left(false);
      }
  }

// ---------------------------------------------------------------------------
// Helper function called from prepare_red_blue_vertical(),
// Subcv* non_vertical is split already from the decomposition,
// Subcv* vertical is not split yet

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::vertical_and_non_vertical(
      Subcv* vertical, Subcv* non_vertical_left, Subcv* non_vertical_right)
  {
    MY_CGAL_SL_PRINT("begin vertical_and_non_vertical()");

    //as an exception we are working left of the event point
    this->m_currentEvent->set_use_compare_y_at_x_left(true);

    //end middle region
    if (vertical->has_lower_scout() && non_vertical_left->has_upper_scout()
        && vertical->lower_scout()->partner()
            == non_vertical_left->upper_scout())
      {
        end_region(vertical->lower_scout(), non_vertical_left->upper_scout());
      }

    //move the scouts on the non-vertical curve
    //to the right;
    //erase the affected scouts from the scout set
    //because their order will change
    Scout_set* m_scouts = this->get_scouts();
    if (non_vertical_left->has_upper_scout())
      {
        My_Scout* non_vertical_upper = non_vertical_left->upper_scout();
        non_vertical_upper->insert_into_purple_arr();
        m_scouts->erase(non_vertical_upper->scout_set_position());
        non_vertical_upper->set_boundary_curve(non_vertical_right);
        non_vertical_right->set_upper_scout(non_vertical_upper);
        non_vertical_left->remove_upper_scout();
      }
    if (non_vertical_left->has_lower_scout())
      {
        My_Scout* non_vertical_lower = non_vertical_left->lower_scout();
        non_vertical_lower->insert_into_purple_arr();
        m_scouts->erase(non_vertical_lower->scout_set_position());
        non_vertical_lower->set_boundary_curve(non_vertical_right);
        non_vertical_right->set_lower_scout(non_vertical_lower);
        non_vertical_left->remove_lower_scout();
      }

    //change assignment of possible guards
    if (non_vertical_left->is_guarded_by_upper_scout())
      {
        non_vertical_left->upper_guard()->set_guarded_curve(non_vertical_right);
      }
    if (non_vertical_left->is_guarded_by_lower_scout())
      {
        non_vertical_left->lower_guard()->set_guarded_curve(non_vertical_right);
      }

    // an intersection test is not needed, since the lower
    // guard must have been on the
    // vertical segment (so it cannot find another intersection)
    // and any upper guard will be idled by
    // the vertical segment from now on

    //temporarily erase the scouts on the vertical segment from the scout set
    if (vertical->has_upper_scout())
      {
        m_scouts->erase(vertical->upper_scout()->scout_set_position());
      }
    if (vertical->has_lower_scout())
      {
        m_scouts->erase(vertical->lower_scout()->scout_set_position());
      }

    //end of the exception
    this->m_currentEvent->set_use_compare_y_at_x_left(false);
  }

// ---------------------------------------------------------------------------
// Special treatment of red blue intersections involving a vertical segment
// before call of the handle_..._curves functions
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::prepare_red_blue_vertical()
  {
    MY_CGAL_SL_PRINT("begin prepare_red_blue_vertical()");

    //red blue intersections involving a vertical segment manifest
    //themselves as a weak intersection, because the involved non-
    //vertical segment is already split from the decomposition
    if (this->m_currentEvent->is_weak_intersection())
      {
        Subcurve* blue_left = *this->m_currentEvent->blue_left_curves_begin();
        Subcurve* red_left = *this->m_currentEvent->red_left_curves_begin();
        Subcurve* blue_right = *this->m_currentEvent->blue_right_curves_begin();
        Subcurve* red_right = *this->m_currentEvent->red_right_curves_begin();

        CGAL_assertion(blue_left->last_curve().base().is_vertical()
            || red_left->last_curve().base().is_vertical());

        if (blue_left->last_curve().base().is_vertical())
          {
            // blue_left is vertical
            // red_left and red_right are the two parts of the non-vertical
            // curve
            vertical_and_non_vertical(blue_left, red_left, red_right);
          }
        else
          {

            if (red_left->last_curve().base().is_vertical())
              {
                // red_left is vertical
                // blue_left and blue_right are the two parts of the non-vertical
                // curve
                vertical_and_non_vertical(red_left, blue_left, blue_right);
              }
          }

        //handle like a regular red blue intersection from now on
        this->m_currentEvent->set_intersection();
      }
  }

// ---------------------------------------------------------------------------
// Handles red blue intersections
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::handle_red_blue_intersection()
  {
    MY_CGAL_SL_PRINT("begin handle_red_blue_intersection()");
    Subcurve* blue_right = *this->m_currentEvent->blue_right_curves_begin();
    Subcurve* red_right = *this->m_currentEvent->red_right_curves_begin();

    //there is exactly one red and exactly one blue left curve,
    //carrying one or two scouts; get these
    My_Scout* red_scout = 0;
    My_Scout* blue_scout = 0;
    if (red_right->has_upper_scout())
      {
        red_scout = red_right->upper_scout();
      }
    else
      {
        red_scout = red_right->lower_scout();
      }
    if (blue_right->has_upper_scout())
      {
        blue_scout = blue_right->upper_scout();
      }
    else
      {
        blue_scout = blue_right->lower_scout();
      }

    //do one or two color swaps
    if (blue_scout != 0 && red_scout == 0)
      {
        scout_swap(blue_right, red_right, blue_scout);
      }
    if (blue_scout == 0 && red_scout != 0)
      {
        scout_swap(red_right, blue_right, red_scout);
      }
    if (blue_scout != 0 && red_scout != 0)
      {
        scout_swap(blue_right, red_right, blue_scout);
        scout_swap(red_right, blue_right, red_scout);
      }

    if (!red_right->last_curve().base().is_vertical())
      {
        this->set_current_red_below(red_right);
      }
    if (!blue_right->last_curve().base().is_vertical())
      {
        this->set_current_blue_below(blue_right);
      }
  }

// ---------------------------------------------------------------------------
// Helper function called by handle_red_blue_intersection(), swaps the role
// of boundary curve and guarded curve in a scout
//

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::scout_swap(
      Subcv* colored_curve, Subcv* other_colored_curve, My_Scout* colored_scout)
  {
    MY_CGAL_SL_PRINT("begin scout_swap()");

    //the order in the scout set changes; the part of the boundary
    //left of the event curve is now cut off, so the scout will be
    //inserted at the correct position of the scout set
    Scout_set* m_scouts = this->get_scouts();
    Scout_set_iterator ssi = m_scouts->insert(colored_scout);
    colored_scout->set_scout_set_position(ssi);

    //move the scout to the other colored curve
    if (colored_scout->type() == My_Scout::LOWER)
      {
        colored_scout->end_lower_inform_lower_above();
        other_colored_curve->set_lower_scout(colored_scout);
        colored_curve->remove_lower_scout();
      }
    else
      {
        colored_scout->end_upper_inform_upper_below();
        other_colored_curve->set_upper_scout(colored_scout);
        colored_curve->remove_upper_scout();
      }

    //set the curve pointers
    colored_scout->set_boundary_curve(other_colored_curve);
    colored_scout->set_guarded_curve(colored_curve);

    //the scout must be active because it is direct neighbor
    //to the guarded curve
    colored_scout->set_active();

    //treat as if a new scout was started
    if (colored_scout->type() == My_Scout::LOWER)
      {
        colored_scout->start_lower_inform_lower_above();
      }
    else
      {
        colored_scout->start_upper_inform_upper_below();
      }

    colored_scout->check_partners_for_intersection();
  }

CGAL_END_NAMESPACE
#endif

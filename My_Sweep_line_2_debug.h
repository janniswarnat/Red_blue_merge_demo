// Copyright (c) 1997  Tel-Aviv University (Israel).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2/Sweep_line_2_debug.h $
// $Id: Sweep_line_2_debug.h 41108 2007-12-06 15:26:30Z efif $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef MY_CGAL_SWEEP_LINE_2_DEBUG_H
#define MY_CGAL_SWEEP_LINE_2_DEBUG_H

#include "My_Basic_sweep_line_2.h"

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//                         DEBUG UTILITIES                                //
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

//check if the invariant holds after the handling of an event
//by Jannis Warnat

template<class Arr, class Tr, class Subcv, class Evnt, typename Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Subcv, Evnt, Alloc>::_validate_invariants()
  {
    MY_CGAL_SL_PRINT("_validate_invariants() after "
        << this->current_event_point);

    Scout_set* m_scouts = this->get_scouts();

    //number of scouts must be even since we always have pairs of scouts enclosing a region
    CGAL_assertion(!(m_scouts->size() & 1));

    bool next_is_upper = false;

    //iterate the scouts
    for (Scout_set_iterator ssi = m_scouts->begin(); ssi != m_scouts->end(); ssi++)
      {
        MY_CGAL_SL_PRINT(
            "------------------------------------------------------------");
        (*ssi)->Print();
        //dummies are always active
        if ((*ssi)->type() == My_Scout::UPPER_DUMMY)
          {
            CGAL_assertion(this->upper_dummy == (*ssi));
            CGAL_assertion((*ssi)->is_active());
          }

        if ((*ssi)->type() == My_Scout::LOWER_DUMMY)
          {
            CGAL_assertion(this->lower_dummy = (*ssi));
            CGAL_assertion((*ssi)->is_active());
          }

        //idled souts do not guard anything
        if ((*ssi)->is_idled())
          {
            CGAL_assertion((*ssi)->guarded_curve() == 0);
          }

        //make sure upper and lower dummies alternate
        if (next_is_upper)
          {
            CGAL_assertion((*ssi)->type() == Scout::UPPER || (*ssi)->type()
                == Scout::UPPER_DUMMY);
            next_is_upper = false;
          }
        else
          {
            CGAL_assertion((*ssi)->type() == Scout::LOWER || (*ssi)->type()
                == Scout::LOWER_DUMMY);
            next_is_upper = true;
          }

        if ((*ssi)->color() == Traits_2::DUMMY)
          {
            continue;
          }

        // make sure a scout always guards a curve of a color different from the color
        // the scout moves on; if a scout guards a curve, it must be the closest curve
        // of a different color
        Status_line* diff_colored_status_line;
        Status_line* same_colored_status_line;

        if ((*ssi)->color() == Traits_2::BLUE)
          {
            diff_colored_status_line = &(this->m_red_statusLine);
            same_colored_status_line = &(this->m_blue_statusLine);
          }
        if ((*ssi)->color() == Traits_2::RED)
          {
            diff_colored_status_line = &(this->m_blue_statusLine);
            same_colored_status_line = &(this->m_red_statusLine);
          }

        std::pair<Status_line_iterator, bool> pair =
            diff_colored_status_line->find_lower((*ssi)->boundary_curve(),
                this->m_statusLineCurveLess);
        Status_line_iterator above = pair.first;

        Status_line_iterator below = above;
        Subcurve* curve_above;
        Subcurve* curve_below;

        if (above == diff_colored_status_line->end() && above
            == diff_colored_status_line->begin())
          {
            curve_above = 0;
            curve_below = 0;
          }
        else
          {
            if (above == diff_colored_status_line->end() && above
                != diff_colored_status_line->begin())
              {
                curve_above = 0;
                below--;
                curve_below = *below;
              }

            if (above != diff_colored_status_line->end() && above
                == diff_colored_status_line->begin())
              {
                curve_above = *above;
                curve_below = 0;
              }

            if (above != diff_colored_status_line->end() && above
                != diff_colored_status_line->begin())
              {

                curve_above = *above;
                below--;
                curve_below = *below;
              }
          }

//        MY_CGAL_SL_PRINT("curve_above:");
//        if (curve_above != 0)
//          {
//            curve_above->Print();
//          }
//        else
//          {
//            MY_CGAL_SL_PRINT("0");
//          }

//        MY_CGAL_SL_PRINT("curve_below:");
//        if (curve_below != 0)
//          {
//            curve_below->Print();
//          }
//        else
//          {
//            MY_CGAL_SL_PRINT("0");
//          }

        CGAL_assertion(!pair.second);

        if ((*ssi)->is_active())
          {
            //an active upper scout guards the curve above of the other color
            if ((*ssi)->type() == My_Scout::UPPER)
              {
//                MY_CGAL_SL_PRINT("(*ssi)->guarded_curve()->Print():");
//                if ((*ssi)->guarded_curve() != 0)
//                  {
//                    (*ssi)->guarded_curve()->Print();
//                  }
//                else
//                  {
//                    MY_CGAL_SL_PRINT("0");
//                  }
                CGAL_assertion((*ssi)->guarded_curve() == curve_above);
              }

            if ((*ssi)->type() == My_Scout::LOWER)
              {
                if (curve_below != 0)
                  {
//                    MY_CGAL_SL_PRINT("(*ssi)->guarded_curve()->Print():");
//                    if ((*ssi)->guarded_curve() != 0)
//                      {
//                        (*ssi)->guarded_curve()->Print();
//                      }
//                    else
//                      {
//                        MY_CGAL_SL_PRINT("0");
//                      }
                    CGAL_assertion((*ssi)->guarded_curve() == curve_below
                        || this->current_event_point.base().x()
                            == curve_below->last_curve().base().left().x());
                  }
                else
                  {
                    CGAL_assertion(
                        (*ssi)->guarded_curve() == curve_below
                            || this->current_event_point.base().x()
                                == (*ssi)->guarded_curve()->last_curve().base().right().x());
                  }
              }
          }
        else //idled
          {
            if ((*ssi)->type() == My_Scout::UPPER)
              {
                Scout_set_reverse_iterator self = Scout_set_reverse_iterator(
                    (*ssi)->scout_set_position());
                self--;
                bool there_is_idling_above = false;
                My_Scout* idling_scout = 0;

                //walk up the scout set
                Scout_set_reverse_iterator u_plus = self;
                while (u_plus != m_scouts->rbegin())
                  {
                    //skip lower scouts
                    u_plus--;
                    u_plus--;

                    if ((*u_plus)->color() == (*self)->color())
                      {
                        //we must go up until we find an active one,
                        //the scouts in between might be idled
                        if ((*u_plus)->is_active())
                          {

                            there_is_idling_above = true;
                            idling_scout = *u_plus;
                            break;
                          }
                      }
                  }

                if (curve_above == 0)
                  {

                  }
                else
                  {
                    CGAL_assertion(there_is_idling_above && curve_above
                        == idling_scout->guarded_curve());
                  }
              }

            if ((*ssi)->type() == My_Scout::LOWER)
              {
                Scout_set_iterator self = (*ssi)->scout_set_position();
                bool there_is_idling_below = false;
                My_Scout* idling_scout = 0;

                //walk down the scout set
                Scout_set_iterator l_minus = self;
                while (l_minus != m_scouts->begin())
                  {
                    //skip upper scouts
                    l_minus--;
                    l_minus--;

                    if ((*l_minus)->color() == (*self)->color())
                      {
                        //we must go down until we find an active one,
                        //the scouts in between might be idled
                        if ((*l_minus)->is_active())
                          {
                            there_is_idling_below = true;
                            idling_scout = *l_minus;
                            break;
                          }
                      }
                  }
                if (curve_below == 0)
                  {

                  }
                else
                  {
                    CGAL_assertion(there_is_idling_below && curve_below
                        == idling_scout->guarded_curve());
                  }

              }
          }
      }

    CGAL_assertion(!next_is_upper);

    // do some checks for the red status line too
    for (Status_line_iterator sli = this->red_status_line_begin(); sli
        != this->red_status_line_end(); sli++)
      {
        CGAL_assertion((*sli)->last_curve().color() == Traits_2::RED);

        if ((*sli)->is_guarded_by_lower_scout())
          {
            CGAL_assertion((*sli)->lower_guard()->is_active());
            CGAL_assertion((*sli)->lower_guard()->color() == Traits_2::BLUE);
          }

        if ((*sli)->is_guarded_by_upper_scout())
          {
            CGAL_assertion((*sli)->upper_guard()->is_active());
            CGAL_assertion((*sli)->upper_guard()->color() == Traits_2::BLUE);
          }
      }

    // do some checks for the blue status line too
    for (Status_line_iterator sli = this->blue_status_line_begin(); sli
        != this->blue_status_line_end(); sli++)
      {
        CGAL_assertion((*sli)->last_curve().color() == Traits_2::BLUE);

        if ((*sli)->is_guarded_by_lower_scout())
          {
            CGAL_assertion((*sli)->lower_guard()->is_active());
            CGAL_assertion((*sli)->lower_guard()->color() == Traits_2::RED);
          }

        if ((*sli)->is_guarded_by_upper_scout())
          {
            CGAL_assertion((*sli)->upper_guard()->is_active());
            CGAL_assertion((*sli)->upper_guard()->color() == Traits_2::RED);
          }
      }
  }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintEventQueue()
  {
    MY_CGAL_SL_DEBUG(std::cout << std::endl << "Event queue: " << std::endl;)
    Event_queue_iterator iter = m_queue->begin();
    while (iter != m_queue->end())
      {
        MY_CGAL_SL_DEBUG(std::cout << "Point " << (*iter)->point() << std::endl;)
        //Event *e = iter->second;
        Event *e = *iter;
        e->Print();
        ++iter;
      }
MY_CGAL_SL_DEBUG  (std::cout << "--------------------------------" << std::endl;)

}

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintRedSubCurves()
  {
    MY_CGAL_SL_DEBUG(std::cout << std::endl << "Red Sub curves: " << std::endl;)
    for(unsigned int i=0;
        i < m_num_of_red_subCurves;
++      i)
        {
          m_red_subCurves[i].Print();
        }
    }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintBlueSubCurves()
  {
    MY_CGAL_SL_DEBUG(std::cout << std::endl << "Blue Sub curves: " << std::endl;)
    for(unsigned int i=0;
        i < m_num_of_blue_subCurves;
++      i)
        {
          m_blue_subCurves[i].Print();
        }
    }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintRedStatusLine()
  {
    if (m_red_statusLine.size() == 0)
      {
        std::cout << std::endl << "Red Status line: empty" << std::endl;
        return;
      }
    std::cout << std::endl << "Red Status line: (";
    if (m_currentEvent->is_finite())
      std::cout << current_event_point << std::endl;
    else
      {
        Arr_parameter_space x = m_currentEvent->parameter_space_in_x(), y =
            m_currentEvent->parameter_space_in_y();

        PrintInfinityType(x, y);
      }
    Status_line_iterator iter = m_red_statusLine.begin();
    while (iter != m_red_statusLine.end())
      {
        (*iter)->Print();
        ++iter;
      }
    std::cout << "Red_Status line - end" << std::endl;
  }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintBlueStatusLine()
  {
    if (m_blue_statusLine.size() == 0)
      {
        std::cout << std::endl << "Blue Status line: empty" << std::endl;
        return;
      }
    std::cout << std::endl << "Blue Status line: (";
    if (m_currentEvent->is_finite())
      std::cout << current_event_point << std::endl;
    else
      {
        Arr_parameter_space x = m_currentEvent->parameter_space_in_x(), y =
            m_currentEvent->parameter_space_in_y();

        PrintInfinityType(x, y);
      }
    Status_line_iterator iter = m_blue_statusLine.begin();
    while (iter != m_blue_statusLine.end())
      {
        (*iter)->Print();
        ++iter;
      }
    std::cout << "Blue_Status line - end" << std::endl;
  }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintInfinityType(
      Arr_parameter_space ps_x, Arr_parameter_space ps_y)
  {
    switch (ps_x)
      {
    case ARR_LEFT_BOUNDARY:
      std::cout << "left boundary";
      return;
    case ARR_RIGHT_BOUNDARY:
      std::cout << "right boundary";
      return;
    case ARR_INTERIOR:
    default:
      break;
      }

    switch (ps_y)
      {
    case ARR_BOTTOM_BOUNDARY:
      std::cout << "bottom boundary";
      return;
    case ARR_TOP_BOUNDARY:
      std::cout << "top boundary";
      return;
    case ARR_INTERIOR:
    default:
      CGAL_error();
      }
  }

template<class Arr, class Tr, class Crv, class Evnt, class Alloc>
  void
  My_Basic_sweep_line_2<Arr, Tr, Crv, Evnt, Alloc>::PrintEvent(const Event* e)
  {
    if (e->is_finite())
      std::cout << e->point();
    else
      {
        Arr_parameter_space x = e->parameter_space_in_x();
        Arr_parameter_space y = e->parameter_space_in_y();
        PrintInfinityType(x, y);
        std::cout << " with unbounded curve: " << e->curve();
      }
  }

#endif

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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/include/CGAL/Sweep_line_2.h $
// $Id: Sweep_line_2.h 40185 2007-09-04 12:08:08Z golubevs $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 (based on old version by Tali Zvi)

#ifndef MY_CGAL_SWEEP_LINE_2_H
#define MY_CGAL_SWEEP_LINE_2_H

/*! \file
 * Definition of the Sweep_line_2 class.
 */
#include "My_Basic_sweep_line_2.h"

CGAL_BEGIN_NAMESPACE

/*! \class
 Sweep_line_2 is a class that implements the sweep line algorithm
 based on the algorithm of Bentley and Ottmann.
 It extends the algorithm to support not only segments but general x-monotone curves
 as well and isolated points.
 The X_monotone_curve_2 type and Point_2 are defined by the traits class that
 is one of the template arguments.

 The algorithm is also extended to support the following degenerate cases:
 - vertical segments
 - multiple (more then two) curves intersecting at one point
 - curves beginning and ending on other curves.
 - overlapping curves

 General flow:
 After the initialization stage, the events are handled from left to right.

 For each event

 First pass - handles special cases in which curves start or end
 at the interior of another curve
 Handle left curves - iterate over the curves that intersect
 at the event point and defined lexicographically to the left of the
 event.
 Handle right curves - iterate over the curves that intersect
 the event point and defined lexicographically to the right of the
 event point. This is where new intersection points
 are calculated.
 End

 Convensions through out the code:
 In order to make the code as readable as possible, some convensions were
 made in regards to variable naming:

 xp - is the intersection point between two curves
 slIter - an iterator to the status line, always points to a curve.

 */
template< class Arrangement_,
class Traits_,
class Subcurve_,
class Event_,
typename Allocator_>
class My_Sweep_line_2 : public My_Basic_sweep_line_2<Arrangement_,
Traits_,
Subcurve_,
Event_,
Allocator_>
  {
  public:

    typedef Arrangement_ Arrangement_2;
    typedef Traits_ Traits_2;
    typedef Event_ Event;
    typedef Subcurve_ Subcurve;
    typedef Allocator_ Allocator;

    typedef My_Basic_sweep_line_2<Arrangement_2,
    Traits_2,
    Subcurve,
    Event,
    Allocator> Base;

    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_handle Halfedge_handle;
    typedef typename Base::My_Scout My_Scout;

    typedef typename Base::Traits_adaptor_2 Traits_adaptor_2;
    typedef typename Traits_adaptor_2::Point_2 Point_2;
    typedef typename Traits_adaptor_2::X_monotone_curve_2 X_monotone_curve_2;

    typedef typename Event::Subcurve_iterator Event_subcurve_iterator;

    typedef typename Base::Base_event Base_event;
    typedef typename Base_event::Attribute Attribute;

    typedef typename Base::Base_subcurve Base_subcurve;

    typedef typename Base::Status_line Status_line;
    typedef typename Base::Status_line_iterator Status_line_iterator;
    typedef typename Base::Scout_set Scout_set;
    typedef typename Base::Scout_set_iterator Scout_set_iterator;

    typedef CGAL::Curve_pair<Subcurve> Curve_pair;
    typedef CGAL::Curve_pair_hasher<Subcurve> Curve_pair_hasher;
    typedef CGAL::Equal_curve_pair<Subcurve> Equal_curve_pair;
    typedef Open_hash<Curve_pair,
    Curve_pair_hasher,
    Equal_curve_pair> Curve_pair_set;

    typedef random_access_input_iterator<std::vector<Object> >
    vector_inserter;

  protected:

    // Data members:

    Curve_pair_set m_curves_pair_set;  // A lookup table of pairs of Subcurves
    // that have been intersected.

    std::vector<Object> m_x_objects; // Auxiliary vector for storing the
    // intersection objects.

    X_monotone_curve_2 sub_cv1; // Auxiliary varibales
    X_monotone_curve_2 sub_cv2; // (for splitting curves).

  public:

    /*!
     * Constructor.
     * \param visitor A pointer to a sweep-line visitor object.
     */
    My_Sweep_line_2 (/*Visitor *visitor*/):
    m_curves_pair_set(0)
      {
      }

    /*!
     * Constructor.
     * \param traits A pointer to a sweep-line traits object.
     * \param visitor A pointer to a sweep-line visitor object.
     */
    My_Sweep_line_2 (Traits_2 *traits/*, Visitor *visitor*/):
    m_curves_pair_set(0)
      {
      }

    /*! Destrcutor. */
    virtual ~My_Sweep_line_2()
      {
      }

  public:

    /*! Initialize the data structures for the sweep-line algorithm. */
    virtual void _init_curve_pairs();

    /*! Complete the sweep process (complete the data structures). */
    virtual void _complete_sweep ();

    /*! Handle the subcurves to the left of the current event point. */
    virtual void _handle_red_left_curves ();
    virtual void _handle_blue_left_curves();

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

    /*! Fix overlapping subcurves before handling the current event. */
    void _fix_overlap_subcurves_from_red();
    void _fix_overlap_subcurves_from_blue();

    /*!
     * Handle overlap at right insertion to event.
     * \param event The event point.
     * \param curve The subcurve representing the overlap.
     * \param iter An iterator for the curves.
     * \param overlap_exist
     */
    void _handle_overlap_from_red (Event* event, Subcurve* curve,
        Event_subcurve_iterator iter, bool overlap_exist);
    void _handle_overlap_from_blue (Event* event, Subcurve* curve,
        Event_subcurve_iterator iter, bool overlap_exist);

    /*!
     * Compute intersections between the two given curves.
     * If the two curves intersect, create a new event (or use the event that
     * already exits in the intersection point) and insert the curves to the
     * event.
     * \param curve1 The first curve.
     * \param curve2 The second curve.
     */
    void _intersect_from_red (Subcurve *c1, Subcurve *c2);
    void _intersect_from_blue (Subcurve *c1, Subcurve *c2);

    /*!
     * When a curve is removed from the status line for good, its top and
     * bottom neighbors become neighbors. This method finds these cases and
     * looks for the intersection point, if one exists.
     * \param leftCurve A pointer to the curve that is about to be deleted.
     * \param remove_for_good Whether the aubcurve is removed for good.
     */
    void _remove_curve_from_red_status_line (Subcurve *leftCurve,
        bool remove_for_good);
    void _remove_curve_from_blue_status_line (Subcurve *leftCurve,
        bool remove_for_good);

    /*!
     * Create an intersection-point event between two curves.
     * \param xp The intersection point.
     * \param mult Its multiplicity.
     * \param curve1 The first curve.
     * \param curve2 The second curve.
     * \param is_overlap Whether the two curves overlap at xp.
     */
    void _create_intersection_point_from_red (Point_2 xp,
        unsigned int mult,
        Subcurve* c1,
        Subcurve* c2,
        bool is_overlap = false);

    void _create_intersection_point_from_blue (Point_2 xp,
        unsigned int mult,
        Subcurve* c1,
        Subcurve* c2,
        bool is_overlap = false);

    /*!
     * Fix a subcurve that represents an overlap.
     * \param sc The subcurve.
     */
    void _fix_finished_overlap_subcurve (Subcurve* sc);

    //functions by Jannis Warnat defined and commented
    // in My_Sweep_line_handler.h
    virtual void prepare_red_blue_intersection();
    virtual void prepare_red_blue_vertical();
    virtual void proceed_according_to_vertex_type();
    void handle_right_endpoint();
    void handle_left_endpoint();
    bool is_in_face_x();
    void start_region();
    void handle_point_x();
    void handle_red_blue_intersection();
    void scout_swap(Subcurve* blue_right, Subcurve* red_right, My_Scout* sc);
    void handle_default();
    void inform_guard_above();
    void inform_guard_below();
    void end_region(My_Scout* lower, My_Scout* upper);
    void end_lower_region(My_Scout* lower, My_Scout* upper);
    void vertical_and_non_vertical(Subcurve* vertical, Subcurve* non_vertical_left, Subcurve* non_vertical_right);
    void change_assignment_of_lower_guard(Subcurve* left);
    void change_assignment_of_upper_guard(Subcurve* left);
    void change_boundary_of_lower_scout(Subcurve* left);
    void change_boundary_of_upper_scout(Subcurve* left);
    My_Scout* start_lower_scout();
    My_Scout* start_upper_scout(Scout_set_iterator lower);
  };

CGAL_END_NAMESPACE

// The member-function definitions can be found in:
#include "My_Sweep_line_2_impl.h"
#include "My_Sweep_line_handler.h"

#endif

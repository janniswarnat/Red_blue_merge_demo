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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/demo_tab.h $
// $Id: demo_tab.h 41314 2007-12-24 16:43:18Z guyzucke $
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Efi Fogel       <efif@post.tau.ac.il>

#ifndef DEMO_TAB_H
#define DEMO_TAB_H

/*! demo_tab.h contain the definition and implementation of
 *  the demo tab classes and the tabs traits classes.
 *  all the possible shared code is in Qt_widget_demo_tab where
 *  the differences is in the traits classes.
 */
#include "base_tab.h"
#include "Red_blue_divide_and_conquer.h"

/*! template class Qt_widget_demo_tab gets a Tab_traits class as
 *  a template parameter. all the Tab_traits classes must support
 *  a set of demands.
 */

template<class TabTraits>
  class Qt_widget_demo_tab : public Qt_widget_base_tab
  {
        typedef Traits_2::Kernel Kernel;

    typedef typename TabTraits::Point_2 Point_2;
    typedef typename TabTraits::X_monotone_curve_2 X_monotone_curve_2;
    typedef typename TabTraits::Vertex_iterator Vertex_iterator;
    typedef typename TabTraits::Edge_iterator Edge_iterator;

  public:
    /*! m_tab_traits - the traits object */
    TabTraits m_tab_traits;

    /*! Original Traits and Kernel */
    Traits_2 m_traits;
    Kernel kernel;

    /*! purple - pointer for the tab planar map */
    Arrangement_2* purple;

    // arrangement to test the curves for degeneracies
    Arrangement_2* test_for_degen;

    // the input curves
    std::list<X_monotone_curve_2> xcurves;

    // the input point_x
    Point_2 point_x;
    bool point_x_set;
    bool x_not_isolated;

    // pivot to rotate around, about the lower left corner of the bounding box
    Point_2 pivot;

    // helps with the deletion of a curve
    typename std::list<X_monotone_curve_2>::iterator removable_curve;
    typename std::list<X_monotone_curve_2>::iterator prev_removable_curve;
    bool removable_curve_set;

    /*! constructor
     *\ param parent - widget parent window
     *\ param tab_number - widget program index
     */
    Qt_widget_demo_tab(QWidget * parent , int tab_number);

    /*! destructor - delete the planar map and the point location pointer
     */
    ~Qt_widget_demo_tab();


    // test if an arrangement of the input curves leads to a degenerate
    // situation
    bool check_for_degen();

    void set_point_x(Point_2 p);

    void reset_point_x();

    // called from draw() if we need to recompute the face containing point_x
    void compute();

    //update pivot if segments or point_x change
    Point_2 compute_pivot();

    /*! draw - called everytime something changed, draw the PM and mark the
     *         point location if the mode is on.
     */
    void draw();

    void draw_arrangement_edges(Arrangement_2* a, QColor c, int i);

    void draw_arrangement_vertices(Arrangement_2* a, QColor c, int i);

    void draw_arrangement(Arrangement_2* arr,QColor c, int width);

    /*! mousePressEvent - mouse click on the tab
     *\ param e - mouse click event
     */
    void mousePressEvent(QMouseEvent *e);

    /*! insert - insert a curve to the planar map
     *\ param e - mouse click event
     *\ param p - the pressed point
     */
    void insert(QMouseEvent *e, Coord_point p);

    /* mousePressEvent_point_location - creats the point location point
     param e - mouse click event*/
    void mousePressEvent_point_location(QMouseEvent *e);

    /*! is_pure - insure no special button is pressed
     *\ param s - keyboard modifier flags that existed
     *  immediately before the event occurred.
     *\ return true if one of them existed, false otherway.
     */
    bool is_pure(Qt::ButtonState s);

    /*! find_removeable_halfedges - find removable curve in the tab
     *\ param e - mouse click event
     */
    void find_removable_halfedges(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);

    /*! mouseMoveEvent - enable seeing the line to be drawn
     *\ param e - mouse click event
     */

    void mouseMoveEvent(QMouseEvent *e);

    /*! leaveEvent - hide the line if you leave the widget's area on the screen
     *\ param e - mouse click event
     */
    void leaveEvent(QEvent * /* e */);

    /*! point
     *\ params x,y - the mouse clicked point coordinates
     *\    return a point according to the current snap mode and
     *  recent points.
     */
    Coord_point point(Coord_type x, Coord_type y);

    bool is_empty();

  };

/*! class Segment_tab_traits defines the segment traits
 */
class Segment_tab_traits
{
public:
  typedef Traits_2::Point_2 Point_2;
  typedef Traits_2::Kernel Kernel;
  typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;
  typedef Arrangement_2::Vertex_iterator Vertex_iterator;
  typedef Arrangement_2::Edge_iterator Edge_iterator;

public:

  /*! coordinate scale - used in conics*/
  int COORD_SCALE;

  /*! constructor */
  Segment_tab_traits() :
    COORD_SCALE(1)
  {
  }

  /*! destructor */
  ~Segment_tab_traits()
  {
  }

  /*! draw_xcurve - use Qt_Widget operator to draw
   *\ param w - the demo widget
   *\ c - xcurve to be drawen
   */
  void
  draw_xcurve(Qt_widget_demo_tab<Segment_tab_traits> * w, X_monotone_curve_2 c)
  {
    (*w) << c;
  }

  /*! first_point - a first point of inserted segment
   */
  void
  first_point(Coord_point p, Mode)
  {
    m_p1 = m_p2 = p;
  }

  /*! Obtain the last point of a segment
   */
  void
  middle_point(Coord_point p, Qt_widget_demo_tab<Segment_tab_traits> * w)
  {
    Coord_kernel ker;

    if (!ker.equal_2_object()(m_p1, p))
      {
        get_segment(Coord_segment(m_p1, p), w);

        w->active = false;
        //w->redraw();  // not working so I use new_object instead
        w->new_object(make_object(Coord_segment(m_p1, p)));
      }
  }

  /*! get_segment - create a new segment, insert him into curves_list
   * and planar map
   */
  void
  get_segment(Coord_segment coord_seg,
      Qt_widget_demo_tab<Segment_tab_traits> * w)
  {
    const Coord_point & coord_source = coord_seg.source();
    const Coord_point & coord_target = coord_seg.target();
    Point_2 source(coord_source.x(), coord_source.y());
    Point_2 target(coord_target.x(), coord_target.y());
    X_monotone_curve_2 seg(source, target);
    if((w->xcurves).empty())
      {
        w->bbox = seg.bbox();
      }
    else
      {
        CGAL::Bbox_2 curve_bbox = seg.bbox();
        w->bbox = w->bbox + curve_bbox;
      }
    (w->xcurves).push_back(seg);
  }

  /*! xcurve_point_distance - return the distance between a point
   * and a xsegment
   */
  Coord_type
  xcurve_point_distance(Coord_point p, X_monotone_curve_2 & c,
      Qt_widget_demo_tab<Segment_tab_traits> *)
  {
    const Point_2 & left = c.left();
    const Point_2 & right = c.right();

    Coord_type x1 = CGAL::to_double(left.x());
    Coord_type y1 = CGAL::to_double(left.y());

    Coord_type x2 = CGAL::to_double(right.x());
    Coord_type y2 = CGAL::to_double(right.y());

    Coord_point coord_left(x1, y1);
    Coord_point coord_right(x2, y2);
    Coord_segment coord_seg(coord_left, coord_right);
    return CGAL::squared_distance(p, coord_seg);
  }

  /*! draw_last_segment - call from mouse move event
   */
  void
  draw_last_segment(Qt_widget_demo_tab<Segment_tab_traits> * w)
  {
    *w << Coord_segment(m_p1, m_p2);
  }

  /*! draw_current_segment - call from mouse move event
   */
  void
  draw_current_segment(Coord_point p,
      Qt_widget_demo_tab<Segment_tab_traits> * w)
  {
    *w << Coord_segment(m_p1, p);
    m_p2 = p;
  }

  /*! temporary points of the created segment */
  Traits_2 m_traits;
  Coord_point m_p1, m_p2;

};


#endif //DEMO_TAB_H

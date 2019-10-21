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

/*! demo_tab.h contain the definition and implementation of
 *  the demo tab classes and the tabs traits classes.
 *  all the possible shared code is in Qt_widget_demo_tab where
 *  the differences is in the traits classes.
 */

#include "demo_tab.h"

template<class TabTraits>
  Qt_widget_demo_tab<TabTraits>::Qt_widget_demo_tab(QWidget * parent,
      int tab_number) :
    Qt_widget_base_tab(parent, tab_number), purple(new Arrangement_2()),
        test_for_degen(0), point_x(Point_2()), point_x_set(false),
        x_not_isolated(false), removable_curve(), prev_removable_curve(),
        removable_curve_set(false)
  {
  }

/*! Destructor - delete the planar map and the point location pointer
 */
template<class TabTraits>
  Qt_widget_demo_tab<TabTraits>::~Qt_widget_demo_tab()
  {
    if (test_for_degen != 0)
      {
        delete test_for_degen;
        test_for_degen = 0;
      }
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::set_point_x(Point_2 p)
  {
    point_x_set = true;
    x_not_isolated = false;
    point_x = p;

    //make sure point_x does not lie on on of the curves
    for (typename std::list<X_monotone_curve_2>::iterator ei = xcurves.begin(); ei
        != xcurves.end(); ei++)
      {
        if (ei->is_vertical()) // vertical segments
          {
            if (point_x.x() == ei->left().x())
              {
                if (point_x.y() >= ei->left().y())
                  {
                    if (point_x.y() <= ei->right().y())
                      {
                        std::cout << "X NOT ISOLATED" << std::endl;
                        x_not_isolated = true;
                      }
                  }
              }
          }
        else // non-vertical
          {
            if (kernel.compare_y_at_x_2_object()(point_x, ei->line())
                == CGAL::EQUAL)
              {
                std::cout << "X NOT ISOLATED" << std::endl;
                x_not_isolated = true;
              }
          }
      }
    bbox = bbox + point_x.bbox();

    //the pivot might have changed
    pivot = compute_pivot();
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::reset_point_x()
  {
    point_x = Point_2();
    point_x_set = false;
    x_not_isolated = false;
    something_changed();
  }

// test if an arrangement of the input curves leads to degenerate
// situation
template<class TabTraits>
  bool
  Qt_widget_demo_tab<TabTraits>::check_for_degen()
  {
    QCursor old = cursor();
    setCursor(Qt::WaitCursor);

    bool degen = false;

    if (test_for_degen != 0)
      {
        delete test_for_degen;
        test_for_degen = 0;
      }

    test_for_degen = new Arrangement_2();
    QColor c = Qt::green;

    clock_t t1, t2;

    t1 = clock();
    CGAL::insert(*test_for_degen, xcurves.begin(), xcurves.end());
    t2 = clock();

    std::cout << "aggregated construction time = " << double(t2 - t1)
        / CLOCKS_PER_SEC << std::endl;

    for (Vertex_iterator vit = test_for_degen->vertices_begin(); vit
        != test_for_degen->vertices_end(); vit++)
      {
        if (!((vit->degree() == 1) || (vit->degree() == 4)))
          {
            setColor(c);
            setLineWidth(5);
            (*this) << vit->point();
            setLineWidth(1);
            std::cout << "degenerate" << std::endl;
            degen = true;
          }
      }

    delete test_for_degen;
    test_for_degen = 0;

    setCursor(old);
    return degen;
  }

// called from draw() if we need to recompute the single face containing point_x
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::compute()
  {
    if (purple != 0)
      {
        delete purple;
        purple = 0;
      }

    this->decomp_time = 0;
    this->sweep_time = 0;
    this->rotate_time = 0;
    this->fuse_time = 0;

    clock_t t1, t2;

    t1 = clock();

    //start our algorithm to find the single face containing point_x
    purple = CGAL::red_blue_divide_and_conquer<Arrangement_2,
        typename std::list<X_monotone_curve_2>::iterator>(xcurves.begin(),
        xcurves.end(), point_x, pivot);
    t2 = clock();
    double dc_time = double(t2 - t1);

    std::cout << "red_blue_divide_and_conquer time = " << dc_time
        / CLOCKS_PER_SEC << std::endl;
    std::cout << "decomposition time = " << decomp_time / CLOCKS_PER_SEC
        << std::endl;
    std::cout << "sweep time = " << sweep_time / CLOCKS_PER_SEC << std::endl;
    std::cout << "rotation time = " << rotate_time / CLOCKS_PER_SEC
        << std::endl;
    std::cout << "fuse time = " << fuse_time / CLOCKS_PER_SEC << std::endl;
  }

template<class TabTraits>
  typename TabTraits::Point_2
  Qt_widget_demo_tab<TabTraits>::compute_pivot()
  {
    double pivot_x = bbox.xmin();
    double pivot_y = bbox.ymin();

    pivot_x -= 2;
    pivot_y -= 2;

    int pivot_x_int = (int) pivot_x;
    int pivot_y_int = (int) pivot_y;

    return Point_2(pivot_x_int, pivot_y_int);
  }

/*! draw - called everytime something changed, draw the PM and mark the
 *         point location if the mode is on.
 */
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::draw()
  {
    this->setFocus();
    QCursor old = cursor();
    setCursor(Qt::WaitCursor);

    setLineWidth(2);
    setColor(Qt::black);

    typename std::list<X_monotone_curve_2>::iterator ci;
    for (ci = xcurves.begin(); ci != xcurves.end(); ci++)
      {
        m_tab_traits.draw_xcurve(this, *ci);
      }

    //draw a degenerate point_x
    if (x_not_isolated)
      {
        setColor(Qt::green);
        setLineWidth(5);
      }

    //draw point_x
    if (point_x_set)
      {
        (*this) << point_x;
      }

    //during deletion of a curve
    setColor(Qt::green);
    setLineWidth(2);
    if (removable_curve_set)
      {
        m_tab_traits.draw_xcurve(this, *removable_curve);
      }

    //compute the single face
    if (real_change && point_x_set && !x_not_isolated)
      {
        compute();
        real_change = false;
      }

    //draw the face stored in the purple arrangement
    if (purple != 0 && point_x_set && !x_not_isolated)
      {
        draw_arrangement(purple, Qt::darkMagenta, 6);
      }

    setCursor(old);
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::draw_arrangement_edges(Arrangement_2* a,
      QColor c, int i)
  {
    if (a != 0)
      {
        for (Edge_iterator ei = a->edges_begin(); ei != a->edges_end(); ++ei)
          {
            setColor(c);
            setLineWidth(i);
            m_tab_traits.draw_xcurve(this, ei->curve());
          }
      }
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::draw_arrangement_vertices(Arrangement_2* a,
      QColor c, int i)
  {
    if (a != 0)
      {
        setLineWidth(i);
        setColor(c);

        //visualize the vertex type
        for (Vertex_iterator vi = a->vertices_begin(); vi != a->vertices_end(); ++vi)
          {
            setPointStyle(CGAL::PLUS);
            if (vi->data().type() == INTERNAL_ENDPOINT)
              {
                this->setPointStyle(CGAL::BOX);
              }
            if (vi->data().type() == EXTERNAL_ENDPOINT)
              {
                this->setPointStyle(CGAL::DISC);
              }
            if (vi->data().type() == DEFAULT)
              {
                this->setPointStyle(CGAL::DISC);
              }

            if (vi->data().type() == POINT_X)
              {
                this->setPointStyle(CGAL::CROSS);
                (*this) << vi->point();
              }
            //(*this) << vi->point();
          }
      }
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::draw_arrangement(Arrangement_2* arr, QColor c,
      int width)
  {
    draw_arrangement_edges(arr, c, width);
    draw_arrangement_vertices(arr, c, width);
  }

/*! mousePressEvent - mouse click on the tab
 *\ param e - mouse click event
 */
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::mousePressEvent(QMouseEvent *e)
  {
    QCursor old = cursor();
    setCursor(Qt::WaitCursor);

    if (mode == MODE_POINT_LOCATION)
      {
        mousePressEvent_point_location(e);
        setCursor(old);
        return;
      }

    if (mode == MODE_DELETE)
      {
        if (!removable_curve_set)
          {
            setCursor(old);
            return;
          }
        else
          {
            xcurves.erase(removable_curve);
            removable_curve_set = false;
            something_real_changed();
            setCursor(old);
            return;
          }
      }

    if (mode == MODE_INSERT)
      {
        Coord_type x, y;
        x_real(e->x(), x);
        y_real(e->y(), y);
        Coord_point p = point(x, y);

        lock();
        QColor old_color = color();
        RasterOp old_rasterop = rasterOp();
        get_painter().setRasterOp(XorROP);

        insert(e, p);

        setRasterOp(old_rasterop);
        setColor(old_color);
        unlock();

        setCursor(old);
        return;
      }
  }

/*! insert - insert a curve to the planar map
 *\ param e - mouse click event
 *\ param p - the pressed point
 */
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::insert(QMouseEvent *e, Coord_point p)
  {
    if (e->button() == Qt::LeftButton && is_pure(e->state()))
      {
        if (!active)
          {
            active = true;
            m_tab_traits.first_point(p, mode);
          }
        else
          {
            m_tab_traits.middle_point(p, this);
          }
      }

  }

/* mousePressEvent_point_location - creates the point location point
 param e - mouse click event*/
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::mousePressEvent_point_location(QMouseEvent *e)
  {
    if (e->button() == Qt::LeftButton && is_pure(e->state()))
      {
        Coord_type x, y;
        x_real(e->x(), x);
        y_real(e->y(), y);

        new_object(make_object(Coord_point(x * m_tab_traits.COORD_SCALE, y
            * m_tab_traits.COORD_SCALE)));
      }

    if (e->button() == Qt::RightButton && is_pure(e->state()))
      {
        reset_point_x();
      }
  }

/*! is_pure - insure no special button is pressed
 *\ param s - keyboard modifier flags that existed
 *  immediately before the event occurred.
 *\ return true if one of them existed, false otherway.
 */
template<class TabTraits>
  bool
  Qt_widget_demo_tab<TabTraits>::is_pure(Qt::ButtonState s)
  {
    if ((s & Qt::ControlButton) || (s & Qt::ShiftButton) || (s & Qt::AltButton))
      return 0;
    else
      return 1;
  }

/*! find_removeable_halfedges - find removable curve in the tab
 *\ param e - mouse click event
 */
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::find_removable_halfedges(QMouseEvent *e)
  {
    //  if the arrangement is empty do nothing
    if (xcurves.empty())
      return;

    Coord_point p(x_real(e->x()) * m_tab_traits.COORD_SCALE, y_real(e->y())
        * m_tab_traits.COORD_SCALE);

    bool is_first = true;
    Coord_type min_dist = 0;
    typename std::list<X_monotone_curve_2>::iterator cli;
    typename std::list<X_monotone_curve_2>::iterator closest_cli;

    for (cli = xcurves.begin(); cli != xcurves.end(); cli++)
      {
        X_monotone_curve_2 & xcurve = *cli;
        Coord_type dist = m_tab_traits.xcurve_point_distance(p, xcurve, this);
        if (is_first || dist < min_dist)
          {
            min_dist = dist;
            closest_cli = cli;
            is_first = false;
          }
      }

    prev_removable_curve = removable_curve;
    removable_curve = closest_cli;
    removable_curve_set = true;
    if (prev_removable_curve != removable_curve)
      {
        something_changed();
      }
  }

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::keyPressEvent(QKeyEvent *e)
  {
    const double dx = this->x_real(200) - this->x_real(0);
    const double dy = this->y_real(200) - this->y_real(0);
    const double ypage = this->y_max() - this->y_min();
    if (e->key() == Qt::Key_Right)
      {
        this->move_center(dx, 0);
      }
    else if (e->key() == Qt::Key_Left)
      {
        this->move_center(-dx, 0);
      }
    else if (e->key() == Qt::Key_Up)
      {
        this->move_center(0, -dy);
      }
    else if (e->key() == Qt::Key_Down)
      {
        this->move_center(0, dy);
      }
    else if (e->key() == Qt::Key_Next)
      { //PageUp
        this->move_center(0, -ypage / 2);
      }
    else if (e->key() == Qt::Key_Prior)
      { //PageDown
        this->move_center(0, ypage / 2);
      }
  }

/*! mouseMoveEvent - enable seeing the line to be drawn
 *\ param e - mouse click event
 */

template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::mouseMoveEvent(QMouseEvent *e)
  {
    this->setFocus();
    double xcoord, ycoord;
    this->x_real(e->x(), xcoord);
    this->y_real(e->y(), ycoord);
    QString x_y("x= ");
    QString temp;
    temp = temp.setNum(xcoord, 'g', 5);
    x_y += temp;

    temp = QString(" y=");
    x_y += temp;
    temp = temp.setNum(ycoord, 'g', 5);
    x_y += temp;
    temp = QString(" ");
    x_y += temp;

    QWidget* qw = qApp->mainWidget();
    QMainWindow* my_window = static_cast<QMainWindow*> (qw);
    my_window->statusBar()->message(x_y);

    if (mode == MODE_DELETE)
      {
        find_removable_halfedges(e);
      }

    if (active) //case for split action //for insert too
      {
        Coord_type x, y;
        x_real(e->x(), x);
        y_real(e->y(), y);
        Coord_point p = point(x, y);
        RasterOp old_raster = rasterOp();//save the initial raster mode
        setRasterOp(XorROP);
        lock();

        setColor(Qt::green);

        if (!first_time)
          m_tab_traits.draw_last_segment(this);

        m_tab_traits.draw_current_segment(p, this);

        unlock();
        setRasterOp(old_raster);
        first_time = false;
      }
  }

/*! leaveEvent - hide the line if you leave the widget's area on the screen
 *\ param e - mouse click event
 */
template<class TabTraits>
  void
  Qt_widget_demo_tab<TabTraits>::leaveEvent(QEvent * /* e */)
  {
    if (active)
      {
        RasterOp old_raster = rasterOp();//save the initial raster mode
        QColor old_color = color();
        lock();
        setRasterOp(XorROP);
        setColor(Qt::green);
        m_tab_traits.draw_last_segment(this);
        setRasterOp(old_raster);
        setColor(old_color);
        unlock();
        first_time = true;
      }
  }

/*! point
 *\ params x,y - the mouse clicked point coordinates
 *\    return a point according to the current snap mode and
 *  recent points.
 */
template<class TabTraits>
  Coord_point
  Qt_widget_demo_tab<TabTraits>::point(Coord_type x, Coord_type y)
  {
    return Coord_point(x, y);
  }

template<class TabTraits>
  bool
  Qt_widget_demo_tab<TabTraits>::is_empty()
  {
    return (!point_x_set && xcurves.empty());
  }

template
Qt_widget_demo_tab<Segment_tab_traits>::Qt_widget_demo_tab(QWidget * parent,
    int tab_number);
template void
Qt_widget_demo_tab<Segment_tab_traits>::draw();
template bool
Qt_widget_demo_tab<Segment_tab_traits>::is_empty();
template bool
Qt_widget_demo_tab<Segment_tab_traits>::check_for_degen();
template void
Qt_widget_demo_tab<Segment_tab_traits>::set_point_x(Point_2 p);

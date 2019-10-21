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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/MyWindow_operations.cpp $
// $Id: MyWindow_operations.cpp 45453 2008-09-09 21:35:22Z lrineau $
//
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#include "MyWindow.h"

//get the demo tab
Qt_widget_demo_tab<Segment_tab_traits>*
MyWindow::demo_tab()
{
  Qt_widget_demo_tab<Segment_tab_traits>
      *w_demo_p =
          static_cast<Qt_widget_demo_tab<Segment_tab_traits> *> (myBar->currentPage());
  return w_demo_p;
}

//get the base tab
Qt_widget_base_tab*
MyWindow::base_tab()
{
  Qt_widget_base_tab *w_base_p =
      static_cast<Qt_widget_base_tab*> (myBar->currentPage());
  return w_base_p;
}

/*! get_new_object - get a point object from current page
 * \param obj - a CGAL object
 */
void
MyWindow::get_new_object(CGAL::Object obj)
{
  typedef Traits_2::Point_2 Point_2;

  // point location
  Coord_point p;

  try
    {
      p = CGAL::object_cast<Coord_point>(obj);
      Point_2 pp = Point_2(p.x(),p.y());
      demo_tab()->set_point_x(pp);

    }
  catch (CGAL::Bad_object_cast)
    {
      std::cout << "exception in get_new_object" << std::endl;
    }
  base_tab()->something_real_changed();
}

/*! about - message box about the demo */
void
MyWindow::about()
{
  QMessageBox::about(this, "About",
      "This is a demo for the Arrangement package\n"
        "Copyright CGAL @2003");
}

/*! aboutQt - message box about Qt */
void
MyWindow::aboutQt()
{
  QMessageBox::aboutQt(this, "About Qt");
}

/*! howto - help menu */
void
MyWindow::howto()
{
  QString home;
  home = "help/index.html";
  CGAL::Qt_help_window * help = new CGAL::Qt_help_window(home, ".", 0,
      "help viewer");
  help->resize(400, 400);
  help->setCaption("Demo HowTo");
  help->show();
}

/*! redraw the widget when timer ends */
void
MyWindow::timer_done()
{
  if (base_tab()->old_state != base_tab()->current_state)
    {
      base_tab()->redraw();
      base_tab()->old_state = base_tab()->current_state;
    }
}

/*! zoom in - enlarge the picture */
void
MyWindow::zooleft()
{
  base_tab()->zoom(m_scailing_factor);
}

/*! zoom out - lessen the picture */
void
MyWindow::zoomout()
{
  base_tab()->zoom(1 / m_scailing_factor);
}

// fit to screen
void
MyWindow::fit()
{
  base_tab()->set_window(base_tab()->bbox.xmin(), base_tab()->bbox.xmax(),
      base_tab()->bbox.ymin(), base_tab()->bbox.ymax());
}

// check for degeneracies
void
MyWindow::check()
{
  bool test = demo_tab()->check_for_degen();
}

/*! initialize the widget */
void
MyWindow::init(Qt_widget_base_tab *widget)
{
  CGAL::set_error_behaviour(CGAL::ABORT);
  connect(widget, SIGNAL(new_cgal_object(CGAL::Object)),
  this, SLOT(get_new_object(CGAL::Object)));
  widget->attach(testlayer);
  widget->setCursor(QCursor(QPixmap((const char**) small_draw_xpm)));
  widget->setFocusPolicy(QWidget::StrongFocus);
  widget->setBackgroundColor(Qt::white);
  tab_number++;
  number_of_tabs++;
  myBar->insertTab(widget,
      QString("Tab " + QString::number(widget->index + 1)), widget->index);
  myBar->setCurrentPage(myBar->indexOf(widget));
  update();
  base_tab()->something_changed();
}

/*! add a tab widget with segment traits */
void
MyWindow::add_segment_tab()
{
  Qt_widget_demo_tab<Segment_tab_traits> *widget = new Qt_widget_demo_tab<
      Segment_tab_traits> (this, tab_number);
  init(widget);
}

/*! remove the current page (tab) from myBar */
void
MyWindow::remove_tab()
{
  if (number_of_tabs > 1)
    {
      QWidget *w_demo_p = myBar->currentPage(); // w_demo_p is a pointer to Qt_widget_demo_tab object
      myBar->removePage(w_demo_p);
      delete w_demo_p; //the destructor of Qt_widget_demo_tab will be called (virtual...)
      number_of_tabs--;
    }
  else
    {
      QMessageBox::information(this, "Remove Tab", "Can not remove last tab");
    }
}

/*! update widget mode
 * \param action - the new widget mode
 */
void
MyWindow::updateMode(QAction *action)
{
  demo_tab()->removable_curve_set = false;

  if (action == insertMode)
    {
      base_tab()->mode = MODE_INSERT;
      base_tab()->setCursor(QCursor(QPixmap((const char**) small_draw_xpm)));
    }
  else if (action == deleteMode)
    {
      base_tab()->mode = MODE_DELETE;
      base_tab()->setCursor(QCursor(QPixmap((const char**) delete_xpm)));
    }
  else if (action == pointLocationMode)
    {
      base_tab()->mode = MODE_POINT_LOCATION;
      base_tab()->setCursor(Qt::CrossCursor);
    }
  base_tab()->something_changed();
}

/*! set the buttons state according to the current mode */
void
MyWindow::setMode(Mode m)
{
  base_tab()->mode = m;
  switch (m)
    {
  case MODE_INSERT:
    insertMode->setOn(TRUE);
    break;
  case MODE_DELETE:
    deleteMode->setOn(TRUE);
    break;
  case MODE_POINT_LOCATION:
    pointLocationMode->setOn(TRUE);
    break;
    }
}

/*! update all modes */
void
MyWindow::update()
{
  setMode(base_tab()->mode);
}

// create random segments in a square
void
MyWindow::random_in_square()
{
  typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;
  typedef Traits_2::Point_2 Point_2;

  std::vector<Coord_segment> segs;
  std::vector<Coord_segment>::iterator i;
  std::list<X_monotone_curve_2> seg_list;
  typedef CGAL::Creator_uniform_2<double, Coord_point> Coord_point_creator;

  bool ok;

  //get number of segments from the user
  int n = QInputDialog::getInteger("RedBlueMerge", "number of segments:", 0, 0,
      1000000, 1, &ok, this);

  if (ok)
    {
      QCursor old = base_tab()->cursor();
      base_tab()->setCursor(Qt::WaitCursor);

      demo_tab()->xcurves.clear();
      demo_tab()->point_x_set = false;

      // Prepare point generator for random points in square
      // with side length n*100
      typedef CGAL::Random_points_in_square_2<Coord_point, Coord_point_creator>
          P2;
      P2 p1(n * 100);
      P2 p2(n * 100);

      typedef CGAL::Creator_uniform_2<Coord_point, Coord_segment> Seg_creator;
      typedef CGAL::Join_input_iterator_2<P2, P2, Seg_creator> Seg_iterator;

      //produce segments
      Seg_iterator g(p1, p2);
      CGAL::copy_n(g, n, std::back_inserter(segs));

      //convert segments and insert in list
      for (i = segs.begin(); i != segs.end(); i++)
        {
          NT x1 = i->source().x();
          NT y1 = i->source().y();
          NT x2 = i->target().x();
          NT y2 = i->target().y();
          Point_2 p1(x1, y1);
          Point_2 p2(x2, y2);
          X_monotone_curve_2 curve(p1, p2);
          CGAL::Bbox_2 curve_bbox = curve.bbox();
          if (i == segs.begin())
            base_tab()->bbox = curve_bbox;
          else
            base_tab()->bbox = base_tab()->bbox + curve_bbox;
          seg_list.push_back(curve);
        }

      demo_tab()->xcurves = seg_list;

      base_tab()->set_window(base_tab()->bbox.xmin(), base_tab()->bbox.xmax(),
          base_tab()->bbox.ymin(), base_tab()->bbox.ymax());

      base_tab()->setCursor(old);
      base_tab()->something_real_changed();
    }
}

void
MyWindow::grid()
{
  bool ok;

  //get the number of vertical segments from the user
  int n = QInputDialog::getInteger("RedBlueMerge",
      "number of vertical segments:", 0, 0, 1000000, 1, &ok, this);

  if (ok)
    {
      QCursor old = base_tab()->cursor();
      base_tab()->setCursor(Qt::WaitCursor);

      demo_tab()->xcurves.clear();
      demo_tab()->point_x_set = false;

      typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;
      typedef Traits_2::Point_2 Point_2;
      std::vector<X_monotone_curve_2> grid_segs;
      std::vector<X_monotone_curve_2>::iterator i;
      std::list<X_monotone_curve_2> seg_list;

      typedef CGAL::Creator_uniform_2<double, Coord_point> Coord_point_creator;
      typedef CGAL::Random_points_on_segment_2<Coord_point, Coord_point_creator>
          Random_P1;

      Coord_point left_bottom(0, 0);
      Coord_point left_top(0, n);
      Coord_point right_bottom(n, 0);
      Random_P1 vertical(left_bottom, left_top);
      Random_P1 horizontal(left_bottom, right_bottom);
      std::vector<Coord_point> horizontal_points;
      std::vector<Coord_point> vertical_points;

      CGAL::copy_n(vertical, n, std::back_inserter(vertical_points));
      CGAL::copy_n(horizontal, n, std::back_inserter(horizontal_points));

      for (int j = 0; j < n; j++)
        {
          Point_2 bottom(horizontal_points[j].x(),left_bottom.y());
          Point_2 top(horizontal_points[j].x(), left_top.y());
          grid_segs.push_back(X_monotone_curve_2(bottom, top));
        }

      for (int j = 0; j < n; j++)
        {
          Point_2 left(left_bottom.x(),vertical_points[j].y());
          Point_2 right(right_bottom.x(), vertical_points[j].y());
          grid_segs.push_back(X_monotone_curve_2(left, right));
        }

//            get a regular grid
//            for (int j = 0; j < n; j++)
//              {
//                Point_2 bottom(j + 1, 1);
//                Point_2 top(j + 1, n + 2);
//                grid_segs.push_back(X_monotone_curve_2(bottom, top));
//              }
//
//            for (int j = 0; j < n; j++)
//              {
//                Point_2 left(0, j + 2);
//                Point_2 right(n + 1, j + 2);
//                grid_segs.push_back(X_monotone_curve_2(left, right));
//              }

      //shuffle the segments, so that horizontal and vertical segments
      //are mixed in the red and blue sets
      std::random_shuffle(grid_segs.begin(), grid_segs.end());

      //insert in list and adapt bbox
      for (i = grid_segs.begin(); i != grid_segs.end(); i++)
        {
          CGAL::Bbox_2 curve_bbox = i->bbox();
          if (i == grid_segs.begin())
            base_tab()->bbox = curve_bbox;
          else
            base_tab()->bbox = base_tab()->bbox + curve_bbox;
          seg_list.push_back(*i);
        }

      demo_tab()->xcurves = seg_list;
      base_tab()->set_window(base_tab()->bbox.xmin(), base_tab()->bbox.xmax(),
          base_tab()->bbox.ymin(), base_tab()->bbox.ymax());
      base_tab()->setCursor(old);
      base_tab()->something_real_changed();
    }
}

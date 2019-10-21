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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/arrangement_2.h $
// $Id: arrangement_2.h 41314 2007-12-24 16:43:18Z guyzucke $
//
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef CGAL_MYWINDOW_H
#define CGAL_MYWINDOW_H
/////////////////////////////////////////////////////////////////////////////////////////
// the demo program includs several calsses:
// 1. MyWindow - main window, create thw window properties (tool bar, menu bar)
// 2. Qt_widget_demo_tab - the program give the user an optoin of multiple tabs with
//    different curve traits (segment_tab, polyline_tab and conic_tab)
// 3. Qt_layer - the screen object attached to every demo_tab that draw on it.
// 4. forms classes - the dailogs windows.
/////////////////////////////////////////////////////////////////////////////////////////

#include <qaction.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtimer.h>
#include <qlabel.h>
#include "icons/hand.xpm"
#include "icons/draw.xpm"

#include "icons/demo_delete.xpm"
#include "icons/demo_insert.xpm"
#include "icons/demo_split.xpm"
#include "icons/demo_zoomout.xpm"
#include "icons/demo_zoomin.xpm"
#include "icons/demo_pointlocation.xpm"
#include "icons/demo_conic_circle.xpm"
#include "icons/demo_rayshoot_down.xpm"
#include "icons/demo_rayshoot_up.xpm"
#include "icons/lower_env_xpm.xpm"
#include "icons/upper_env_xpm.xpm"
#include "icons/demo_snapgrid.xpm"
#include "icons/vertices.xpm"
#include "icons/demo_snapvertex.xpm"

#include "qt_layer.h"

class Qt_layer;
class Qt_widget_base_tab;

/*! class MyWindow is the main class that controls all the window
 operations
 */
class MyWindow : public QMainWindow
{
Q_OBJECT
public:

  /*! constructor */
  MyWindow(int w, int h);
  /*! distructor */
  ~MyWindow();

  //get the base tab
  Qt_widget_base_tab*
  base_tab();
  //get the demo tab
  Qt_widget_demo_tab<Segment_tab_traits>*
  demo_tab();

private:
  /*! read from file */
  void
  load(const QString& filename, bool clear_flag = false);
  /*! initialize widget */
  void
  init(Qt_widget_base_tab *widget);

  /*private*/
public slots:
  /*! get_new_object - connects between the widget and main window */
  void
  get_new_object(CGAL::Object obj);
  /*! open an information dialog*/
  /*! open an information dialog*/
  void about();
  /*! open an information dialog*/
  void aboutQt();
  void howto();
  /*! add a new tab of segment traits */
  void
  add_segment_tab();
  /*! remove current tab */
  void
  remove_tab();
  /*! connect the timer to main window */
  void
  timer_done();
  /*! change the mode of current tab */
  void
  updateMode(QAction *action);
  /*! update the window buttons according to change in mode */
  void
  setMode(Mode m);
  /*! update all the window buttons */
  void
  update();
  /*! zoom in the picture */
  void
  zooleft();
  /*! zoom out the picture */
  void
  zoomout();
  // fit to screen
  void
  fit();
  //check for degeneracies
  void
  check();
  /*! open a file */
  void
  fileOpen(bool clear_flag = false);
  /*! open a file and add a segment tab */
  void
  fileOpenSegment();
  /*! save file */
  void
  fileSave();
  /*! open a save file dialog box */
  void
  fileSaveAs();
  //produce random segments
  void
  random_in_square();
  //get a grid
  void
  grid();

private:
  /*! myBar - hold the tab widgets */
  QTabWidget *myBar;

private:
  /*! the index number of the next tab in the window */
  int tab_number;
  /*! number of tabs in the window */
  int number_of_tabs;
  /*! testlayer attached to all widget tabs and show them when needed */
  Qt_layer *testlayer;

  /*! insert mode action */
  QAction *insertMode;
  /*! delete mode action */
  QAction *deleteMode;
  /*! point location mode action */
  QAction *pointLocationMode;

  /*! zoomin button */
  QAction *zoominBt;
  /*! zoomout button */
  QAction *zoomoutBt;
  QAction *FitBt; //fit button

  QAction *degenBt; //check for degeneracies button

  QAction *gridBt; //get grid button
  QAction *randomSquareBt; //random segments button

  /*! the name of the file to be saved */
  QString m_filename;
  /*! window hight */
  int m_height;
  /*! window width */
  int m_width;
  /*! scailing factor */
  double m_scailing_factor;
};

#endif // MYWINDOW_H

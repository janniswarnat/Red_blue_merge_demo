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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/arrangement_2.cpp $
// $Id: arrangement_2.cpp 45453 2008-09-09 21:35:22Z lrineau $
//
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#include "MyWindow.h"

//////////////////////////////////////////////////////////////////////////////
/*! MyWindow constructor
 * \param w - window width
 * \param h - window hight
 */
MyWindow::MyWindow(int w, int h)
{
  myBar = new QTabWidget(this);
  setCentralWidget(myBar);
  m_width = w;
  m_height = h;
  tab_number = 0;
  number_of_tabs = 0;
  statusBar();
  testlayer = new Qt_layer(myBar);
  m_scailing_factor = 2;

  QActionGroup *modeGroup = new QActionGroup(this); // Connected later
  modeGroup->setExclusive(TRUE);

  //insert segments
  insertMode = new QAction("Insert", QPixmap((const char**) insert_xpm),
      "&Insert", 0, modeGroup, "Insert");
  insertMode->setToggleAction(TRUE);

  //delete segments
  deleteMode = new QAction("Delete", QPixmap((const char**) delete_xpm),
      "&Delete", 0, modeGroup, "Delete");
  deleteMode->setToggleAction(TRUE);

  //insert point_x
  pointLocationMode = new QAction("Set Point", QPixmap(
      (const char**) pointlocation_xpm), "&Set Point", 0, modeGroup,
      "Set Point");
  pointLocationMode->setToggleAction(TRUE);

  // zoom in
  zoominBt = new QAction("Zoom in", QPixmap((const char**) zoomin_xpm),
      "&Zoom in", 0, this, "Zoom in");
  // zoom out
  zoomoutBt = new QAction("Zoom out", QPixmap((const char**) zoomout_xpm),
      "&Zoom out", 0, this, "Zoom out");

  // fit to screen
  FitBt = new QAction("Fit to screen", QPixmap((const char**) split_xpm), "&Fit to screen", 0,
      this, "Fit to screen");

  // test for degeneracies
  degenBt = new QAction("Degenerate", QPixmap((const char**) snapvertex_xpm), "&Degenerate", 0,
      this, "Degenerate");

  // get a grid
  gridBt = new QAction("Grid", QPixmap((const char**) snapgrid_xpm), "&Grid", 0,
      this, "Grid");

  // produce random segments
  randomSquareBt = new QAction("Random", QPixmap((const char**) vertices_icon), "&Random", 0,
      this, "Random");

  //create a timer for checking if somthing changed
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()),
  this, SLOT(timer_done()) );
  timer->start(200, FALSE);

  // file menu
  QPopupMenu * file = new QPopupMenu(this);
  menuBar()->insertItem("&File", file);
  file->insertItem("&Open Segment File...", this, SLOT(fileOpenSegment()));
  file->insertItem("&Save...", this, SLOT(fileSave()));
  file->insertItem("&Save As...", this, SLOT(fileSaveAs()));
  file->insertItem("&Quit", qApp, SLOT( closeAllWindows() ), CTRL + Key_Q);
  menuBar()->insertSeparator();

  // tab menu
  QPopupMenu * tab = new QPopupMenu(this);
  menuBar()->insertItem("&Tab", tab);
  tab->insertItem("Add &Tab", this, SLOT(add_segment_tab()));
  tab->insertSeparator();
  tab->insertItem("Remove &Tab", this, SLOT(remove_tab()));
  menuBar()->insertSeparator();

  // help menu
  QPopupMenu * help = new QPopupMenu(this);
  menuBar()->insertItem("&Help", help);
  help->insertItem("How To...", this, SLOT(howto()));//, Key_F1);
  help->insertSeparator();
  help->insertItem("&About...", this, SLOT(about()), CTRL+Key_A );
  help->insertItem("About &Qt...", this, SLOT(aboutQt()) );

  QToolBar *modeTools = new QToolBar(this, "mode operations");
  modeTools->setLabel("Mode Operations");
  insertMode->addTo(modeTools);
  deleteMode->addTo(modeTools);
  pointLocationMode->addTo(modeTools);

  QToolBar *zoomTool = new QToolBar(this, "zoom");
  zoomTool->setLabel("Zoom");
  zoomoutBt->addTo(zoomTool);
  zoominBt->addTo(zoomTool);
  FitBt->addTo(zoomTool);

  QToolBar *randomTool = new QToolBar(this, "random");
  randomTool->setLabel("Random");
  degenBt->addTo(randomTool);
  gridBt->addTo(randomTool);
  randomSquareBt->addTo(randomTool);

  connect(zoomoutBt, SIGNAL( activated () ) ,
  this, SLOT( zoomout() ) );

  connect(zoominBt, SIGNAL( activated () ) ,
  this, SLOT( zooleft() ) );

  connect(FitBt, SIGNAL( activated () ) ,
  this, SLOT( fit() ) );

  connect(degenBt, SIGNAL( activated () ) ,
  this, SLOT( check() ) );

  connect(gridBt, SIGNAL( activated () ) ,
  this, SLOT( grid() ) );

  connect(randomSquareBt, SIGNAL( activated () ) ,
  this, SLOT( random_in_square() ) );

  connect(modeGroup, SIGNAL( selected(QAction*) ),
  this, SLOT( updateMode(QAction*) ) );

  // connect the change of current tab
  connect(myBar, SIGNAL( currentChanged(QWidget * ) ),
  this, SLOT( update() ) );

  add_segment_tab();
  resize(m_width, m_height);
  setUsesTextLabel(true);
}

/*! destructor */
MyWindow::~MyWindow()
{
}

#include "MyWindow.moc"

/*! main */
int
main(int argc, char **argv)
{
  const QString my_title_string("Red blue merge demo");
  QApplication app(argc, argv);
  MyWindow widget(1024, 768);
  app.setMainWidget(&widget);
  widget.setCaption(my_title_string);
  widget.setMouseTracking(TRUE);
  widget.show();
  return app.exec();
}

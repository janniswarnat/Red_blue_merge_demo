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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/MyWindow_files.cpp $
// $Id: MyWindow_files.cpp 45453 2008-09-09 21:35:22Z lrineau $
//
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#include "MyWindow.h"

/*! open a segment file and add new tab */
void
MyWindow::fileOpenSegment()
{
  if (demo_tab()->is_empty())
    {
      fileOpen(true);
    }
  else
    {
      add_segment_tab();
      fileOpen();
    }
}// fileOpenSegment


/*! open a file */
void
MyWindow::fileOpen(bool clear_flag)
{

  QString filename = QFileDialog::getOpenFileName(QString::null, 0, this,
      "file open", "Demo -- File Open");
  if (!filename.isEmpty())
    {
      load(filename, clear_flag);
    }
  else
    {
      statusBar()->message("File Open abandoned", 2000);
    }
}

/*! load from file
 * \param filename - name of the file
 */
void
MyWindow::load(const QString& filename, bool clear_flag)
{
  std::cout << "load() begin" << std::endl;
  typedef Traits_2::Point_2 Point_2;
  typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;

  std::ifstream inputFile(filename);
  // Creates an ofstream object named inputFile
  if (!inputFile.is_open()) // Always test file open
    {
      return;
    }

  QCursor old = base_tab()->cursor();
  base_tab()->setCursor(Qt::WaitCursor);

  if (clear_flag)
    {
      demo_tab()->xcurves.clear();
      demo_tab()->point_x_set = false;
    }

  //number of segments
  int count;
  inputFile >> count;
  Point_2 point_x;
  bool set_point_x = false;

  //file contains a point_x, read in
  if (count == 0)
    {
      NT px0, py0;
      inputFile >> px0 >> py0;
      set_point_x = true;
      point_x = Point_2(px0, py0);
      inputFile >> count;
    }

  int i;
  std::list<X_monotone_curve_2> seg_list;

  //read the segments and put them in the list
  for (i = 0; i < count; i++)
    {
      NT x0, y0, x1, y1;
      inputFile >> x0 >> y0 >> x1 >> y1;

      Point_2 p1(x0, y0);
      Point_2 p2(x1, y1);
      X_monotone_curve_2 curve(p1, p2);

      CGAL::Bbox_2 curve_bbox = curve.bbox();
      if (i == 0)
        {
        base_tab()->bbox = curve_bbox;
        }
      else
        {
        base_tab()->bbox = base_tab()->bbox + curve_bbox;
        }
      seg_list.push_back(curve);
    }

  demo_tab()->xcurves = seg_list;

  base_tab()->set_window(base_tab()->bbox.xmin(), base_tab()->bbox.xmax(),
      base_tab()->bbox.ymin(), base_tab()->bbox.ymax());

  if(set_point_x)
    {
      demo_tab()->set_point_x(point_x);
    }

  inputFile.close();
  base_tab()->setCursor(old);
  base_tab()->something_real_changed();
  std::cout << "load() end" << std::endl;
}

/*! save file in a different name */
void
MyWindow::fileSaveAs()
{
  QString filename = QFileDialog::getSaveFileName(QString::null, 0, this,
      "file save as", "Segments -- File Save As");
  if (!filename.isEmpty())
    {
      int answer = 0;
      if (QFile::exists(filename))
        answer = QMessageBox::warning(this, "Overwrite File", QString(
            "Overwrite\n\'%1\'?"). arg(filename), "&Yes", "&No", QString::null,
            1, 1);
      if (answer == 0)
        {
          m_filename = filename;
          fileSave();
          return;
        }
    }
  statusBar()->message("Saving abandoned", 2000);
}

/*! save file */
void
MyWindow::fileSave()
{
  typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;

  if (m_filename.isEmpty())
    {
      fileSaveAs();
      return;
    }

  std::ofstream outFile(m_filename);
  // Creates an ofstream object named outFile
  if (!outFile.is_open()) // Always test file open
    {
      return;
    }

  //first write the number of segments
  outFile << demo_tab()->xcurves.size();
  outFile << "\n";

  //write the segments
  for (std::list<X_monotone_curve_2>::iterator ci = demo_tab()->xcurves.begin(); ci
      != demo_tab()->xcurves.end(); ci++)
    {
      outFile << *ci << "\n";
    }

  outFile.close();

  setCaption(QString("Arrangement -- %1").arg(m_filename));
  statusBar()->message(QString("Saved \'%1\'").arg(m_filename), 2000);
}


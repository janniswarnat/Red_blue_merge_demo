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

#include "demo_tab.h"

/*! class Qt_widget_base_tab - inherits from CGAL::Qt_widget
 *  contain all data members that are not part of the traits
 */

Qt_widget_base_tab::Qt_widget_base_tab(QWidget *parent,
    int tab_number) :
      CGAL::Qt_widget(parent),
      current_state(0),
      real_change(false),
      old_state(-1),
      index(tab_number),
      mode(MODE_INSERT),
      first_time(true), active(false), bbox(
          CGAL::Bbox_2(-10, -10, 10, 10))
{
  static_cast<CGAL::Qt_widget&> (*this) << CGAL::LineWidth(2)
      << CGAL::BackgroundColor(CGAL::BLACK);
  set_window(-10, 10, -10, 10);
  setMouseTracking(TRUE);
}

/*! Destructor */
Qt_widget_base_tab::~Qt_widget_base_tab()
{
}

/*! increment current_state to indicate that something has changed
 */
void
Qt_widget_base_tab::something_changed()
{
  current_state++;
}

//geometric change, so recomputation is necessary
void
Qt_widget_base_tab::something_real_changed()
{
  something_changed();
  real_change = true;
}



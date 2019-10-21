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

#ifndef BASE_TAB_H
#define BASE_TAB_H

#include "cgal_types.h"
#include <qtabwidget.h>
#include <qcursor.h>

#include <CGAL/IO/pixmaps/hand.xpm>
#include <CGAL/IO/pixmaps/holddown.xpm>

/*! class Qt_widget_base_tab - inherits from CGAL::Qt_widget
 *  contain all data members that are not part of the traits
 */

class Qt_widget_base_tab : public CGAL::Qt_widget
{
public:

  double decomp_time;
  double sweep_time;
  double rotate_time;
  double fuse_time;

  /*! Constructor */
  Qt_widget_base_tab(QWidget *parent, int tab_number);

  /*! Destructor */
  ~Qt_widget_base_tab();

  /*! current_state - indicates when a tab state is changed */
  int current_state;

  /*! old state of current tab */
  int old_state;

  // there was geometric change, so recomputation is necessary
  bool real_change;

  /*! index - each tab has a unique index */
  int index;

  /*! mode - current mode (insert,delete or point location) */
  Mode mode;

  /*! first_time - true when it is the first mouse click of the object */
  bool first_time;

  /*! active - true if the first point was inserted */
  bool active;

  /*! bbox - bounding box */
  CGAL::Bbox_2 bbox;

  /*! increment current_state to indicate that something has changed
   */
  void something_changed();

  //geometric change, so recomputation is necessary
  void something_real_changed();
};


#endif

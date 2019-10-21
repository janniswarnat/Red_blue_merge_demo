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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Arrangement_on_surface_2/demo/Arrangement_on_surface_2/cgal_types.h $
// $Id: cgal_types.h 40214 2007-09-05 15:46:47Z efif $
//
//
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>

#ifndef CGAL_TYPES_HEADER_H
#define CGAL_TYPES_HEADER_H

#include <CGAL/basic.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/IO/Qt_widget.h>
#include <CGAL/IO/Qt_help_window.h>

#include <CGAL/Cartesian.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_segment_traits_2.h>

#include <CGAL/Arrangement_2.h>

#include <CGAL/assertions_behaviour.h>

#include <CGAL/Memory_sizer.h>

#include "arr_print.h"

#include <qapplication.h>
#include <qmainwindow.h>
#include <qcolor.h>

#include <sstream>

#include <CGAL/point_generators_2.h>

enum Mode
{
  MODE_INSERT, MODE_DELETE, MODE_POINT_LOCATION
};

enum Vertex_type
{
  //none of below
  DEFAULT = 0,

  //vertex represents point_x
  POINT_X = 1,

  //vertex represents an original segment's endpoint
  INTERNAL_ENDPOINT = 2,

  //vertex represents an original segment's endpoint
  //in the arrangement of the other color
  EXTERNAL_ENDPOINT = 4
};

// Coordinate related typedef - using inexact number type
typedef double Coord_type;
typedef CGAL::Cartesian<Coord_type> Coord_kernel;
typedef Coord_kernel::Point_2 Coord_point;
typedef Coord_kernel::Segment_2 Coord_segment;

#define MY_CGAL_DC_PRINT(a)
//#define MY_CGAL_DC_PRINT(a) { std::cout << a << std::endl ; }

#define MY_CGAL_SL_PRINT(a)
//#define MY_CGAL_SL_PRINT(a) { std::cout << a << std::endl ; }

#define MY_CGAL_SL_DEBUG(a)
//#define MY_CGAL_SL_DEBUG(a) {a}


#ifdef CGAL_USE_GMP

#include <CGAL/Gmpq.h>

//typedef CGAL::Gmpq NT;

#else

#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>

//typedef CGAL::Quotient<CGAL::MP_Float> NT;

#endif

class Vertex_data;

//the kernel
//typedef CGAL::Cartesian<NT> Kernel;
typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::FT NT;

//the traits class
typedef CGAL::Arr_segment_traits_2<Kernel> Traits_2;

// we use an extended dcel to store data with the vertices and the edes;
// we don´t store meaningful date with the faces
typedef CGAL::Arr_extended_dcel<Traits_2, Vertex_data, bool, bool> Dcel;

//the arrangement
typedef CGAL::Arrangement_2<Traits_2, Dcel> Arrangement_2;

// With an arrangement vertex we store the Vertex_type;
// furthermore it is sometimes helpful  to store a
// Vertex_handle to a vertex representing the same
// geometric point in another arrangement

class Vertex_data
{
  typedef Arrangement_2::Vertex_handle Vertex_handle;

private:

  Vertex_type type_;
  Vertex_handle v_handle;
  Vertex_handle invalid_v;

  public:

    Vertex_data (Vertex_data const & a)// Copy constructor
    {
      type_ = a.type();
      v_handle = invalid_v;
    }

    Vertex_data & operator = (Vertex_data const & a) // Assignment operator
    {
      this->set_type(a.type());
      this->unset_vertex_handle();
      return *this;
    }

    Vertex_data ()
    {
      type_ = DEFAULT;
      v_handle = invalid_v;
    }

  Vertex_data (Vertex_type t, Vertex_handle vh)
  {
    type_ = t;
    v_handle = vh;
  }

  Vertex_data (Vertex_type t)
  {
    type_ = t;
    v_handle = invalid_v;
  }

  void set_type(Vertex_type t)
  {
    type_ = t;
  }

  Vertex_type type() const
  {
    return type_;
  }

  void set_vertex_handle(Vertex_handle vh)
  {
    v_handle = vh;
  }

  void unset_vertex_handle()
  {
    v_handle = invalid_v;
  }

  bool has_vertex_handle()
  {
    return (v_handle != invalid_v);
  }

  Vertex_handle vertex_handle()
  {
    return v_handle;
  }
 };

#endif

#ifndef _PRINT_ARR_H_
#define _PRINT_ARR_H_

template<class Arrangement>
void
print_vertex(typename Arrangement::Vertex_const_handle vit)
{
  std::cout << "(" << CGAL::to_double(vit->point().x()) << " " << CGAL::to_double(
      vit->point().y()) << ")";
  std::cout << " data().type() = " << vit->data().type() << std::endl;
  if (vit->is_isolated())
    std::cout << " - Isolated." << std::endl;
  else
    std::cout << " - degree " << vit->degree() << std::endl;
}

template<class Arrangement>
void
print_halfedge(typename Arrangement::Halfedge_const_handle eit)
{
  std::cout << "[ (" << CGAL::to_double(eit->source()->point().x()) << " "
      << CGAL::to_double(eit->source()->point().y()) << ")(" << CGAL::to_double(
      eit->target()->point().x()) << " " << CGAL::to_double(
      eit->target()->point().y()) << ") ] << eit->data() = " << eit->data() << " eit->twin()->data() = " << eit->twin()->data() << std::endl;
}

//-----------------------------------------------------------------------------
// Print all vertices (points) and edges (curves) along a connected component
// boundary.
//
template<class Arrangement>
  void
  print_ccb(typename Arrangement::Ccb_halfedge_const_circulator circ)
  {
    typename Arrangement::Ccb_halfedge_const_circulator curr = circ;
    typename Arrangement::Halfedge_const_handle he;

    std::cout << "(" << CGAL::to_double(curr->source()->point().x()) << " "
        << CGAL::to_double(curr->source()->point().y()) << ")";

    do
      {
        he = curr;

        std::cout << "   [ (" << CGAL::to_double(he->source()->point().x()) << " "
            << CGAL::to_double(he->source()->point().y()) << ")(" << CGAL::to_double(
            he->target()->point().x()) << " " << CGAL::to_double(
            he->target()->point().y()) << ")]   " << "(" << CGAL::to_double(
            he->target()->point().x()) << " " << CGAL::to_double(
            he->target()->point().y()) << ")";

        ++curr;
      }
    while (curr != circ);
    std::cout << std::endl;

    return;
  }

//-----------------------------------------------------------------------------
// Print the boundary description of an arrangement face.
//

template<class Arrangement>
  void
  print_face(typename Arrangement::Face_const_handle f)
  {
    // Print the outer boundary.
    if (f->is_unbounded())
      {
        std::cout << "Unbounded face. " << std::endl;
      }
    else
      {
        std::cout << "Outer boundary: ";
        print_ccb<Arrangement> (f->outer_ccb());
      }

    // Print the boundary of each of the holes.
    typename Arrangement::Hole_const_iterator hole;
    int index = 1;

    for (hole = f->holes_begin(); hole != f->holes_end(); ++hole, ++index)
      {
        std::cout << "    Hole #" << index << ": ";
        print_ccb<Arrangement> (*hole);
      }

    // Print the isolated vertices.
    typename Arrangement::Isolated_vertex_const_iterator iv;

    for (iv = f->isolated_vertices_begin(), index = 1; iv
        != f->isolated_vertices_end(); ++iv, ++index)
      {
        std::cout << "    Isolated vertex #" << index << " " << &(*iv) << ": "
            << "(" << CGAL::to_double(iv->point().x()) << " " << CGAL::to_double(
            iv->point().y()) << ") data().type() = " << iv->data().type() << std::endl;
      }

    return;
  }

//-----------------------------------------------------------------------------
// Print the given arrangement.
//
template<class Arrangement>
  void
  print_arrangement(const Arrangement& arr)
  {

    std::cout << "begin print_arrangement()" << std::endl;
    CGAL_precondition(arr.is_valid());

    // Print the arrangement vertices.
    typename Arrangement::Vertex_const_iterator vit;

    std::cout << arr.number_of_vertices() << " vertices:" << std::endl;
    for (vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit)
      {
        print_vertex<Arrangement>(vit);
      }

    // Print the arrangement edges.
    typename Arrangement::Edge_const_iterator eit;

    std::cout << arr.number_of_edges() << " edges:" << std::endl;
    for (eit = arr.edges_begin(); eit != arr.edges_end(); ++eit)
      {
        print_halfedge<Arrangement>(eit);
      }
    typename Arrangement::Face_const_iterator fit;

    std::cout << arr.number_of_faces() << " faces:" << std::endl;
    for (fit = arr.faces_begin(); fit != arr.faces_end(); ++fit)
      {
      print_face<Arrangement> (fit);
      }

    return;
  }

#endif

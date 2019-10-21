#ifndef RED_BLUE_DIVIDE_AND_CONQUER_H
#define RED_BLUE_DIVIDE_AND_CONQUER_H

#include <CGAL/Aff_transformation_2.h>
#include "My_Sweep_line_2.h"
#include "My_Arr_overlay_traits_2.h"
#include <CGAL/Arr_vertical_decomposition_2.h>

/*! \file
 * free functions for the algorithm to compute a single face of an arrangement
 * by Jannis Warnat
 */

CGAL_BEGIN_NAMESPACE

// ---------------------------------------------------------------------------
// Get the vertex representing point_x from an arrangement
//

template<class Arrangement>
typename Arrangement::Vertex_handle vertex_x(Arrangement* arr)
  {
    MY_CGAL_DC_PRINT("begin vertex_x()");

    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Face_iterator Face_iterator;
    typedef typename Arrangement_2::Isolated_vertex_iterator Isolated_vertex_iterator;

    for(Face_iterator fit = arr->faces_begin(); fit != arr->faces_end(); fit++)
      {
        for(Isolated_vertex_iterator ivit = fit->isolated_vertices_begin(); ivit != fit->isolated_vertices_end(); ivit++)
          {
            if(ivit->data().type() == POINT_X)
              {
                Vertex_handle vh_x = ivit;
                return vh_x;
              }
          }
      }
  }

// ---------------------------------------------------------------------------
// Do the vertical decomposition
//

template<class Arrangement>
void
decompose_vertically(
    Arrangement** red,
    Arrangement** blue
)
  {
    MY_CGAL_DC_PRINT("begin decompose_vertically()");
    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;

    insert_external_endpoints(red,blue);

    //get a decomposition as provided by CGAL::decompose()
    std::list<std::pair<Vertex_const_handle, std::pair<Object, Object> > > red_rays;
    decompose(*(*red),std::back_inserter(red_rays));
    std::list<std::pair<Vertex_const_handle, std::pair<Object, Object> > > blue_rays;
    decompose(*(*blue),std::back_inserter(blue_rays));

    split_edges(*red,red_rays.begin(),red_rays.end());
    split_edges(*blue,blue_rays.begin(),blue_rays.end());
  }

// ---------------------------------------------------------------------------
// Use the output from CGAL::decompose() and split the edges accordingly
//

template<typename Arrangement, typename Iterator>
void
split_edges(Arrangement* arr, Iterator begin, Iterator end)
  {
    MY_CGAL_DC_PRINT("split_edges()");
    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Geometry_traits_2 Traits_2;
    typedef typename Traits_2::Point_2 Base_point_2;
    typedef typename Traits_2::X_monotone_curve_2 Base_x_monotone_curve_2;
    typedef typename Traits_2::Ray_2 Ray_2;
    typedef typename Traits_2::Segment_2 Segment_2;

    typedef typename Arrangement_2::Vertex_handle Vertex_handle;

    typedef typename Arrangement_2::Halfedge_const_handle Halfedge_const_handle;
    typedef typename Arrangement_2::Halfedge_handle Halfedge_handle;

    Traits_2* traits;

    //iterate over CGAL decomposition
    for(Iterator rit = begin; rit != end; rit++)

      {
        //the vertex from which the rays were shot
        Vertex_handle rit_first_non_const = arr->non_const_handle(rit->first);

        //no splitting for default vertices
        if(rit->first->data().type() == DEFAULT)
          {
            continue;
          }

        // 0 = above, 1 = below, from CGAL decomposition
        for(int i = 0; i < 2; i++)
          {

            Halfedge_const_handle e, invalid_e;

            if(i == 1) //below

              {
                try
                  {
                    e = object_cast<Halfedge_const_handle>(rit->second.first);
                  }
                catch(Bad_object_cast)
                  {

                  }
              }
            else // i == 0, above

              {
                try
                  {
                    e = object_cast<Halfedge_const_handle>(rit->second.second);
                  }
                catch(Bad_object_cast)
                  {

                  }
              }

            if(e != invalid_e)//ray hit an edge

              {
                //find the split point
                Base_point_2 above_or_below;
                if( i == 1) //below
                  {
                    above_or_below = Base_point_2(rit->first->point().x(),(rit->first->point().y())-1);
                  }
                else //above
                  {
                    above_or_below = Base_point_2(rit->first->point().x(),(rit->first->point().y())+1);
                  }
                Ray_2 ray(rit->first->point(),above_or_below); //ray
                Base_x_monotone_curve_2 curve(e->curve());//curve
                Segment_2 seg = Segment_2(curve); //convert to segment
                Base_point_2 intersection_base_point;

                //split the edge
                Base_x_monotone_curve_2 sub1,sub2;
                Halfedge_handle he_to_split_vertex;

                //weak intersection, edge is already split
                if(rit->first->point().x() == e->target()->point().x())
                  {
                    he_to_split_vertex = arr->non_const_handle(e);
                    intersection_base_point = e->target()->point();
                  }
                else //new strong intersection
                  {
                    Object obj = intersection(ray,seg);

                    try
                      {
                        intersection_base_point = object_cast<Base_point_2>(obj);
                      }
                    catch(Bad_object_cast)
                      {
                        CGAL_assertion(false);
                      }

                    //split the edge
                    traits->split_2_object()(curve,intersection_base_point,sub1,sub2);
                    he_to_split_vertex = arr->split_edge(arr->non_const_handle(e),sub1,sub2);
                  }

                //the split vertex is always DEFAULT
                Vertex_handle split_vertex = he_to_split_vertex->target();
                split_vertex->set_data(DEFAULT);
              }
          }//for 0,1
      }
  }

// ---------------------------------------------------------------------------
// Insert endpoints as external endpoints in arrangement of other color
//

template<typename Arrangement>
void
insert_external_endpoints(Arrangement** red, Arrangement** blue)
  {
    MY_CGAL_DC_PRINT("begin insert_external_endpoints()");
    typedef Arrangement Arrangement_2;
    typedef typename Traits_2::Point_2 Base_point_2;
    typedef typename Arrangement_2::Vertex_iterator Vertex_iterator;
    typedef typename Arrangement_2::Isolated_vertex_iterator Isolated_vertex_iterator;
    typedef typename Arrangement_2::Face_iterator Face_iterator;
    typedef typename Arrangement_2::Face_handle Face_handle;

    std::vector<Base_point_2> red_endpoints;

    //extract red endpoints
    for (Vertex_iterator vit = (*red)->vertices_begin(); vit != (*red)->vertices_end(); ++vit)
      {
        if(vit->data().type() == INTERNAL_ENDPOINT)
          {
            red_endpoints.push_back(vit->point());
          }
      }
    std::vector<Base_point_2> blue_endpoints;

    //extract blue endpoints
    for (Vertex_iterator vit = (*blue)->vertices_begin(); vit != (*blue)->vertices_end(); vit++)
      {
        if(vit->data().type() == INTERNAL_ENDPOINT)
          {
            blue_endpoints.push_back(vit->point());
          }
      }

    //employ adapted single face function to insert the endpoints
    Arrangement_2* red_ext = single_face(*red,&blue_endpoints);
    Arrangement_2* blue_ext = single_face(*blue,&red_endpoints);

    Face_handle red_fh_x = (vertex_x(red_ext))->face();
    Face_handle blue_fh_x = (vertex_x(blue_ext))->face();

    //remove endpoints that are not contained in the face containing point_x
    for(Face_iterator fit = blue_ext->faces_begin(); fit != blue_ext->faces_end(); fit++)
      {
        for (Isolated_vertex_iterator ivit = fit->isolated_vertices_begin(); ivit != fit->isolated_vertices_end();)
          {
            Isolated_vertex_iterator next = ivit;
            next++;
            if(ivit->data().type() == EXTERNAL_ENDPOINT)
              {
                if(ivit->face() != blue_fh_x)
                  {
                     blue_ext->remove_isolated_vertex(ivit);
                  }
              }
            ivit = next;
          }
      }

    for(Face_iterator fit = red_ext->faces_begin(); fit != red_ext->faces_end(); fit++)
      {
        for (Isolated_vertex_iterator ivit = fit->isolated_vertices_begin(); ivit != fit->isolated_vertices_end();)
          {
            Isolated_vertex_iterator next = ivit;
            next++;
            if(ivit->data().type() == EXTERNAL_ENDPOINT)
              {

                if(ivit->face() != red_fh_x)
                  {
                    red_ext->remove_isolated_vertex(ivit);
                  }
              }
            ivit = next;
          }
      }

    delete (*red);
    delete (*blue);
    *red = red_ext;
    *blue = blue_ext;
  }
// ---------------------------------------------------------------------------
// Left right sweep over the points and curves of the red and blue arrangements
//

template<typename Arrangement>
Arrangement*
prepare_and_sweep(Arrangement* red, Arrangement* blue)
  {
    MY_CGAL_DC_PRINT("begin prepare_and_sweep()");
    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Geometry_traits_2 Traits_2;
    typedef typename Traits_2::Point_2 Base_point_2;

    typedef typename Arrangement_2::Vertex_const_iterator Vertex_const_iterator;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Edge_iterator Edge_iterator;
    typedef typename Arrangement_2::Vertex_const_handle Vertex_const_handle;
    typedef typename Arrangement_2::Halfedge_handle Halfedge_handle;

    typedef My_Arr_overlay_traits_2<Traits_2, Arrangement_2, Arrangement_2> Meta_traits_2;
    typedef typename Meta_traits_2::X_monotone_curve_2 X_monotone_curve_2;
    typedef typename Meta_traits_2::Point_2 Point_2;
    typedef My_Sweep_line_2<Arrangement_2,Meta_traits_2> Sweep_line;

    //initialize sweep line with the meta traits
    Meta_traits_2 meta_traits (*(red->geometry_traits()));
    Sweep_line* sweep_line = new Sweep_line(&meta_traits);

    //map the points to avoid duplication of meta points
    std::map<Base_point_2, Point_2> points_map;
    typename std::map<Base_point_2, Point_2>::iterator ppit;
    Object empty_obj;

    //initialize red meta points
    for(Vertex_const_iterator vit = red->vertices_begin(); vit != red->vertices_end(); vit++)
      {
        Vertex_const_handle vch = vit;
        Point_2 meta_point = Point_2 (vch->point(),CGAL::make_object(vch),empty_obj);
        std::pair<Base_point_2, Point_2> p_pair = std::pair<Base_point_2, Point_2>(meta_point.base(),meta_point);
        points_map.insert(p_pair);
      }

    //initialize blue meta points, some may already exist and just need to be updated
    for(Vertex_const_iterator vit = blue->vertices_begin(); vit != blue->vertices_end(); vit++)
      {
        Vertex_const_handle vch = vit;
        Point_2 meta_point = Point_2 (vch->point(),empty_obj,CGAL::make_object(vch));
        std::pair<Base_point_2, Point_2> pair = std::pair<Base_point_2, Point_2>(meta_point.base(),meta_point);

        if((vit->data().type() != DEFAULT))
          {
            ppit = points_map.find(meta_point.base());

            if(ppit != points_map.end())
              {
                ppit->second.set_blue_object(CGAL::make_object(vch));
              }
            else
              {
                points_map.insert(pair);
              }
          }
        else
          {

            points_map.insert(pair);
          }
      }

    //insert all red and blue vertices into the purple arrangement and put the points in a vector,
    //store the purple vertex with the meta point
    Arrangement_2* purple = sweep_line->get_purple();
    std::vector<Point_2> points_vector;

    for(ppit = points_map.begin(); ppit != points_map.end(); ppit++)
      {
        Vertex_const_handle vh = purple->insert_in_face_interior(ppit->first,purple->unbounded_face());
        Vertex_handle non_const_vh = purple->non_const_handle(vh);
        non_const_vh->set_data(ppit->second.purple_data());
        ppit->second.set_purple_object(CGAL::make_object(vh));
        points_vector.push_back(ppit->second);
      }

    //initialize the meta traits red curves and put them in a vector
    Halfedge_handle he, invalid_he;
    std::vector<X_monotone_curve_2> red_xcurves_vec (red->number_of_edges());
    unsigned int i = 0;
    Edge_iterator eit;
    for (eit = red->edges_begin(); eit != red->edges_end(); ++eit, i++)
      {
        he = eit;
        if (he->direction() != ARR_RIGHT_TO_LEFT)
          {
            he = he->twin();
          }

        red_xcurves_vec[i] = X_monotone_curve_2 (eit->curve(),he,invalid_he);
        ppit = points_map.find(he->source()->point());
        red_xcurves_vec[i].set_right_ex_point(ppit->second);
        ppit = points_map.find(he->target()->point());
        red_xcurves_vec[i].set_left_ex_point(ppit->second);
      }

    //initialize the meta traits blue curves and put them in a vector
    std::vector<X_monotone_curve_2> blue_xcurves_vec (blue->number_of_edges());
    i=0;
    for (eit = blue->edges_begin(); eit != blue->edges_end(); ++eit, i++)
      {
        he = eit;
        if (he->direction() != ARR_RIGHT_TO_LEFT)
          {

            he = he->twin();
          }

        blue_xcurves_vec[i] = X_monotone_curve_2 (eit->curve(),invalid_he,he);
        ppit = points_map.find(he->source()->point());
        blue_xcurves_vec[i].set_right_ex_point(ppit->second);
        ppit = points_map.find(he->target()->point());
        blue_xcurves_vec[i].set_left_ex_point(ppit->second);
      }

    //do the sweep
    sweep_line->sweep (red_xcurves_vec.begin(),
        red_xcurves_vec.end(),
        blue_xcurves_vec.begin(),
        blue_xcurves_vec.end(),
        points_vector.begin(),
        points_vector.end());

    delete sweep_line;
    return purple;
  }

// ---------------------------------------------------------------------------
// Construct a singleton arrangement
//

template<class Arrangement>
Arrangement*
singleton_arrangement(
    typename Arrangement::X_monotone_curve_2 curve,
    typename Arrangement::Point_2 point_x
)
  {
    MY_CGAL_DC_PRINT("begin singleton_arrangement()");

    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Face_handle Face_handle;

    Arrangement_2* singleton = new Arrangement_2();
    Face_handle uf = singleton->unbounded_face();

    //insert point_x
    Vertex_handle vh_x = singleton->insert_in_face_interior(point_x, uf);
    vh_x->set_data(POINT_X);

    //insert the left endpoint of the curve
    Vertex_handle vh_left = singleton->insert_in_face_interior(curve.left(),uf);
    vh_left->set_data(INTERNAL_ENDPOINT);

    //insert the right endpoint of the curve
    Vertex_handle vh_right = singleton->insert_in_face_interior(curve.right(),uf);
    vh_right->set_data(INTERNAL_ENDPOINT);

    //insert the curve
    singleton->insert_at_vertices(curve,vh_left,vh_right);

    return singleton;
  }

// ---------------------------------------------------------------------------
// The recursive function to start the single face computation
//

template<class Arrangement, class Iterator>
Arrangement*
red_blue_divide_and_conquer(
    Iterator begin, Iterator end,
    typename Arrangement::Point_2 point_x,
    typename Arrangement::Point_2 pivot)
  {
    MY_CGAL_DC_PRINT("begin red_blue_divide_and_conquer");

    //arrangement to return
    Arrangement* purple;

    //no curves
    if (begin == end)
      {
        purple = new Arrangement();
        return purple;
      }

    Iterator iter = begin;
    ++iter;

    //one curve
    if (iter == end)
      {
        purple = singleton_arrangement<Arrangement>(*begin, point_x);
      }
    //more than one curve

    else
      {
        //find the position to divide the set of curves
        Iterator div_it = begin;
        unsigned int count = 0;

        for (iter = begin; iter != end; ++iter)
          {
            if (count % 2 == 0)
              {
                ++div_it;
              }

            count++;
          }

        //continue recursively
        Arrangement* red = red_blue_divide_and_conquer<Arrangement>(begin, div_it, point_x, pivot);
        Arrangement* blue = red_blue_divide_and_conquer<Arrangement>(div_it, end, point_x, pivot);

        //merge red and blue
        purple = red_blue_merge(red, blue, pivot);
      }
    return purple;
  }

// ---------------------------------------------------------------------------
// Merge a red and a blue face into a purple one
//
template<class Arrangement>
Arrangement*
red_blue_merge(
    Arrangement* red,
    Arrangement* blue,
    typename Arrangement::Point_2 pivot
)
  {
    MY_CGAL_DC_PRINT("begin red_blue_merge()");
    typedef Arrangement Arrangement_2;

    QWidget* qw = qApp->mainWidget();
    QMainWindow* qmw = static_cast<QMainWindow*> (qw);
    QTabWidget* qtw = static_cast<QTabWidget*> (qmw->centralWidget());
    Qt_widget_base_tab* base = static_cast<Qt_widget_base_tab*> (qtw->currentPage());


    clock_t t1,t2;
    t1 = clock();

    //compute the vertical decompositions of both arrangements
    decompose_vertically(&red,&blue);

    t2 = clock();
    double decomp_time = double(t2 - t1);
    base->decomp_time += decomp_time;

    //sweep from left to right
    Arrangement_2* purple = prepare_and_sweep(red,blue);

    t1 = clock();
    double sweep_time = double(t1-t2);
    base->sweep_time += sweep_time;

    //rotate both arrangements
    Arrangement_2* red_rotated = rotate(red,pivot);
    Arrangement_2* blue_rotated = rotate(blue,pivot);
    delete red;
    red = 0;
    delete blue;
    blue = 0;

    t2 = clock();
    double rotate_time = double(t2-t1);
    base->rotate_time += rotate_time;

    //equivalent to sweep from right to left
    Arrangement_2* purple_rotated = prepare_and_sweep(red_rotated,blue_rotated);
    delete red_rotated;
    red_rotated = 0;
    delete blue_rotated;
    blue_rotated = 0;

    t1 = clock();
    sweep_time = double(t1-t2);
    base->sweep_time += sweep_time;

    //undo rotation and fuse the results of the two sweeps
    Arrangement_2* purple_rotated_back = rotate(purple_rotated,pivot);

    t2 = clock();
    rotate_time = double(t2-t1);
    base->rotate_time += rotate_time;

    Arrangement_2* purple_fused = fuse(purple, purple_rotated_back);

    delete purple;
    purple = 0;
    delete purple_rotated;
    purple_rotated = 0;
    delete purple_rotated_back;
    purple_rotated_back = 0;

    t1 = clock();
    double fuse_time = double(t1-t2);
    base->fuse_time += fuse_time;

    //extract the single face containing POINT_X
    Arrangement_2* purple_single_face = single_face(purple_fused);
    delete purple_fused;
    purple_fused = 0;

    return purple_single_face;
  }

// ---------------------------------------------------------------------------
// Rotate an arrangement around pivot
//

template<class Arrangement>
Arrangement*
rotate(Arrangement* arr, typename Arrangement::Point_2 pivot)
  {
    MY_CGAL_DC_PRINT("begin rotate()");

    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Face_handle Face_handle;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Vertex_iterator Vertex_iterator;
    typedef typename Arrangement_2::Edge_iterator Edge_iterator;
    typedef typename Arrangement_2::Point_2 Point_2;
    typedef typename Arrangement_2::X_monotone_curve_2 X_monotone_curve_2;
    typedef Aff_transformation_2<Kernel> Aff_transformation_2;

    //will contain the rotated arrangement
    Arrangement_2* rotated = new Arrangement_2();

    //three matrices
    Aff_transformation_2 translate(1,0,-(pivot.x()),0,1,-(pivot.y()));
    Aff_transformation_2 rot(-1,0,0,0,-1,0);
    Aff_transformation_2 translate_back(1,0,pivot.x(),0,1,pivot.y());

    Face_handle uf = rotated->unbounded_face();

    for(Vertex_iterator vit = arr->vertices_begin(); vit != arr->vertices_end(); vit++)
      {
        //translate and rotate the point
        Point_2 rot_point = translate(vit->point());
        rot_point = rot(rot_point);
        rot_point = translate_back(rot_point);

        //insert in the new arrangement and memorize the new handle
        Vertex_handle vh = rotated->insert_in_face_interior(rot_point,uf);
        vh->set_data(vit->data());
        vit->data().set_vertex_handle(vh);
      }

    for(Edge_iterator ch = arr->edges_begin(); ch != arr->edges_end(); ch++)
      {
        //get the new handles and insert the curve
        Vertex_handle vh_source = ch->source()->data().vertex_handle();
        Vertex_handle vh_target = ch->target()->data().vertex_handle();

        X_monotone_curve_2 curve = X_monotone_curve_2(vh_source->point(),vh_target->point());
        rotated->insert_at_vertices(curve,vh_source,vh_target);

      }

    return rotated;
  }

// ---------------------------------------------------------------------------
// Fuse two arrangements into one
//

template<class Arrangement>
Arrangement* fuse(Arrangement* first, Arrangement* second)
  {
    MY_CGAL_DC_PRINT("begin fuse()");

    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Face_handle Face_handle;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Vertex_iterator Vertex_iterator;
    typedef typename Arrangement_2::Edge_iterator Edge_iterator;
    typedef typename Arrangement_2::Point_2 Point_2;
    typedef typename Arrangement_2::X_monotone_curve_2 X_monotone_curve_2;

    Arrangement_2* fused = new Arrangement_2();
    Face_handle uf = fused->unbounded_face();

    //map the inserted points to their handles to avoid redundant insertions
    std::map<Point_2, Vertex_handle> points_map;
    typename std::map<Point_2, Vertex_handle>::iterator pm_it;

    //insert all vertices from the first arrangement
    for(Vertex_iterator vit = first->vertices_begin(); vit != first->vertices_end(); vit++)
      {
        Vertex_handle vh = fused->insert_in_face_interior(vit->point(),uf);
        vh->set_data(vit->data());
        points_map.insert(std::make_pair(vit->point(),vh));
      }

    //insert additional vertices from the second arrangement
    for(Vertex_iterator vit = second->vertices_begin(); vit != second->vertices_end(); vit++)
      {
        if(vit->data().type() == DEFAULT)
          {
            //legacy DEFAULT and new red blue intersections
            pm_it = points_map.find(vit->point());
            if(pm_it == points_map.end())
              {
                Vertex_handle vh = fused->insert_in_face_interior(vit->point(),uf);
                vh->set_data(vit->data());
                points_map.insert(std::make_pair(vit->point(),vh));
              }
          }
      }

    //memorize inserted curves as point pairs to avoid redundant insertions
    std::set< std::pair < Point_2, Point_2> > point_pairs;
    typename std::set< std::pair < Point_2, Point_2> >::iterator pair_it;

    //insert all curves from the first arrangement and memorize them
    for(Edge_iterator ch = first->edges_begin(); ch != first->edges_end(); ch++)
      {
        Point_2 source = ch->source()->point();
        Point_2 target = ch->target()->point();

        pm_it = points_map.find(source);
        Vertex_handle vh_source = pm_it->second;
        pm_it = points_map.find(target);
        Vertex_handle vh_target = pm_it->second;

        X_monotone_curve_2 curve = X_monotone_curve_2(source,target);
        fused->insert_at_vertices(curve,vh_source,vh_target);

        if(ch->direction() == ARR_LEFT_TO_RIGHT)
          {
            point_pairs.insert(std::make_pair(source,target));
          }
        else
          {
            point_pairs.insert(std::make_pair(target,source));
          }
      }

    //insert additional curves from the second arrangement
    for(Edge_iterator ch = second->edges_begin(); ch != second->edges_end(); ch++)
      {
        Point_2 source = ch->source()->point();
        Point_2 target = ch->target()->point();

        if(ch->direction() == ARR_LEFT_TO_RIGHT)
          {
            pair_it = point_pairs.find(std::make_pair(source,target));
          }
        else
          {
            pair_it = point_pairs.find(std::make_pair(target,source));
          }

        if(pair_it == point_pairs.end())
          {
            pm_it = points_map.find(source);
            Vertex_handle vh_source = pm_it->second;
            pm_it = points_map.find(target);
            Vertex_handle vh_target = pm_it->second;
            X_monotone_curve_2 curve = X_monotone_curve_2(source,target);
            fused->insert_at_vertices(curve,vh_source,vh_target);
          }
      }

    return fused;
  }

// ---------------------------------------------------------------------------
// Extract a single face from an arrangement to get rid of superfluous faces;
// also used by insert_external_endpoints()
//

template<class Arrangement>
Arrangement* single_face(Arrangement* arr,
    std::vector<typename Arrangement::Point_2>* vector = 0)
  {
    MY_CGAL_DC_PRINT("begin single_face()");

    typedef Arrangement Arrangement_2;
    typedef typename Arrangement_2::Vertex_handle Vertex_handle;
    typedef typename Arrangement_2::Face_handle Face_handle;
    typedef typename Arrangement_2::Halfedge_handle Halfedge_handle;
    typedef typename Arrangement_2::Edge_iterator Edge_iterator;
    typedef typename Arrangement_2::Face_iterator Face_iterator;
    typedef typename Arrangement_2::Isolated_vertex_iterator Isolated_vertex_iterator;
    typedef typename Arrangement_2::Inner_ccb_iterator Inner_ccb_iterator;
    typedef typename Arrangement_2::Ccb_halfedge_circulator Ccb_halfedge_circulator;
    typedef typename Arrangement_2::Point_2 Point_2;

    //output arrangement
    Arrangement_2* single = new Arrangement_2();

    //insert point_x;
    Vertex_handle arr_vh_x = vertex_x(arr);
    Vertex_handle vh_x = single->insert_in_face_interior(arr_vh_x->point(),single->unbounded_face());
    vh_x->set_data(POINT_X);
    Face_handle fh_x = arr_vh_x->face();

    Vertex_handle vh_ext;

    //extension for use in insert_external_endpoints
    if(vector != 0)
      {
        typename std::vector<Point_2>::iterator pit;
        for(pit = vector->begin(); pit != vector->end(); pit++)
          {
            vh_ext = single->insert_in_face_interior(*pit,single->unbounded_face());
            vh_ext->set_data(EXTERNAL_ENDPOINT);
          }
      }

    //prepare dcel
    for(Edge_iterator eit = arr->edges_begin(); eit != arr->edges_end(); eit++)
      {
        eit->set_data(false);
        eit->twin()->set_data(false);
      }

    //Insert the holes in fh_x into 'single'
    for (Inner_ccb_iterator hole = fh_x->holes_begin(); hole != fh_x->holes_end(); ++hole)
      {
        Ccb_halfedge_circulator curr = *hole;

        do
          {
            Halfedge_handle arr_he = curr;
            if(arr_he->data() == false)
              {
                Vertex_handle single_he_source;
                if(arr_he->source()->data().has_vertex_handle())
                  {
                    single_he_source = arr_he->source()->data().vertex_handle();
                  }
                else
                  {
                    single_he_source = single->insert_in_face_interior(arr_he->source()->point(),vh_x->face());
                    single_he_source->set_data(arr_he->source()->data());
                    arr_he->source()->data().set_vertex_handle(single_he_source);
                  }

                Vertex_handle single_he_target;
                if(arr_he->target()->data().has_vertex_handle())
                  {
                    single_he_target = arr_he->target()->data().vertex_handle();
                  }
                else
                  {
                    single_he_target = single->insert_in_face_interior(arr_he->target()->point(),vh_x->face());
                    single_he_target->set_data(arr_he->target()->data());
                    arr_he->target()->data().set_vertex_handle(single_he_target);
                  }

                single->insert_at_vertices(arr_he->curve(),single_he_source,single_he_target);
                arr_he->set_data(true);
                arr_he->twin()->set_data(true);
              }
            curr++;
          }while(curr != *hole);
      }

    //Insert the outer ccb into 'single'
    if(!(fh_x->is_unbounded()))
      {
        Ccb_halfedge_circulator curr = fh_x->outer_ccb();

        do
          {
            Halfedge_handle arr_he = curr;
            if(arr_he->data() == false)
              {
                Vertex_handle single_he_source;
                if(arr_he->source()->data().has_vertex_handle())
                  {
                    single_he_source = arr_he->source()->data().vertex_handle();
                  }
                else
                  {
                    single_he_source = single->insert_in_face_interior(arr_he->source()->point(),vh_x->face());
                    single_he_source->set_data(arr_he->source()->data());
                    arr_he->source()->data().set_vertex_handle(single_he_source);
                  }

                Vertex_handle single_he_target;
                if(arr_he->target()->data().has_vertex_handle())
                  {
                    single_he_target = arr_he->target()->data().vertex_handle();
                  }
                else
                  {
                    single_he_target = single->insert_in_face_interior(arr_he->target()->point(),vh_x->face());
                    single_he_target->set_data(arr_he->target()->data());
                    arr_he->target()->data().set_vertex_handle(single_he_target);
                  }

                single->insert_at_vertices(arr_he->curve(),single_he_source,single_he_target);
                arr_he->set_data(true);
                arr_he->twin()->set_data(true);
              }
            curr++;
          }while(curr != fh_x->outer_ccb());
      }

    return single;
  }

CGAL_END_NAMESPACE

#endif


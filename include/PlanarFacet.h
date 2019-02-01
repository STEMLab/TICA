#pragma once
#include "Plane.h"
#include "Point2D.h"
#include "Line2D.h"
#include "Ray3D.h"
#include "BinTexture.h"
#include <vector>

struct PlanarFacet;
struct PlanarFacetEdge;
struct PlanarFacetVertex;

struct PlanarFacetVertex {
	PlanarFacetVertex(PlanarFacet*, const Point2D&);

	Point2D p;

	PlanarFacet *base;

	PlanarFacetEdge* inbound;
	PlanarFacetEdge* outbound;

	//=======================================
	// get_prev_identical_point()
	// get_next_identical_point()
	//---------------------------------------
	// RESULT
	//  get the prev/next vertex sharing the actual spatial point
	//=======================================
	PlanarFacetVertex* get_prev_identical_point(void) const;
	PlanarFacetVertex* get_next_identical_point(void) const;

	//=======================================
	// to_point3d()
	//---------------------------------------
	// RESULT
	//  get averaged point in 3d space
	//=======================================
	Point3D to_point3d(void) const;


	//=======================================
	// remove()
	//---------------------------------------
	// RESULT
	//  inbound and outbound edges are merged into one edge
	//  returns the merged edge
	//  (the other edge is automatically removed, but should be freed manually)
	//  returns 0 if failed or no edges exist
	//  PlanarFacetVertex should be freed manually
	//=======================================
	PlanarFacetEdge* remove();
};

struct PlanarFacetEdge {
	PlanarFacetEdge(PlanarFacet*, PlanarFacetVertex*, PlanarFacetVertex*);

	PlanarFacet *base;
	PlanarFacetVertex *u;
	PlanarFacetVertex *v;
	PlanarFacetEdge *opposite;

	PlanarFacetEdge *prev(void) const;
	PlanarFacetEdge *next(void) const;

	bool set_opposite_connection(PlanarFacetEdge *n);
	
	//=======================================
	// to_vector{2/3}()
	//---------------------------------------
	// RESULT
	//  get averaged point in {2/3}d space
	//=======================================
	Vector3D to_vector3(void) const;
	Vector2D to_vector2(void) const;

	//=======================================
	// split()
	//---------------------------------------
	// PARAM
	//  ratio: 0~1
	//         0:u
	//         1:v
	// RESULT
	//  split the edge into two edges (u,w), (w,v) where w = (1-ratio)*u + (ratio)*v
	//=======================================
	PlanarFacetVertex* split(length_t ratio);
};

struct PointOnFacet {

};
struct OnVertexPoint : public PointOnFacet {
	PlanarFacetVertex *base;
};
struct OnEdgePoint : public PointOnFacet {
	PlanarFacetEdge *base;
	length_t alpha;
};
struct InteriorPoint : public PointOnFacet {
	PlanarFacetEdge *base;
	Point2D p;
};

struct PlanarFacet {
	PlanarFacet(void);
	PlanarFacet(const Plane&);

	Plane plane;

	std::vector<PlanarFacetVertex*> vertices;
	std::vector<PlanarFacetEdge*> edges;

	PlanarFacetEdge* exterior;
	std::vector<PlanarFacetEdge*> holes;

	bool includes_point(const Point2D&) const;
	PlanarFacetEdge* find_nearest_edge(const Point2D&) const;

	PlanarFacetVertex* create_vertex(const Point2D& p);
	PlanarFacetEdge* create_edge(PlanarFacetVertex * p, PlanarFacetVertex * q);
	PlanarFacetEdge* create_edge(const Point2D& p, const Point2D& q);

	void set_exterior(PlanarFacetEdge *e);
	void add_hole(PlanarFacetEdge *e);
	
	//***************************************
	//***************************************
	// OPERATIONS for CHECKING
	//***************************************
	bool find_ray_intersection(const Ray3D& ray, Point2D* pt, PlanarFacetVertex* nearest_vertex, PlanarFacetEdge* nearest_edge) const;

	//***************************************
	// END OF OPERATIONS for CHECKING
	//***************************************
	//***************************************
	bool is_collapsed_vertex(PlanarFacetVertex* v) const;

	//***************************************
	//***************************************
	// OPERATIONS for MODIFICATION
	//***************************************
	bool remove_vertex_from_list(PlanarFacetVertex* v, bool del);
	bool remove_edge_from_list(PlanarFacetEdge* e, bool del);
	void remove_collapsed_vertices(void);

	//=======================================
	// merge()
	//---------------------------------------
	// PARAM
	//  facet: a facet to merge
	//---------------------------------------
	// PARAM
	//  edge: this facet will be merged with the opposite facet beyond the edge
	//---------------------------------------
	// PRECONDITION
	//  two facets shares an edge
	//  two facets are on the same plane
	//
	// RESULT
	//  *this becomes the merged PlanarFacet
	//=======================================
	bool merge(PlanarFacet* facet);
	bool merge(PlanarFacetEdge* edge);


	//=======================================
	// split()
	//---------------------------------------
	// PARAM
	//  u, v: vertices for newly created edge
	//---------------------------------------
	// PRECONDITION
	//  two facets shares an edge
	//  two facets are on the same plane
	//
	// RESULT
	//  *this becomes the one split PlanarFacet
	//  the other PlanarFacet is returned
	//=======================================
	PlanarFacet* split(PlanarFacetVertex* u, PlanarFacetVertex* v);

	//***************************************
	// END OF OPERATIONS for MODIFICATION
	//***************************************
	//***************************************


	//***************************************
	//***************************************
	// FUNCTIONS for DRAWING
	//***************************************
	void prepare_for_drawing(BinTexture *t=0);
	void draw(BinTexture *t=0);
	void release(void);

	void triangulate(BinTexture*t=0);
	bool dirty;
	int drawing_object_id;
	//***************************************
	// END OF FUNCTIONS for DRAWING
	//***************************************
	//***************************************
};

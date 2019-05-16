#pragma once

#include "Common.h"
#include "Geometry.h"
#include <vector>

class Vertex;
class FacetVertex;
class FacetEdge;
class Facet;

void draw_sphere(float x, float y, float z, double radius);

struct DrawSetting {
	void setColori(unsigned int i);
	void setColorf(float r, float g, float b, float alpha = 1.0);

	virtual void begin_facets(void) = 0;
	virtual void begin_vertices(void) = 0;
	virtual void begin_edges(void) = 0;

	virtual bool begin_vertex(Vertex*) = 0;
	virtual bool begin_facet(int, Facet*) = 0;
	virtual bool begin_facetvertex(int, FacetVertex*) = 0;
	virtual bool begin_facetedge(int, FacetEdge*) = 0;
};

class Vertex {
public:
	friend class Facet;
	Vertex(coord_t _x = 0, coord_t _y = 0, coord_t _z = 0);
	void draw(length_t r = 0, DrawSetting* d = 0) ;

	bool join(Vertex *v);
	bool move_to(const Point3D& p);
	bool get_feasible_point_nearest_to_point(const Point3D& p, Point3D* ret, const Plane* default_plane = 0) const;
	bool get_feasible_point_nearest_to_ray(const Line3D& ray, Point3D* ret, const Plane* default_plane) const;
	bool move_to_ray(const Line3D& ray, const Plane* default_plane = 0);

	bool absorb(Vertex* v);
	bool remove(void);

	void triangulate_associated_facets(void);

	Point3D to_point(void) const;
	Vector3D to_vector(void) const;
	Facet* get_facet(int i) const;

	Facet* get_a_shared_facet(const Vertex *v) const;
	int num_shared_facets(const Vertex *v=0) const;
	bool share_a_facet(const Vertex *v) const;
	bool form_an_edge(const Vertex *v) const;

private:
	coord_t x;
	coord_t y;
	coord_t z;

	std::vector<FacetVertex*> associated_facetvertex;
};

class FacetVertex {
public:
	friend class Vertex;
	friend class Facet;
	friend class FacetEdge;

	Point3D to_point3(void) const;
	FacetVertex *prev(void) const;
	FacetVertex *next(void) const;

	Facet* get_facet(void);
	Vertex* get_vertex(void);

private:
	FacetVertex(Facet*, Vertex* v);

	Vertex* associated_vertex;

	FacetEdge* incoming;
	FacetEdge* outgoing;

	Facet* facet;
};

class FacetEdge {
public:
	friend class Facet;
	friend class FacetVertex;

	Vector3D to_vector3(void) const;
	FacetVertex *get_u();
	FacetVertex *get_v();
	Facet* get_f();

	bool is_shared(void) const;

	FacetEdge* next() const;
	FacetEdge* prev() const;

private:
	FacetEdge(Facet*, FacetVertex*, FacetVertex*);


	Vector3D compute_ring_normal_vector(void) const;

	FacetVertex* u;
	FacetVertex* v;

	Facet* facet;
};

#include "Texture.h"

class Facet {
public:
	void release(std::vector<Vertex*> *pts=0);
	void triangulate(void);
	void draw(DrawSetting* d = 0, int idx = 0) ;
	void draw_edge(DrawSetting* d = 0) ;
	void draw_vertex(DrawSetting* d = 0) ;

	static Facet* create_facet(Vertex* v1, Vertex* v2, Vertex* v3);
	static Facet* create_facet(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4); 
	static Facet* create_facet(const std::vector<Vertex *> &vs);

	void make_hole(const std::vector<Vertex *> &vs);

	FacetVertex* create_vertex(Vertex *v);
	FacetEdge* create_edge(FacetVertex *u, FacetVertex *v);
	
	FacetEdge* get_edge(int i) const;
	FacetVertex* get_vertex(int i) const;
	FacetVertex* get_vertex(Vertex *v) const;

	FacetEdge* get_exteior_edge(void) const;
	FacetEdge* get_hole_edge(int i) const;
	int num_edges(void) const;
	int num_holes(void) const;
	int num_vertices(void) const;
	int num_shared_edge(Facet* f);

	// Checks
	bool has_no_adjacent_polygons(void) const;


	// Actions
	static Facet* merge(Facet* fa, Facet* fb, std::vector<Vertex*>* rv, std::vector<Facet*>*);
	void make_solid(const Vector3D&, std::vector<Vertex*>*, std::vector<Facet*>*);
	bool split_by_edge(FacetVertex *u, FacetVertex *v, std::vector<Facet*>*);
	void reverse(void);

	//bool remove_vertex(int i);
	bool remove_vertex(FacetVertex *v);
	bool remove_edge(int i);
	bool remove_edge(FacetEdge *e);
	bool remove_hole(int i);
	bool split_edge(int i, Vertex *v);
	bool split_edge(FacetEdge* e, Vertex *v);

	bool planarize(void);

	Plane get_plane(void) const;

private:
	//FacetVertex *create_vertex(Vertex*);
	//FacetEdge *create_edge(FacetVertex*, FacetVertex*);

	Facet();

	std::vector<FacetVertex*> vertex;
	std::vector<FacetEdge*> edge;

	Plane plane;
	FacetEdge* exterior;
	std::vector<FacetEdge*> holes;

	unsigned int gl_polygon_id;

	// additional unit-task for editing
	void detatch_a_vertex(FacetVertex *v);


public:
	__temp_texture *__temp_texture;
};
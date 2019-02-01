#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "PlanarFacet.h"
#include "Point3DAccumulator.h"

PlanarFacetVertex::PlanarFacetVertex(PlanarFacet *poly, const Point2D& _p)
	: base(poly), p(_p), inbound(0), outbound(0) {

}

PlanarFacetVertex* PlanarFacetVertex::get_prev_identical_point(void) const {
	if (!inbound) return 0;
	if (!inbound->opposite) return 0;
	return inbound->opposite->u;
}
PlanarFacetVertex* PlanarFacetVertex::get_next_identical_point(void) const {
	if (!outbound) return 0;
	if (!outbound->opposite) return 0;
	return outbound->opposite->v;
}

Point3D PlanarFacetVertex::to_point3d(void) const {
	Point3DAccumulator acc;
	acc += base->plane.convert(this->p);
	{
		PlanarFacetVertex *p = get_next_identical_point();
		while (p && p != this) {
			acc += p->base->plane.convert(p->p);
			p = p->get_next_identical_point();
		}
		if (!p) {
			p = get_prev_identical_point();
			while (p && p != this) {
				acc += p->base->plane.convert(p->p);
				p = p->get_prev_identical_point();
			}
		}
	}
	return acc.get_average_point();
}

PlanarFacetEdge* PlanarFacetVertex::remove() {
	if (!inbound && !outbound) return 0;
	
	// disconnect opposite edges
	if (inbound) {
		if (inbound->opposite) inbound->opposite->opposite = 0;
		inbound->opposite = 0;
	}
	if (outbound) {
		if (outbound->opposite) outbound->opposite->opposite = 0;
		outbound->opposite = 0;
	}

	// connect inbound->u to outbound->v
	PlanarFacetEdge *e_merged = 0;
	if (outbound) {
		if (inbound) {
			inbound->v = outbound->v;
		}
		if (outbound->v) {
			outbound->v->inbound = inbound;
		}
		outbound->v = 0;
		outbound->base = 0;
		base->remove_edge_from_list(outbound, true);
		e_merged = inbound;
	}
	else {
		if (inbound) {
			inbound->u->outbound = 0;
			inbound->u = 0;
		}
		inbound->base = 0;
		base->remove_edge_from_list(inbound, true);
	}
	base->remove_vertex_from_list(this, true);
	return e_merged;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PlanarFacetEdge::PlanarFacetEdge(PlanarFacet* _base, PlanarFacetVertex* _u, PlanarFacetVertex* _v)
: base(_base), u(_u), v(_v), opposite(0) {
	assert(_u && _v);
	
	assert(_u->outbound==0);
	_u->outbound = this;
	assert(_v->inbound == 0);
	_v->inbound = this;
}

PlanarFacetEdge* PlanarFacetEdge::next(void) const {
	if (v) return v->outbound;
	return 0;
}

PlanarFacetEdge* PlanarFacetEdge::prev(void) const {
	if (u) return u->inbound;
	return 0;
}

bool PlanarFacetEdge::set_opposite_connection(PlanarFacetEdge *n) {
	if (!n) {
		if (opposite) {
			opposite->opposite = 0;
			opposite = 0;
		}
		return true;
	}
	if (opposite || n->opposite) return false;
	if (n->u->to_point3d() == this->v->to_point3d()
		&& n->v->to_point3d() == this->u->to_point3d()) {
		n->opposite = this;
		this->opposite = n;
		return true;
	}
	return false;
}

//=======================================
// to_vector3()
//---------------------------------------
// RESULT
//  get averaged point in 3d space
//=======================================
Vector3D PlanarFacetEdge::to_vector3(void) const {
	return v->to_point3d() - u->to_point3d();
}
Vector2D PlanarFacetEdge::to_vector2(void) const {
	return v->p - u->p;
}

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
PlanarFacetVertex* PlanarFacetEdge::split(length_t ratio) {
	assert(u&&v);

	PlanarFacetVertex* new_v = new PlanarFacetVertex(base, u->p + ratio * (v->p - u->p));
	base->vertices.push_back(new_v);
	
	u->outbound = 0;
	new_v->outbound = this;
	PlanarFacetEdge* new_e = new PlanarFacetEdge(base, u, new_v);
	base->edges.push_back(new_e);
	
	u = new_v;

	if (opposite) {
		opposite->opposite = 0;
		PlanarFacetVertex* new_opposite_v = opposite->split(1 - ratio);
			
		opposite = 0;
		if (new_opposite_v->inbound) new_opposite_v->inbound->set_opposite_connection(this);
		if (new_opposite_v->outbound) new_opposite_v->outbound->set_opposite_connection(new_e);
	}
	return new_v;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PlanarFacet::PlanarFacet(void) : exterior(0), dirty(false), drawing_object_id(-1) { ; }
PlanarFacet::PlanarFacet(const Plane& p) : exterior(0), plane(p), dirty(false), drawing_object_id(-1) { ; }

bool PlanarFacet::includes_point(const Point2D& point) const {
	// Modified version of a source from Computational Geometry in C
	if (edges.empty()) return false;
	int rcross = 0, lcross = 0;
	for (int i = 0; i < edges.size(); ++i) {
		coord_t ux;
		coord_t uy;
		coord_t vx;
		coord_t vy;

		const Point2D& u = edges[i]->u->p;
		const Point2D& v = edges[i]->v->p;

		ux = u.x - point.x;
		uy = u.y - point.y;
		vx = v.x - point.x;
		vy = v.y - point.y;

		if (ux == 0 && uy == 0) {
			// [ASSUME] vertices are listed in the counter-clockwise order.
			// [WARNING] == operator is used on floating points
			// on vertex
			return true;
		}

		if ((uy > 0) != (vy > 0)) {
			int sign = ((uy > vy) - (uy < vy));
			coord_t x = vx * uy - ux * vy;

			if (x * sign > 0) ++rcross;
		}

		if ((uy < 0) != (vy < 0)) {
			int sign = ((uy > vy) - (uy < vy));
			coord_t x = vx * uy - ux * vy;

			if (x * sign < 0) ++lcross;
		}
	}

	if ((rcross & 0x1) != (lcross & 0x1)) {
		// on_edge
		return true;
	}

	if ((rcross & 0x1) == 1) {
		// inside
		return true;
	}

	return false;
}
PlanarFacetEdge* PlanarFacet::find_nearest_edge(const Point2D&) const {
	// TODO
	throw "NOT IMPLEMENTED";
	return 0;
}

PlanarFacetVertex* PlanarFacet::create_vertex(const Point2D& p) {
	PlanarFacetVertex * ret = new PlanarFacetVertex(this, p);
	vertices.push_back(ret);
	return ret;
}

PlanarFacetEdge* PlanarFacet::create_edge(PlanarFacetVertex * p, PlanarFacetVertex * q) {
	PlanarFacetEdge * ret = new PlanarFacetEdge(this, p, q);
	edges.push_back(ret);
	dirty = true;
	return ret;
}
PlanarFacetEdge* PlanarFacet::create_edge(const Point2D& _p, const Point2D& _q) {
	PlanarFacetVertex * p = new PlanarFacetVertex(this, _p);
	PlanarFacetVertex * q = new PlanarFacetVertex(this, _q);
	return create_edge(p, q);
}

void PlanarFacet::set_exterior(PlanarFacetEdge *e) {
	exterior = e;
}
void PlanarFacet::add_hole(PlanarFacetEdge *e) {
	holes.push_back(e);
}

//***************************************
//***************************************
// OPERATIONS for CHECKING
//***************************************
bool PlanarFacet::find_ray_intersection(const Ray3D& ray, Point2D* pt, PlanarFacetVertex* nearest_vertex, PlanarFacetEdge* nearest_edge) const {
	throw "NOT IMPLEMENTED";
}
#include <iostream>
using namespace std;

bool PlanarFacet::is_collapsed_vertex(PlanarFacetVertex* v) const {
	if (!v) return false;
	if (v->base != this) return false;
	if (v->inbound) {
		if (v->inbound->u->to_point3d() == v->to_point3d()) return true;
	}
	if (v->outbound) {
		if (v->outbound->v->to_point3d() == v->to_point3d()) return true;
	}
	if (v->inbound && v->outbound) {
		if (v->outbound->v->to_point3d() == v->inbound->u->to_point3d()) return true;
	}
	return false;
}

//***************************************
// END OF OPERATIONS for CHECKING
//***************************************
//***************************************


//***************************************
//***************************************
// OPERATIONS for MODIFICATION
//***************************************
bool PlanarFacet::remove_vertex_from_list(PlanarFacetVertex* v, bool del) {
	for (int i = 0; i < (int)vertices.size(); ++i) {
		if (vertices[i] == v) {
			if(del) delete vertices[i];
			vertices.erase(vertices.begin() + i);
			return true;
		}
	}
	return false;
}
bool PlanarFacet::remove_edge_from_list(PlanarFacetEdge* e, bool del) {
	for (int i = 0; i < (int)edges.size(); ++i) {
		if (edges[i] == e) {
			PlanarFacetEdge* e_adj = e->next();
			if (!e_adj) e_adj = e->prev();
			if (exterior == edges[i]) {
				exterior = e_adj;
			}
			for (int j = 0; j < holes.size(); ++j) {
				if (holes[j] == e) {
					holes[j] = e_adj;
					break;
				}
			}
			if(del) delete edges[i];
			edges.erase(edges.begin() + i);
			return true;
		}
	}
	return false;
}

bool PlanarFacet::merge(PlanarFacet* facet) {
	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i]->opposite && edges[i]->opposite->base == facet) {
			return merge(edges[i]);
		}
	}
	return false;
}
bool PlanarFacet::merge(PlanarFacetEdge* edge) {
	if (edge->base != this) return false;
	if (edge->opposite == 0) return false;

	PlanarFacetEdge * e_opposite = edge->opposite;
	PlanarFacet * f_opposite = e_opposite->base;
	PlanarFacetEdge * e = e_opposite->next();
	PlanarFacetEdge * e_end = e_opposite->prev();

	PlanarFacetVertex *prev_v = edge->u;
	edge->u->outbound = 0;
	edge->v->inbound = 0;
	edge->set_opposite_connection(0);

	while (e != e_opposite) {
		PlanarFacetVertex *v = 0;

		if (e != e_end) {
			v = create_vertex(this->plane.convert(f_opposite->plane.convert(e->v->p)));
		}
		else {
			v = edge->v;
		}
		PlanarFacetEdge *adj = e->opposite;
		e->set_opposite_connection(0);

		PlanarFacetEdge *new_e = create_edge(prev_v, v);
		new_e->set_opposite_connection(adj);

		prev_v = v;
		e = e->next();
	}

	remove_edge_from_list(edge, true);
	remove_collapsed_vertices();
	return true;
}

void PlanarFacet::remove_collapsed_vertices(void) {
	bool flag = true;
	while (flag) {
		flag = false;
		for (int i = 0; i < vertices.size(); ++i) {
			if (is_collapsed_vertex(vertices[i])) {
				vertices[i]->remove();
				flag = true;
				break;
			}
		}
	}
}

PlanarFacet* PlanarFacet::split(PlanarFacetVertex* u, PlanarFacetVertex* v) {
	if (u->outbound->v == v) return 0;
	PlanarFacet *f = new PlanarFacet(plane);
	PlanarFacetVertex* w = v;
	while (w != u) {
		w->outbound->base = f;
		w->base = f;
		remove_edge_from_list(w->outbound, false);
		remove_vertex_from_list(w, false);

		f->edges.push_back(w->outbound);
		f->vertices.push_back(w);
		w = w->outbound->v;
	}

	PlanarFacetVertex* new_v = create_vertex(v->p); // belongs to *this
	new_v->inbound = v->inbound;
	if (v->inbound) v->inbound->v = new_v;
	v->inbound = 0;

	PlanarFacetVertex* new_u = f->create_vertex(u->p); // belongs to new facet
	new_u->inbound = u->inbound;
	if (u->inbound) u->inbound->v = new_u;
	u->inbound = 0;

	PlanarFacetEdge *new_e1 = create_edge(new_v, u);
	set_exterior(new_e1);
	
	PlanarFacetEdge *new_e2 = f->create_edge(new_u, v);
	f->set_exterior(new_e2);
	
	new_e1->set_opposite_connection(new_e2);

	return f;
}

//***************************************
// END OF OPERATIONS for MODIFICATION
//***************************************
//***************************************


//***************************************
//***************************************
// FUNCTIONS for DRAWING
//***************************************
void PlanarFacet::prepare_for_drawing(BinTexture *t) {
	if (drawing_object_id == -1) {
		drawing_object_id = glGenLists(1);
	}
	glNewList(drawing_object_id, GL_COMPILE);
	triangulate(t);
	glEndList();
	dirty = false;
}

void PlanarFacet::draw(BinTexture *t) {
	//TODO
	if (dirty) prepare_for_drawing(t);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glCallList(drawing_object_id);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glDisable(GL_TEXTURE_2D);
	glPointSize(3);
	for (int i = 0; i < (int)this->edges.size(); ++i) {
		Point3D u = plane.convert(edges[i]->u->p);
		Point3D v = plane.convert(edges[i]->v->p);

		if (edges[i]->opposite == 0) { glColor3f(1, 0, 0); }
		else { glColor3f(0, 1, 0); }

		glBegin(GL_LINES);
		glVertex3f(u.x, u.y, u.z);
		glVertex3f(v.x, v.y, v.z);
		glEnd();

		glBegin(GL_POINTS);
		glVertex3f(u.x, u.y, u.z);
		glEnd();
	}
}

void PlanarFacet::release(void) {
	dirty = false;
	//vertices.clear();
	if (drawing_object_id >= 0) glDeleteLists(drawing_object_id, 1);
	drawing_object_id = -1;
	for (int i = 0; i < (int)edges.size(); ++i) {
		if (edges[i]->opposite) {
			edges[i]->opposite->opposite = 0;
		}
		delete edges[i];
	}
	for (int i = 0; i < (int)vertices.size(); ++i) delete vertices[i];
	vertices.clear();
	edges.clear();
}

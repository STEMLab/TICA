#include "GeomObjects.h"
#include <GL/glew.h>

#include <iostream>
#include <map>
#include <set>
#include "Util.h"
using namespace std;

GLUquadric* create_gluquadric_solid(void) {
	GLUquadric *quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_FILL);
	return quad;
}

GLUquadric* create_gluquadric_wireframe(void) {
	GLUquadric *quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_LINE);
	return quad;
}

static GLUquadric *quadric_solid = create_gluquadric_solid();
static GLUquadric *quadric_wireframe = create_gluquadric_wireframe();

void draw_sphere(float x, float y, float z, double radius) {
	glPushMatrix();
	glTranslatef(x, y, z);
	gluSphere(quadric_solid, radius, 8, 8);
	glPopMatrix();
}

void DrawSetting::setColori(unsigned int i) {
	unsigned char b[4];
	decompose_integer(i, b);
	glColor4ub(b[0], b[1], b[2], b[3]);
}
void DrawSetting::setColorf(float r, float g, float b, float alpha) {
	glColor4f(r, g, b, alpha);
}

SubspaceFilter::SubspaceFilter(void)
: dof(3) {
}
SubspaceFilter::SubspaceFilter(const Plane& p)
	: dof(2), plane(p) {
}
SubspaceFilter::SubspaceFilter(const Line3D& l)
	: dof(1), line(l) {
}
bool SubspaceFilter::projection(const Point3D& p, Point3D* _ret, const Vector3D& projectionVector) const {
	scalar_t alpha;
	scalar_t beta;
	Point3D ret = p;

	switch (dof) {
	case 0:
		if (!projectionVector.is_zero()) {
			if (!interp(point, Line3D(p, projectionVector), &alpha)) {
				return false;
			}
			if (!(point-(p+alpha*projectionVector)).is_zero()) return false;
		}
		else {
			ret = point;
		}
		break;
	case 1:
		if (projectionVector.is_zero()) {
			if (!interp(p, line, &alpha)) {
				return false;
			}
			ret = line.p + alpha*line.v;
		}
		else {
			if (!interp(Line3D(p, projectionVector), line, &alpha, &beta)) {
				return false;
			}
			ret = line.p + beta*line.v;
		}
		break;
	case 2:
		if (projectionVector.is_zero()) {
			if (!interp(p, plane, &alpha)) {
				return false;
			}
			ret = p - alpha * plane.h;
		}
		else {
			if (!interp(Line3D(p, projectionVector), plane, &alpha)) {
				return false;
			}
			ret = p + alpha * projectionVector;
		}
	case 3:
		break;
	default:
		return false;
	}
	if (_ret) *_ret = ret;
	return true;
}

bool SubspaceFilter::add_plane(const Plane& p, bool invalidate) {
	if (invalidate) {
		bool r = add_plane(p, false);
		if (!r) {
			dof = -1;
		}
		return r;
	}
	scalar_t alpha;
	scalar_t beta;

	switch (dof) {
	case 0:
		if (!interp(point, p, &alpha)) {
			return false;
		}
		if (alpha * p.h.length() < EPSILON) return true;
		return false;
	case 1:
		if (!interp(line, p, &alpha)) {
			// parallel
			if (!interp(line.p, p, &alpha)) return false;
			if (abs(alpha*p.h.length()) < EPSILON) return true; // line is on the plane 
			return false;
		}
		point = line.p + alpha * line.v;
		dof = 0;
		return true;
	case 2:
		if (!interp(plane, p, &line, 0, 0)) {
			// parallel
			if (!interp(p.p, plane, &alpha)) return false;
			if (abs(alpha * plane.h.length()) < EPSILON) return true;
			return false;
		}
		if (line.v.is_zero()) return false;
		line.v = line.v.normalized();
		dof = 1;
		return true;
	case 3:
		plane = p;
		dof = 2;
		return true;
	default:
		return false;
	}
}

Vertex::Vertex(coord_t _x, coord_t _y, coord_t _z)
	:x(_x), y(_y), z(_z)
{
	;
}
bool Vertex::move_to(const Point3D& p) {
	x = p.x;
	y = p.y;
	z = p.z;
	return true;
}

bool Vertex::get_feasible_point_nearest_to_point(const Point3D& p, Point3D* ret, const Plane* default_plane) const {
	Point3D q;
	Plane plane;
	scalar_t alpha;
	scalar_t beta;
	Line3D l;
	switch (associated_facetvertex.size()) {
	case 0:
		if (!default_plane) {
			if (ret) *ret = p;
			return true;
		}
		if (!interp(p, *default_plane, &alpha)) return false;
		if (alpha < 0) return false;
		q = p - alpha * default_plane->h;
		if (ret) *ret = q;
		return true;
	case 1:
		plane = associated_facetvertex[0]->get_facet()->get_plane();
		if (!interp(p, plane, &alpha)) return false;
		q = p - alpha * plane.h;
		if (ret) *ret = q;
		return true;
	case 2:
		if (!interp(associated_facetvertex[0]->get_facet()->get_plane(), associated_facetvertex[1]->get_facet()->get_plane(), &l, 0, 0)) return false;
		if (!interp(p, l, &alpha)) return false;
		//cout << alpha << ',' << p.x << ',' << p.y << ',' << p.z << "THROW " << l.p.x << ',' << l.p.y << ',' << l.p.z << " V" << l.v.x << ',' << l.v.y << ',' << l.v.z << endl;
		q = l.p + alpha * l.v;
		if (ret) *ret = q;
		return true;
	default:;
	}
	return false;
}

bool Vertex::get_feasible_point_nearest_to_ray(const Line3D& ray, Point3D* ret, const Plane* default_plane) const {
	Point3D p;
	Plane plane;
	scalar_t alpha;
	scalar_t beta;
	Line3D l;
	switch (associated_facetvertex.size()) {
	case 0:
		if (!default_plane) return false;
		if (!interp(ray, *default_plane, &alpha)) return false;
		if (alpha < 0) return false;
		p = ray.p + alpha * ray.v;
		if( ret ) *ret = p;
		return true;
	case 1:
		if (!interp(ray, associated_facetvertex[0]->get_facet()->get_plane(), &alpha)) return false;
		if (alpha < 0) return false;
		p = ray.p + alpha * ray.v;
		if (ret) *ret = p;
		return true;
	case 2:
		if (!interp(associated_facetvertex[0]->get_facet()->get_plane(), associated_facetvertex[1]->get_facet()->get_plane(), &l, 0, 0)) return false;
		if (!interp(ray, l, &alpha, &beta)) return false;
		if (alpha < 0) return false;
		p = l.p + beta * l.v;
		if(ret) *ret = p;
		return true;
	default:;
	}
	return false;
}

bool Vertex::join(Vertex *v) {
	if (associated_facetvertex.size() == 0) {
		this->move_to(v->to_point());
		return true;
	}
	if (v->associated_facetvertex.size() == 0) {
		v->move_to(this->to_point());
		return true;
	}
	if (associated_facetvertex.size() >= 3 || v->associated_facetvertex.size() >= 3) {
		return false;
	}

	if (associated_facetvertex.size() == 1) {
		if (v->associated_facetvertex.size() == 1) {
			// plane vs plane
			Plane plane1 = this->associated_facetvertex[0]->facet->get_plane();
			Plane plane2 = v->associated_facetvertex[0]->facet->get_plane();

			Line3D l;
			if (!interp(plane1, plane2, &l, 0, 0)) return false;
			//DO something;
			Point3D x_p, y_p;
			scalar_t alpha;
			if (!interp(this->to_point(), l, &alpha)) return false;
			x_p = l.p + l.v * alpha;
			if (!interp(v->to_point(), l, &alpha)) return false;
			y_p = l.p + l.v * alpha;
			Point3D c = ((x_p.to_vector() + y_p.to_vector()) / 2.0).to_point();
			v->move_to(c);
			this->move_to(c);

			return true;
		}
		else {
			// plane vs line
			Plane plane1 = this->associated_facetvertex[0]->facet->get_plane();
			Plane plane2a = v->associated_facetvertex[0]->facet->get_plane();
			Plane plane2b = v->associated_facetvertex[1]->facet->get_plane();
			Line3D l;
			if (!interp(plane2a, plane2b, &l, 0, 0)) return false;
			scalar_t alpha;
			if (!interp(l, plane1, &alpha)) return false;
			Point3D x = l.p + l.v * alpha;
			v->move_to(x);
			this->move_to(x);
			return true;
		}
	}
	if (associated_facetvertex.size() == 2) {
		if (v->associated_facetvertex.size() == 1) {
			// line vs plane
			return v->join(this);
		}
		else {
			// line vs line
			Plane plane1a = this->associated_facetvertex[0]->facet->get_plane();
			Plane plane1b = this->associated_facetvertex[1]->facet->get_plane();

			Plane plane2a = v->associated_facetvertex[0]->facet->get_plane();
			Plane plane2b = v->associated_facetvertex[1]->facet->get_plane();

			Line3D l1, l2;
			if (!interp(plane1a, plane1b, &l1, 0, 0)) return false;
			if (!interp(plane2a, plane2b, &l2, 0, 0)) return false;

			scalar_t alpha, beta;
			if (!interp(l1, l2, &alpha, &beta)) return false;

			Point3D x = l1.p + l1.v * alpha;
			Point3D y = l2.p + l2.v * alpha;

			if (!((y - x).is_zero())) return false;

			this->move_to(x);
			v->move_to(y);

			return true;
		}
	}
	
	return false;
}

bool Vertex::move_to_ray(const Line3D& ray, const Plane* default_plane) {
	Point3D p;
	if (!get_feasible_point_nearest_to_ray(ray, &p, default_plane)) return false;
	return move_to(p);
}

void Vertex::triangulate_associated_facets(void) {
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		associated_facetvertex[i]->get_facet()->triangulate();
	}
}
Point3D Vertex::to_point(void) const { return Point3D(x, y, z); }
Vector3D Vertex::to_vector(void) const { return Vector3D(x, y, z); }

void Vertex::draw(length_t radius, DrawSetting* drawsetting) {
	if (drawsetting && !drawsetting->begin_vertex(this)) return;
	if (radius < 0) return;
	if (radius > 0) {
		draw_sphere(x, y, z, radius);
	}
	else {
		glBegin(GL_POINTS);
		glVertex3f(x, y, z);
		glEnd();
	}
}

bool Vertex::absorb(Vertex* v) {
	if (this == v) return false;
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (associated_facetvertex[i]->facet == v->associated_facetvertex[j]->facet) {
				//if (associated_facetvertex[i]->next() == v->associated_facetvertex[j]) {
				//	return associated_facetvertex[i]->facet->remove_vertex(v->associated_facetvertex[j]);
				//}
				return false;
			}
		}
	}
	//if (associated_facetvertex.size() + v->associated_facetvertex.size() > 3) return false;

	for (int i = 0; i < v->associated_facetvertex.size(); ++i) {
		//cout << "ADD" << endl;
		associated_facetvertex.push_back(v->associated_facetvertex[i]);
		v->associated_facetvertex[i]->associated_vertex = this;
	}
	return true;
}
bool Vertex::remove(void) {
	bool flag = true;
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		Facet *f = associated_facetvertex[i]->facet;
		if (f->remove_vertex(associated_facetvertex[i])) {
			--i;
			f->triangulate();
		} else {
			flag = false;
		}
	}
	return flag;
}

Facet* Vertex::get_a_shared_facet(const Vertex *v) const {
	if ((v == 0 || v == this) && !associated_facetvertex.empty()) {
		return associated_facetvertex[0]->facet;
	}
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (associated_facetvertex[i]->facet == v->associated_facetvertex[j]->facet) return associated_facetvertex[i]->facet;
		}
	}
	return 0;
}
int Vertex::num_shared_facets(const Vertex *v) const {
	if (!v) return associated_facetvertex.size();
	int cnt = 0;
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (associated_facetvertex[i]->facet == v->associated_facetvertex[j]->facet) ++cnt;
		}
	}
	return cnt;
}
Facet* Vertex::get_facet(int i) const {
	return associated_facetvertex[i]->facet;
}
bool Vertex::share_a_facet(const Vertex *v) const {
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (associated_facetvertex[i]->facet == v->associated_facetvertex[j]->facet) return true;
		}
	}
	return false;
}
bool Vertex::form_an_edge(const Vertex *v) const {
	for (int i = 0; i < associated_facetvertex.size(); ++i) {
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (associated_facetvertex[i]->next() == v->associated_facetvertex[j]) return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
FacetVertex::FacetVertex(Facet* _f, Vertex* _v)
	:facet(_f),associated_vertex(_v),incoming(0),outgoing(0) 
{

}

FacetVertex* FacetVertex::prev(void) const {
	if (!incoming) return 0;
	return incoming->u;
}

FacetVertex* FacetVertex::next(void) const {
	if (!outgoing) return 0;
	return outgoing->v;
}

Facet* FacetVertex::get_facet(void) {
	return facet;
}

Vertex* FacetVertex::get_vertex(void) {
	return associated_vertex;
}

Point3D FacetVertex::to_point3(void) const {
	return Point3D(associated_vertex->to_point());
}

FacetEdge *FacetVertex::incoming_edge(void) const { return incoming;  }
FacetEdge *FacetVertex::outgoing_edge(void) const { return outgoing;  }

FacetEdge::FacetEdge(Facet* _f, FacetVertex* _u, FacetVertex* _v)
	: facet(_f), u(_u), v(_v)
{
		u->outgoing = this;
		v->incoming = this;
}

FacetEdge* FacetEdge::next(void) const {
	return v->outgoing;
}

FacetEdge* FacetEdge::prev(void) const {
	return u->incoming;
}

Vector3D FacetEdge::to_vector3(void) const {
	return v->to_point3() - u->to_point3();
}

Vector3D FacetEdge::compute_ring_normal_vector(void) const {
	const FacetEdge* e = this;
	Vector3D ret(0, 0, 0);
	do {
		const FacetEdge* e_next = e->next();
		if (!e_next) return ret;
		ret = ret + e->to_vector3().cross_product(e_next->to_vector3());
		e = e_next;
	} while (e != this);
	return ret;
}

bool FacetEdge::is_shared(void) const {
	return (u->associated_vertex->num_shared_facets(v->associated_vertex) > 1);
}

FacetVertex *FacetEdge::get_u() { return u; }
FacetVertex *FacetEdge::get_v() { return v; }
Facet *FacetEdge::get_f() { return facet; }

Facet::Facet()
	: gl_polygon_id(-1)
{
	__temp_texture=0; }

void Facet::release(std::vector<Vertex*> *pts) { 
	// CALLLIST
	if (gl_polygon_id != -1) {
		glDeleteLists(gl_polygon_id, 1);
		gl_polygon_id = -1;
	}

	for (int i = 0; i < (int)vertex.size(); ++i) {
		Vertex *v = vertex[i]->associated_vertex;
		detatch_a_vertex(vertex[i]);
		delete vertex[i];
		if (v && v->associated_facetvertex.empty()) {
			for (int j = 0; j < pts->size(); ++j) {
				if (v == (*pts)[j]) {
					delete v;
					pts->erase(pts->begin() + j);
					break;
				}
			}
		}
	}
	vertex.clear();
	for (int i = 0; i < (int)edge.size(); ++i) {
		delete edge[i];
	}
	edge.clear();
	exterior = 0;
}

void Facet::draw(DrawSetting* drawsetting, int i) {
	if (drawsetting) drawsetting->begin_facets();
	if (!drawsetting || drawsetting->begin_facet(i,this)) {
		if (gl_polygon_id != -1) {
			glCallList(gl_polygon_id);
		}
	}
}
void Facet::draw_edge(DrawSetting* drawsetting) {
	if (drawsetting) drawsetting->begin_edges();
	for (int i = 0; i < edge.size(); ++i) {
		if (!drawsetting || drawsetting->begin_facetedge(i,edge[i])) {
			Point3D u = edge[i]->u->to_point3();
			Point3D v = edge[i]->v->to_point3();
			glBegin(GL_LINES);
			glVertex3f(u.x, u.y, u.z);
			glVertex3f(v.x, v.y, v.z);
			glEnd();
		}
	}
}

void Facet::draw_vertex(DrawSetting* drawsetting) {
	if (drawsetting) drawsetting->begin_vertices();
	for (int i = 0; i < vertex.size(); ++i) {
		Point3D v = vertex[i]->to_point3();
		if (!drawsetting || drawsetting->begin_facetvertex(i,vertex[i])) {
			glBegin(GL_POINTS);
			glVertex3f(v.x, v.y, v.z);
			glEnd();
		}
	}
}

int Facet::winding_number(const Point3D& p) const {
	if (edge.empty()) return 0;
	Point3D p_proj = plane.project(p);
	Point3D u = edge[0]->u->to_point3();
	Point3D v = edge[0]->v->to_point3();
	Point3D c = u + 0.5*(v - u);

	Line3D ray(p_proj, c - p_proj);
	if (ray.v.is_zero()) return 1;

	int w = 0;
	for (int i = 0; i < edge.size(); ++i) {
		Line3D e(edge[i]->u->to_point3(), edge[i]->to_vector3());
		float alpha, beta;
		if (interp(ray, e, &alpha, &beta)) {
			if (alpha < 0) continue;
			if (beta < 0 || beta > 1) continue;

			float h = ray.v.cross_product(e.v).dot_product(plane.h);
			if (h < 0) --w;
			else if (h > 0) ++w;
		}
	}
	return w;
}

static __temp_texture *__current_coord = 0;
static void __stdcall vertex_callback(const GLvoid *data) {
	Point3D v = ((const FacetVertex*)data)->to_point3();
	if (__current_coord) {
		Point2D uv = __current_coord->convert(v);

		glTexCoord2f(uv.x, uv.y);
	}
	glVertex3f(v.x, v.y, v.z);
}

void Facet::triangulate(void) {
	if (gl_polygon_id != -1) {
		glDeleteLists(gl_polygon_id, 1);
	}
	gl_polygon_id = glGenLists(1);
	glNewList(gl_polygon_id, GL_COMPILE);

	__current_coord = 0;
	if (this->__temp_texture) {
		__current_coord = this->__temp_texture;
	}

	GLUtesselator *tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)(void))glBegin);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall*)(void))glEnd);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)(void))vertex_callback);
	
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);
	{
		FacetEdge* e_start = exterior;
		FacetEdge* e = e_start;
		do {
			FacetVertex *vertex2 = e->u;
			Point3D v = vertex2->to_point3();
			GLdouble p[3] = { v.x, v.y, v.z };
			gluTessVertex(tess, p, (void*)vertex2);

			e = e->next();
		} while (e != e_start);
	}
	gluTessEndContour(tess);

	for (int i = 0; i < holes.size(); ++i) {
		gluTessBeginContour(tess);
		{
			FacetEdge* e_start = holes[i];
			FacetEdge* e = e_start;
			do {
				FacetVertex *vertex2 = e->u;
				Point3D v = vertex2->to_point3();
				GLdouble p[3] = { v.x, v.y, v.z };
				gluTessVertex(tess, p, (void*)vertex2);
				e = e->next();
			} while (e != e_start);
		}
		gluTessEndContour(tess);
	}
	
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);
	glEndList();
}


Facet* Facet::create_facet(Vertex *v1, Vertex *v2, Vertex *v3) {
	/*if (v1->associated_facetvertex.size() >= 3 ||
		v2->associated_facetvertex.size() >= 3 ||
		v3->associated_facetvertex.size() >= 3) return 0;
		*/
	Facet *f = new Facet();
	FacetVertex *u1 = f->create_vertex(v1);
	FacetVertex *u2 = f->create_vertex(v2);
	FacetVertex *u3 = f->create_vertex(v3);

	FacetEdge *e1 = f->create_edge(u1, u2);
	FacetEdge *e2 = f->create_edge(u2, u3);
	FacetEdge *e3 = f->create_edge(u3, u1);
	
	f->exterior = e1;
	f->plane.p = Point3D(((v1->to_vector() + v2->to_vector() + v3->to_vector()) / 3).to_point());
	f->plane.h = (v2->to_point() - v1->to_point()).cross_product(v3->to_point() - v1->to_point()).normalized();
	
	return f;
}

Facet* Facet::create_facet(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4) {
	/*if (v1->associated_facetvertex.size() >= 3 ||
		v2->associated_facetvertex.size() >= 3 ||
		v3->associated_facetvertex.size() >= 3 ||
		v4->associated_facetvertex.size() >= 3 ) return 0;
		*/
	Facet *f = new Facet();
	FacetVertex *u1 = f->create_vertex(v1);
	FacetVertex *u2 = f->create_vertex(v2);
	FacetVertex *u3 = f->create_vertex(v3);
	FacetVertex *u4 = f->create_vertex(v4);

	FacetEdge *e1 = f->create_edge(u1, u2);
	FacetEdge *e2 = f->create_edge(u2, u3);
	FacetEdge *e3 = f->create_edge(u3, u4);
	FacetEdge *e4 = f->create_edge(u4, u1);

	f->exterior = e1;
	f->plane.p = Point3D(((v1->to_vector() + v2->to_vector() + v3->to_vector() + v4->to_vector()) / 4).to_point());
	f->plane.h = (v2->to_point() - v1->to_point()).cross_product(v3->to_point() - v1->to_point()).normalized();

	return f;
}

Facet* Facet::create_facet(const vector<Vertex *> &vs) {
	//if (vs.size() < 4) return 0;
	for (int i = 0; i < vs.size(); ++i) {
		//if (vs[i]->associated_facetvertex.size() >= 3) return 0;
	}

	Facet *f = new Facet();
	vector<FacetVertex *> fvs;
	Vector3D center(0, 0, 0);
	for (int i = 0; i < vs.size(); ++i) {
		FacetVertex *fv = f->create_vertex(vs[i]);
		if (!fv) {
			cerr << " NULL VERTEX" << endl;
		}
		fvs.push_back(fv);
		center = center + fvs[i]->to_point3().to_vector();
	}
	center = center / fvs.size();
	vector<FacetEdge*> es;
	for (int i = 0; i < fvs.size(); ++i) {
		es.push_back(f->create_edge(fvs[i], fvs[(i + 1) % fvs.size()]));
	}
	
	f->exterior = es[0];
	f->plane.p = center.to_point();

	Vector3D normal(0, 0, 0);
	for (int i = 0; i < vs.size(); ++i) {
		normal = normal + (vs[(i) % vs.size()]->to_point() - vs[0]->to_point()).cross_product(
			vs[(i + 1) % vs.size()]->to_point() - vs[0]->to_point());
	}
	//cout << normal.length() << endl;
	f->plane.h = normal.normalized();
	
	return f;
}

void Facet::make_hole(const std::vector<Vertex*>& vs) {
	if (vs.empty()) return;

	vector<FacetVertex *> fvs;
	for (int i = 0; i < vs.size(); ++i) {
		FacetVertex *fv = create_vertex(vs[i]); 
		fvs.push_back(fv);
	}
	vector<FacetEdge*> es;
	for (int i = 0; i < fvs.size(); ++i) {
		es.push_back(create_edge(fvs[i], fvs[(i + 1) % fvs.size()]));
	}

	holes.push_back(es[0]);
}

FacetVertex* Facet::create_vertex(Vertex *v) {
	//if (v->associated_facetvertex.size() >= 3) return get_vertex(v);
	//FacetVertex *u_old = get_vertex(v);
	//if (u_old) return u_old;
	FacetVertex *u = new FacetVertex(this, v);
	v->associated_facetvertex.push_back(u);
	vertex.push_back(u);
	return u;
}

FacetEdge* Facet::create_edge(FacetVertex *u, FacetVertex *v) {
	FacetEdge *e = new FacetEdge(this, u, v);
	edge.push_back(e);
	return e;
}

FacetEdge* Facet::get_edge(int i) const {
	return edge[i];
}


FacetVertex* Facet::get_vertex(int i) const {
	return vertex[i];
}

FacetVertex* Facet::get_vertex(Vertex *v) const {
	if (!v) return 0;
	for (int i = 0; i < v->associated_facetvertex.size(); ++i) {
		if (v->associated_facetvertex[i]->facet == this) {
			return v->associated_facetvertex[i];
		}
	}
	return 0;
}

FacetEdge* Facet::get_exteior_edge(void) const { return exterior; }
FacetEdge* Facet::get_hole_edge(int i) const { return holes[i]; }
int Facet::num_edges(void) const { return edge.size(); }
int Facet::num_holes(void) const { return holes.size(); }
int Facet::num_vertices(void) const { return vertex.size(); }
int Facet::num_shared_edge(Facet *f) {
	int n = 0;
	for (int i = 0; i < edge.size(); ++i) {
		for (int j = 0; j < f->edge.size(); ++j) {
			if (edge[i]->get_u()->associated_vertex == f->edge[j]->get_v()->associated_vertex
				&& edge[i]->get_v()->associated_vertex == f->edge[j]->get_u()->associated_vertex ) {
				++n;
			}
		}
	}
	return n;
}

// Actions



bool Facet::has_no_adjacent_polygons(void) const {
	for (int i = 0; i < vertex.size(); ++i) {
		if (vertex[i]->associated_vertex->associated_facetvertex.size() != 1) return false;
	}
	return true;
}

void Facet::make_solid(const Vector3D& h, std::vector<Vertex*>* rv, std::vector<Facet*>* rf) {
	if (h.is_zero()) return;

	map<FacetVertex*, Vertex*> new_vs;
	for (int i = 0; i < vertex.size(); ++i) {
		Point3D p = vertex[i]->to_point3() + h;
		Vertex *v = new Vertex(p.x, p.y, p.z);
		new_vs[vertex[i]] = v;
		if(rv) rv->push_back(v);
	}

	{
		
		Facet *f_ceiling = new Facet();
		f_ceiling->plane.p = plane.p + h;
		f_ceiling->plane.h = -1 * plane.h;
		{
			FacetEdge *e = exterior;
			FacetVertex *lv = 0;
			do {
				Vertex *u = new_vs.at(e->u);

				FacetVertex *v1 = f_ceiling->create_vertex(u);
				//cout << v1 << endl;
				if (lv) {
					f_ceiling->create_edge(lv, v1);
				}
				lv = v1;

				e = e->prev();
			} while (e != exterior);
			if (lv) {
				f_ceiling->exterior = f_ceiling->create_edge(lv, f_ceiling->get_vertex(new_vs.at(e->u)));
				//cout << f_ceiling->exterior << endl;
			}

		}

		for (int i = 0; i < holes.size(); ++i) {
			FacetEdge *e = holes[i];
			FacetVertex *lv = 0;
			do {
				Vertex *u = new_vs.at(e->u);

				FacetVertex *v1 = f_ceiling->create_vertex(u);
				if (lv) {
					FacetEdge *e_new = f_ceiling->create_edge(lv, v1);
				}
				lv = v1;

				e = e->prev();
			} while (e != holes[i]); 
			if (lv) {
				f_ceiling->holes.push_back(f_ceiling->create_edge(lv, f_ceiling->get_vertex(new_vs.at(e->u))));
			}
		}
		f_ceiling->triangulate();
		if (rf) rf->push_back(f_ceiling);
	}


	

	{
		FacetEdge *e = exterior;
		do {
			Facet *f = new Facet();
			Vertex *u = new_vs.at(e->u);
			Vertex *v = new_vs.at(e->v);

			f->plane.p = ((u->to_vector() + v->to_vector() - h) / 2).to_point();
			f->plane.h = plane.h.cross_product(e->to_vector3()).normalized();
			
			FacetVertex *v1 = f->create_vertex(e->v->associated_vertex);
			FacetVertex *v2 = f->create_vertex(e->u->associated_vertex);
			FacetVertex *v3 = f->create_vertex(u);
			FacetVertex *v4 = f->create_vertex(v);

			f->exterior = f->create_edge(v1, v2);
			f->create_edge(v2, v3);
			f->create_edge(v3, v4);
			f->create_edge(v4, v1);

			f->triangulate();

			if (rf) rf->push_back(f);

			e = e->next();
		} while (e != exterior);
	}

	for (int i = 0; i < holes.size(); ++i) {
		FacetEdge *e = holes[i];
		do {
			Facet *f = new Facet();
			Vertex *u = new_vs.at(e->u);
			Vertex *v = new_vs.at(e->v);

			f->plane.p = ((u->to_vector() + v->to_vector() - h) / 2).to_point();
			f->plane.h = plane.h.cross_product(e->to_vector3()).normalized();

			FacetVertex *v1 = f->create_vertex(e->v->associated_vertex);
			FacetVertex *v2 = f->create_vertex(e->u->associated_vertex);
			FacetVertex *v3 = f->create_vertex(u);
			FacetVertex *v4 = f->create_vertex(v);

			f->exterior = f->create_edge(v1, v2);
			f->create_edge(v2, v3);
			f->create_edge(v3, v4);
			f->create_edge(v4, v1);

			f->triangulate();

			if (rf) rf->push_back(f);

			e = e->next();
		} while (e != holes[i]);
	}
	
}


bool Facet::split_by_edge(FacetVertex *u, FacetVertex *v, std::vector<Facet*>* rf) {
	if (u->facet != this || v->facet != this) return false;
	if (u->next() == v || v->next() == u) return false;
	//if (u->associated_vertex->associated_facetvertex.size() >= 3) return false;
	//if (v->associated_vertex->associated_facetvertex.size() >= 3) return false;
	if (!holes.empty()) return false;
	
	vector<FacetVertex*> other_side;
	FacetVertex *x = v;
	while (x != u) {
		other_side.push_back(x);
		x = x->next();
	}

	vector<Vertex*> vs;
	for (int i = 1; i < other_side.size(); ++i) {
		vs.push_back(other_side[i]->associated_vertex);
		this->remove_vertex(other_side[i]);
	}
	this->triangulate();

	Facet *new_f = new Facet();
	new_f->plane = plane;
	FacetVertex *new_u = new_f->create_vertex(u->associated_vertex);
	FacetVertex *new_v = new_f->create_vertex(v->associated_vertex);
	FacetVertex *last_v = new_v;
	for (int i = 0; i < vs.size(); ++i) {
		FacetVertex *new_w = new_f->create_vertex(vs[i]);
		new_f->create_edge(last_v, new_w);
		last_v = new_w;
	}
	new_f->create_edge(last_v, new_u);
	new_f->exterior = new_f->create_edge(new_u, new_v);
	new_f->triangulate();
	if (rf) rf->push_back(new_f);
	return true;
}

void Facet::reverse(void) {

	for (int i = 0; i < vertex.size(); ++i) {
		swap(vertex[i]->incoming, vertex[i]->outgoing);
	}
	for (int i = 0; i < edge.size(); ++i) {
		swap(edge[i]->u, edge[i]->v);
	}
	plane.h = -1 * plane.h;
	this->triangulate();
}

Facet* Facet::merge(Facet* fa, Facet* fb, vector<Vertex*>* rv, vector<Facet*>* rf) {
	if (fb->plane.h.dot_product(fa->plane.h) < 0.7) {
		cerr << "NOT CO-PLANAR: " << fb->plane.h.dot_product(fa->plane.h) << endl;
		return 0;
	}

	set<FacetEdge*> shared_edge;
	for (int i = 0; i < fa->edge.size(); ++i) {
		FacetEdge *e = fa->edge[i];
		Vertex* v = e->v->associated_vertex;
		FacetVertex *fbv = fb->get_vertex(v);
		if (fbv && fbv->next()->associated_vertex == e->u->associated_vertex) {
			shared_edge.insert(e);
		}
	}
	for (int i = 0; i < fb->edge.size(); ++i) {
		FacetEdge *e = fb->edge[i];
		Vertex* v = e->v->associated_vertex;
		FacetVertex *fav = fa->get_vertex(v);
		if (fav && fav->next()->associated_vertex == e->u->associated_vertex) {
			shared_edge.insert(e);
		}
	}

	vector<vector<Vertex*> > rings;
	set<FacetEdge*> visited;
	for (int i = 0; i < fa->edge.size(); ++i) {
		FacetEdge *e = fa->edge[i];
		if (shared_edge.count(e) || visited.count(e)) continue;

		FacetEdge *e_start = e;
		vector<Vertex*> ring;
		do {
			if (visited.count(e)) return 0;
			visited.insert(e);
			ring.push_back(e->u->associated_vertex);
			FacetEdge *e_next = e->next();
			if (shared_edge.count(e_next)) {
				if (e->facet == fa) {
					e_next = fb->get_vertex(e->v->associated_vertex)->outgoing;
				}
				else if(e->facet == fb) {
					e_next = fa->get_vertex(e->v->associated_vertex)->outgoing;
				}
			}
			e = e_next;
		} while (e != e_start);

		rings.push_back(ring);
	}
	for (int i = 0; i < fb->edge.size(); ++i) {
		FacetEdge *e = fb->edge[i];
		if (shared_edge.count(e) || visited.count(e)) continue;

		FacetEdge *e_start = e;
		vector<Vertex*> ring;
		do {
			if (visited.count(e)) return 0;
			visited.insert(e);
			ring.push_back(e->u->associated_vertex);
			FacetEdge *e_next = e->next();
			if (shared_edge.count(e_next)) {
				if (e->facet == fa) {
					e_next = fb->get_vertex(e->v->associated_vertex)->outgoing;
				}
				else if (e->facet == fb) {
					e_next = fa->get_vertex(e->v->associated_vertex)->outgoing;
				}
			}
			e = e_next;
		} while (e != e_start);

		rings.push_back(ring);
	}
	
	Vector3D h = ((fa->plane.h + fb->plane.h) / 2).normalized();
	int exterior_i = -1;
	for (int i = 0; i < rings.size(); ++i) {
		Vector3D vh(0, 0, 0);
		for (int j = 0; j < rings[i].size(); ++j) {
			int j1 = (j + 1) % rings[i].size();
			int j2 = (j + 2) % rings[i].size();
			vh = vh + (rings[i][j1]->to_point() - rings[i][0]->to_point()).cross_product(rings[i][j2]->to_point() - rings[i][0]->to_point());
		}
		if (h.dot_product(vh) > 0) {
			exterior_i = i;
			break;
		}
	}

	//detatch vertices
	set<Vertex*> all_v;
	for (int i = 0; i < fa->vertex.size(); ++i ) {
		Vertex *v = fa->vertex[i]->associated_vertex;
		all_v.insert(v);
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (v->associated_facetvertex[j] == fa->vertex[i]) {
				v->associated_facetvertex.erase(v->associated_facetvertex.begin() + j);
				break;
			}
		}
		fa->vertex[i]->associated_vertex = 0;
	}
	for (int i = 0; i < fb->vertex.size(); ++i) {
		Vertex *v = fb->vertex[i]->associated_vertex;
		all_v.insert(v);
		for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
			if (v->associated_facetvertex[j] == fb->vertex[i]) {
				v->associated_facetvertex.erase(v->associated_facetvertex.begin() + j);
				break;
			}
		}
		fb->vertex[i]->associated_vertex = 0;
	}

	//make a new facet
	Facet *new_f = new Facet();
	Vector3D acc(0, 0, 0);
	for (auto i = all_v.begin(); i != all_v.end(); ++i) {
		acc = acc + (*i)->to_vector();
	}
	acc = acc / (all_v.size());
	new_f->plane.h = h;
	new_f->plane.p = acc.to_point();

	if (rf) {
		for (int i = 0; i < rf->size(); ++i) {
			if ((*rf)[i] == fa || (*rf)[i] == fb) {
				rf->erase(rf->begin() + i);
				--i;
			}
		}
	}
	fa->release();
	fb->release();
	delete fa;
	delete fb;
	
	{
		FacetVertex *last_v = 0;
		for (int j = 0; j < rings[exterior_i].size(); ++j) {
			FacetVertex *v = new_f->create_vertex(rings[exterior_i][j]);
			all_v.erase(rings[exterior_i][j]);
			if (last_v) {
				new_f->create_edge(last_v, v);
			}
			last_v = v;
		}
		new_f->exterior = new_f->create_edge(last_v, new_f->get_vertex(rings[exterior_i][0]));
	}
	
	for (int i = 0; i < rings.size(); ++i) {
		FacetVertex *last_v = 0;
		if (i == exterior_i) continue;
		for (int j = 0; j < rings[i].size(); ++j) {
			FacetVertex *v = new_f->create_vertex(rings[i][j]);
			all_v.erase(rings[i][j]);
			if (last_v) {
				new_f->create_edge(last_v, v);
			}
			last_v = v;
		}
		new_f->holes.push_back(new_f->create_edge(last_v, new_f->get_vertex(rings[i][0])));
	}
	//new_f->planarize();
	new_f->triangulate();
	if (rf) rf->push_back(new_f);
	if (rv) {
		for (int i = 0; i < rv->size(); ++i) {
			Vertex *v = (*rv)[i];
			if (all_v.count(v) && v->associated_facetvertex.empty()) {
				delete v;
				rv->erase(rv->begin() + i);
				--i;
			}
		}
	}

	return new_f;
}



bool Facet::remove_vertex(FacetVertex *v) {
	if (!v) return false;
	return remove_edge(v->incoming);
}
bool Facet::remove_edge(int i) {
	if (edge.size() == 2) return false;
	if (i >= edge.size()) return false;
	FacetEdge *e = edge[i];
	FacetEdge *e_next = e->next();
	e_next->u = e->u;
	e->u->outgoing = e_next;
	
	for (int i = 0; i < e->v->associated_vertex->associated_facetvertex.size(); ++i) {
		if (e->v->associated_vertex->associated_facetvertex[i] == e->v) {
			e->v->associated_vertex->associated_facetvertex.erase(e->v->associated_vertex->associated_facetvertex.begin() + i);
			break;
		}
	}
	
	if (e == exterior) exterior = e_next;
	else for (int i = 0; i < holes.size(); ++i) {
		if (e == holes[i]) {
			holes[i] = e_next;
			break;
		}
	}


	for (int i = 0; i < vertex.size(); ++i) {
		if (vertex[i] == e->v) {
			vertex.erase(vertex.begin() + i);
			break;
		}
	}
	delete e->v;
	delete e;
	edge.erase(edge.begin() + i);
	return true;
}
bool Facet::remove_edge(FacetEdge *e) {
	for (int i = 0; i < edge.size(); ++i) {
		//cerr << "CHECK " << i << '\t' << edge[i] << '\t' << e << endl;
		if( e == edge[i] ) return remove_edge(i);
	}
	return false;
}

bool Facet::remove_hole(int idx) {
	if ( idx < 0 || holes.size() <= idx) return false;
	FacetEdge *e_start = holes[idx];
	FacetEdge *e = e_start;
	do {
		FacetEdge *e_next = e->next();
		detatch_a_vertex(e->v);
		for (int i = 0; i < vertex.size(); ++i) {
			if (vertex[i] == e->v) {
				vertex.erase(vertex.begin() + i);
				break;
			}
		}
		for (int i = 0; i < edge.size(); ++i) {
			if (edge[i] == e) {
				edge.erase(edge.begin() + i);
				break;
			}
		}
		delete e->v;
		delete e;
		e = e_next;
	} while (e_start != e);
	holes.erase(holes.begin() + idx);
	triangulate();
	return true;
}

bool Facet::split_edge(int i, Vertex *v3) {
	if (i >= edge.size()) return false;
	return split_edge(edge[i], v3);
}
bool Facet::split_edge(FacetEdge* e, Vertex *v3) {
	//if (v3->associated_facetvertex.size() >= 3)  return false;
	for (int i = 0; i < v3->associated_facetvertex.size(); ++i) {
		if (v3->associated_facetvertex[i]->facet == this) return false;
	}

	FacetVertex* u = create_vertex(v3);
	u->incoming = e;

	FacetVertex* v = e->v;
	v->incoming = 0;
	e->v = u;

	create_edge(u, v);
	return true;
}

void Facet::set_plane(const Plane& p) {
	plane = p;
}

Plane Facet::get_plane(void) const {
	return plane;
}

bool Facet::planarize(void) {
	bool failed = false;
	set<Facet*> adj_facets;
	for (int i = 0; i < vertex.size(); ++i) {
		Vertex *v = vertex[i]->associated_vertex;
		//cout << v->associated_facetvertex.size() << endl;
		if (v->associated_facetvertex.size() == 1) {
			scalar_t alpha;
			Point3D p = v->to_point();
			if (!interp(p, plane, &alpha)) {
				failed = true;
				continue;
			}

			p = p - alpha * plane.h;
			v->move_to(p);
		}
		else if (v->associated_facetvertex.size() == 2) {
			Plane p[1];
			{
				int k = 0;
				for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
					if (v->associated_facetvertex[j]->facet == this) continue;
					p[k++] = v->associated_facetvertex[j]->facet->plane;
					adj_facets.insert(v->associated_facetvertex[j]->facet);
				}
			}
			Line3D l;
			if (!interp(p[0], plane, &l, 0, 0)) {
				failed = true;
				continue;
			}
			Point3D x = v->to_point();
			scalar_t alpha;
			if (!interp(x, l, &alpha)) {
				failed = true;
				continue;
			}
			x = l.p + alpha * l.v;
			v->move_to(x);
		}
		else if (v->associated_facetvertex.size() == 3) {
			Plane p[2];
			{
				int k = 0;
				for (int j = 0; j < v->associated_facetvertex.size(); ++j) {
					if (v->associated_facetvertex[j]->facet == this) continue;
					p[k++] = v->associated_facetvertex[j]->facet->plane;
					adj_facets.insert(v->associated_facetvertex[j]->facet);
				}
			}
			Line3D l;
			if (!interp(p[0], p[1], &l, 0, 0)) {
				failed = true;
				continue;
			}
			//cerr << "LEN" << l.v.length() << endl;
			scalar_t alpha;
			if (!interp(l, plane, &alpha)) {
				failed = true;
				continue;
			}

			//cerr << "ALPHA" << alpha << endl;
			//cerr << "P" << l.p.x << ',' << l.p.y << ',' << l.p.z << endl;
			//cerr << "V" << l.v.x << ',' << l.v.y << ',' << l.v.z << endl;
			Point3D x = l.p + alpha * l.v;
			//cerr << "X" << x.x << ',' << x.y << ',' << x.z << endl;
			//x = v->to_point();
			//cerr << "X" << x.x << ',' << x.y << ',' << x.z << endl;
			//cerr << "D" << p[0].h.dot_product(plane.h) << ',' << p[1].h.dot_product(plane.h)  << endl;
			//cerr << v->x << ',' << v->y << ',' << v->z << " ==> " << x.x << ',' << x.y << ',' << x.z << endl;
			v->move_to(x);
		}
		else {
			failed = true;
		}
	}
	for (auto i = adj_facets.begin(); i != adj_facets.end(); ++i) {
		(*i)->triangulate();
	}
	this->triangulate();
	return !failed;
}

void Facet::detatch_a_vertex(FacetVertex *v) {
	if (!v) return;
	if (v->facet != this) return;
	if (!v->associated_vertex) return;
	for (int i = 0; i < v->associated_vertex->associated_facetvertex.size(); ++i) {
		if (v->associated_vertex->associated_facetvertex[i] == v) {
			v->associated_vertex->associated_facetvertex.erase(v->associated_vertex->associated_facetvertex.begin() + i);
			v->associated_vertex = 0;
			break;
		}
	}
}
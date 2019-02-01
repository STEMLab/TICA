#include "PlanarFacet.h"
#include "BinTexture.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static BinTexture* Texture_info = 0;
static Plane current_plane;
static std::vector<Point2D> current_triangulation_buffer;

static void __stdcall tessBeginCB(GLenum which) {
	glBegin(which);
}

static void __stdcall tessVertexCB(const GLvoid *data)
{
	int i = (int)data;
	Point2D p2d = current_triangulation_buffer[i];
	Point3D p = current_plane.convert(p2d);
	if (Texture_info) {
		double u, v;
		Texture_info->convert(p2d.x, p2d.y, &u, &v);
		glTexCoord2f(u, v);
	}
	glVertex3f(p.x, p.y, p.z);
}


static void __stdcall tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
	const GLfloat neighborWeight[4], GLdouble **outData) {

	Point2D p(newVertex[0], newVertex[1]);
	current_triangulation_buffer.push_back(p);

	*(int*)outData = (int)(current_triangulation_buffer.size() - 1);
}

static void __stdcall tessErrorCB(GLenum errorCode)
{
	const GLubyte *errorStr;
	errorStr = gluErrorString(errorCode);
	std::cerr << "[ERROR]: " << errorStr << std::endl;
}

static void __stdcall tessEndCB()
{
	glEnd();
}

void PlanarFacet::triangulate(BinTexture* t) {
	Texture_info = t;
	current_triangulation_buffer.clear();
	current_plane = plane;

	GLUtesselator *tess = gluNewTess(); // create a tessellator

	gluTessCallback(tess, GLU_TESS_BEGIN, (void(__stdcall*)(void))tessBeginCB);
	gluTessCallback(tess, GLU_TESS_END, (void(__stdcall*)(void))tessEndCB);
	gluTessCallback(tess, GLU_TESS_ERROR, (void(__stdcall*)(void))tessErrorCB);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void(__stdcall*)(void))tessVertexCB);
	gluTessCallback(tess, GLU_TESS_COMBINE, (void(__stdcall*)(void))tessCombineCB);

	double buffer[4] = { 0, };
	gluTessBeginPolygon(tess, 0);
	gluTessBeginContour(tess);

	{
		PlanarFacetEdge *e_start = this->exterior;
		PlanarFacetEdge *e = e_start;
		do {
			current_triangulation_buffer.push_back(e->u->p);
			buffer[0] = e->u->p.x; buffer[1] = e->u->p.y;
			gluTessVertex(tess, buffer, (void*)(current_triangulation_buffer.size() - 1));
			e = e->next();
		} while (e && e_start != e);
	}
	gluTessEndContour(tess);

	for (int i = 0; i < (int)holes.size(); ++i) {
		gluTessBeginContour(tess);
		PlanarFacetEdge *e_start = holes[i];
		PlanarFacetEdge *e = e_start;
		do {
			current_triangulation_buffer.push_back(e->u->p);
			buffer[0] = e->u->p.x; buffer[1] = e->u->p.y;
			gluTessVertex(tess, buffer, (void*)(current_triangulation_buffer.size() - 1));
			e = e->next();
		} while (e && e_start != e);
		gluTessEndContour(tess);
	}
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);        // delete after tessellation
}
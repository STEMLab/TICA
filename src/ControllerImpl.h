#pragma once
#include "Controller.h"
#include "Util.h"
#include <imgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <set>

struct Indexer : public DrawSetting {
	Indexer(int _o = 0);

	void begin_facets(void);
	void begin_vertices(void);
	void begin_edges(void);

	bool begin_vertex(Vertex*);
	bool begin_facet(int,Facet*);
	bool begin_facetvertex(int, FacetVertex*);
	bool begin_facetedge(int, FacetEdge*);

	void assign_index(void);

	int offset;
};

struct Highlighter : public DrawSetting {
	Highlighter(int _target, int _i = 0);
	void begin_facets(void);
	void begin_vertices(void);
	void begin_edges(void);

	bool begin_vertex(Vertex*);
	bool begin_facet(int, Facet*);
	bool begin_facetvertex(int, FacetVertex*);
	bool begin_facetedge(int, FacetEdge*);
	
	int target;
	int index;
};

struct Selector : public DrawSetting {
	Selector(int _target, int _i = 0);
	void begin_facets(void);
	void begin_vertices(void);
	void begin_edges(void);

	bool begin_vertex(Vertex*);
	bool begin_facet(int, Facet*);
	bool begin_facetvertex(int, FacetVertex*);
	bool begin_facetedge(int, FacetEdge*);

	int target;
	int index;

	int f_i;
	int v_i;
	int e_i;
	int selected_g;
	int selected_i;
	enum {
		TYPE_NONE,
		TYPE_FACET,
		TYPE_VERTEX,
		TYPE_EDGE,
	}selected_type;
};

struct BasicViewSelector : public Controller {
	BasicViewSelector(Controller*);
	virtual void draw(DrawSetting* d, float vertex_r, float edge_w);
	virtual void draw_select_scene(void);
	virtual void draw_scene(void);

	virtual void make_ui(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

protected:
	std::vector<Vertex*>& pts;
	std::vector<Facet*>& facets;
};

struct MainViewer : public BasicViewSelector {
	MainViewer(Controller *);
	virtual void make_ui(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

	std::set<Vertex*> selected_vertex;
	std::set<FacetEdge*> selected_edge;
	std::set<Facet*> selected_facet;

	void clear_selection(void);
	
	Controller *next;
};


struct PolygonLifter : public BasicViewSelector {
	PolygonLifter(Controller *, Facet *f);
	virtual void make_ui(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);
	virtual void draw_scene(void);

	Facet *base_facet;
	bool finalized;
	bool canceled;
	length_t height;
	int last_x, last_y;
};

struct SolidMaker : public BasicViewSelector {
	SolidMaker(Controller *, Facet *f);
	virtual void make_ui(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);
	virtual void draw_scene(void);

	Facet *base_facet;
	bool finalized;
	bool canceled;
	length_t height;
	int last_x, last_y;
};

struct ConnectionEditor : public BasicViewSelector {
	ConnectionEditor(Controller *, Facet *f1, Facet *f2);
	virtual void make_ui(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);
	virtual void draw_scene(void);

	Facet *base_facet;
	Facet *opposite_facet;
	bool finalized;
	bool canceled;
	int selected_i;
	bool door;

	Connection c;
};

struct PolygonSpliter : public Controller {
	PolygonSpliter(Controller *, Facet *f);
	virtual void make_ui(void);
	virtual void draw(DrawSetting* d, float vertex_r, float edge_w);
	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

	Facet *target_facet;
	bool finalized;
	bool canceled;
	int selected_i;
	float facet_alpha;

	Vertex* start_v;
	FacetEdge* start_e;
	Vertex* end_v;
	FacetEdge* end_e;
	Point3D hover_p;
	std::vector<Point3D> mid_points;
};

struct WallMaker : public BasicViewSelector {
	WallMaker(Controller *, FacetEdge *e);
	virtual void make_ui(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);
	virtual void draw_scene(void);

	FacetEdge *base_edge;
	bool finalized;
	bool canceled;
	length_t height;
	int last_x, last_y;
};

struct PolygonEditor : public Controller {
	int selected_i;
	std::vector<Vertex*>& pts;
	std::vector<Facet*>& facets;
	Vertex *clicked_v;
	Facet *clicked_f;

	PolygonEditor(Controller *, const Plane&);
	virtual void make_ui(void);
	virtual void draw(DrawSetting* d, float vertex_r, float edge_w);
	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

	bool finalized;
	Plane baseplane;
	Vector3D base_u;
	Vector3D base_v;
};



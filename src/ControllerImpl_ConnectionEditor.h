#pragma once

#include "ControllerImpl.h"

struct ConnectionEditorDrawSetting : public DrawSetting {
	virtual void begin_facets(void);
	virtual void begin_vertices(void);
	virtual void begin_edges(void);

	virtual bool begin_vertex(Vertex*);
	virtual bool begin_facet(int, Facet*);
	virtual bool begin_facetvertex(int, FacetVertex*);
	virtual bool begin_facetedge(int, FacetEdge*);

	virtual void begin_connections(void) = 0;
	virtual void begin_connection(int) = 0;
	virtual bool begin_connection_edge(int) = 0;
	virtual bool begin_connection_vertex(int) = 0;

	virtual void begin_connection_on_opposite_facet(int) = 0;
	virtual bool begin_connection_edge_on_opposite_facet(int) = 0;
	virtual bool begin_connection_vertex_on_opposite_facet(int) = 0;
};

struct ConnectionBasicViewer : public Controller {
	ConnectionBasicViewer(Controller *);

	virtual void make_ui(void);
	virtual void draw(ConnectionEditorDrawSetting* d, double ptsize, int linewidth);
	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

	virtual int get_current_stage(void) const;

	int current_selected_obj;

	Controller* next;
};

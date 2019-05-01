#pragma once

#include "Controller.h"
#include <set>
#include "Texture.h"

struct StateEditorDrawSetting : public DrawSetting {
	virtual void begin_facets(void);
	virtual void begin_vertices(void);
	virtual void begin_edges(void);

	virtual bool begin_vertex(Vertex*);
	virtual bool begin_facet(int, Facet*);
	virtual bool begin_facetvertex(int, FacetVertex*);
	virtual bool begin_facetedge(int, FacetEdge*);

	virtual void begin_cellspace(int) = 0;
	virtual void end_cellspace(int);
	virtual void begin_layer(int) = 0;
	virtual void end_layer(int);
	virtual bool begin_state(int,State*) = 0;
	virtual void end_state(int, State*);
	virtual bool begin_transition(int,Transition*) = 0;
	virtual void end_transition(int, Transition*);
	virtual bool begin_interlayerconnection(int, InterlayerConnection*);
	virtual void end_interlayerconnection(int, InterlayerConnection*);
};

struct StateBasicViewer : public Controller {
	StateBasicViewer(Controller *);

	virtual void make_ui(void);
	virtual void draw(StateEditorDrawSetting* d, double state_size = 0.5, int icon_size = 32, int transition_linewidth = 5);
	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);

	virtual int get_current_stage(void) const;

protected:
};

struct StateMainController : public StateBasicViewer {
	StateMainController(Controller *);

	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void make_ui(void);
	virtual void post_draw(void);

protected:
	void clear_selection(void);
	Facet *selected_facet;
	State *selected_state;
	Transition *selected_transition;
	/*std::set<Facet*> selected_facets;
	std::set<State*> selected_states;
	std::set<Transition*> selected_transitions;*/
	StateBasicViewer *next;
};


struct StatePSFeaturePlacer : public StateBasicViewer {
	StatePSFeaturePlacer(Controller *);

	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void make_ui(void);
	virtual void post_draw(void);

protected:
	Facet* on_facet;
	Point3D base_pt;
	length_t height;
	int ps_type;
	bool finalized;
	bool canceled;
};

struct StateLocationEditor: public StateBasicViewer {
	StateLocationEditor(Controller *);

	virtual void draw_select_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void make_ui(void);
	virtual void post_draw(void);

protected:
	bool finalized;
	Point3D *ptr;
};

struct StatePointPlacer : public StateBasicViewer {
	StatePointPlacer(Controller *, Point3D& pt);

	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj);
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj);
	virtual void post_draw(void);

protected:
	Point3D &base_pt;
	Point3D init_pt;
	bool finalized;
	bool canceled;
};

struct StateInterlayerConnectionEstablisher : public StateBasicViewer {
	StateInterlayerConnectionEstablisher(Controller *, State*);

	virtual void draw_select_scene(void);
	virtual void draw_scene(void);
	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj);
	virtual void make_ui(void);
	virtual void post_draw(void);

protected:
	bool finalized;
	bool canceled;
	State *container;
	std::set<State*> selected;
};
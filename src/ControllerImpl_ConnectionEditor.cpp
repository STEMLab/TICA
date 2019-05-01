#include "ControllerImpl_ConnectionEditor.h"
#include "Util.h"

void ConnectionEditorDrawSetting::begin_facets(void) { return ; }
void ConnectionEditorDrawSetting::begin_vertices(void) { return ; }
void ConnectionEditorDrawSetting::begin_edges(void) { return ; }

bool ConnectionEditorDrawSetting::begin_vertex(Vertex*) { return true; }
bool ConnectionEditorDrawSetting::begin_facet(int, Facet*) { return true; }
bool ConnectionEditorDrawSetting::begin_facetvertex(int, FacetVertex*) { return true; }
bool ConnectionEditorDrawSetting::begin_facetedge(int, FacetEdge*) { return true; }

struct ConnectionEditorIndexer : public ConnectionEditorDrawSetting {
	ConnectionEditorIndexer(void)
		:idx(0) {

	}
	void begin_facets(void) { return; }
	void begin_vertices(void) { return; }
	void begin_edges(void) { return; }

	bool begin_vertex(Vertex*) { return false; }
	bool begin_facet(int, Facet*) { setColori(++idx); return true; }
	bool begin_facetvertex(int, FacetVertex*) { return false; }
	bool begin_facetedge(int, FacetEdge*) { return false; }

	virtual void begin_connections(void) {
	}
	virtual void begin_connection(int) {
	}
	virtual bool begin_connection_edge(int) {
		setColori(++idx);
		return true;
	}
	virtual bool begin_connection_vertex(int) {
		setColori(++idx);
		return true;
	}

	virtual void begin_connection_on_opposite_facet(int) {
	}
	virtual bool begin_connection_edge_on_opposite_facet(int) {
		setColori(++idx);
		return true;
	}
	virtual bool begin_connection_vertex_on_opposite_facet(int) {
		setColori(++idx);
		return true;
	}

protected:
	int idx;
};

struct ConnectionEditorSelector : public ConnectionEditorDrawSetting {
	ConnectionEditorSelector(int _target_idx)
		:target_idx(_target_idx), selection_type(CONNECTIONEDITOR_SELECTION_TYPE_NONE), selected_i(-1), selected_j(-1), idx(0) {
		;
	}
	void begin_facets(void) { return; }
	void begin_vertices(void) { return; }
	void begin_edges(void) { return; }

	bool begin_vertex(Vertex*) { return false; }
	bool begin_facet(int, Facet*) { 
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_FACET;
			selected_i = idx;
		}
		return false; 
	}
	bool begin_facetvertex(int, FacetVertex*) { return false; }
	bool begin_facetedge(int, FacetEdge*) { return false; }

	virtual void begin_connections(void) {
	}
	virtual void begin_connection(int i) {
		current_connection_i = i;
	}
	virtual bool begin_connection_edge(int i) {
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_EDGE;
			selected_i = current_connection_i;
			selected_j = i;
		}
		return false;
	}
	virtual bool begin_connection_vertex(int i) {
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_VERTEX;
			selected_i = current_connection_i;
			selected_j = i;
		}
		return false;
	}

	virtual void begin_connection_on_opposite_facet(int i) {
		current_connection_i = i;
	}
	virtual bool begin_connection_edge_on_opposite_facet(int i) {
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_EDGE_ON_OPPOSITE_FACET;
			selected_i = current_connection_i;
			selected_j = i;
		}
		return true;
	}
	virtual bool begin_connection_vertex_on_opposite_facet(int i) {
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_VERTEX_ON_OPPOSITE_FACET;
			selected_i = current_connection_i;
			selected_j = i;
		}
		return true;
	}

	int selected_i;
	int selected_j;

	enum CONNECTIONEDITOR_SELECTION_TYPE {
		CONNECTIONEDITOR_SELECTION_TYPE_NONE,
		CONNECTIONEDITOR_SELECTION_TYPE_FACET,
		CONNECTIONEDITOR_SELECTION_TYPE_VERTEX,
		CONNECTIONEDITOR_SELECTION_TYPE_EDGE,
		CONNECTIONEDITOR_SELECTION_TYPE_VERTEX_ON_OPPOSITE_FACET,
		CONNECTIONEDITOR_SELECTION_TYPE_EDGE_ON_OPPOSITE_FACET,
	} selection_type;

protected:
	int target_idx;
	int idx;
	int current_connection_i;
};

struct ConnectionEditorHighlighter : public ConnectionEditorDrawSetting {
	ConnectionEditorHighlighter(int _target_idx)
		:target_idx(_target_idx), idx(0) {
		;
	}
	void begin_facets(void) { return; }
	void begin_vertices(void) { return; }
	void begin_edges(void) { return; }

	bool begin_vertex(Vertex*) { return false; }
	bool begin_facet(int, Facet*) {
		if (++idx == target_idx) {
			setColorf(1, 0, 0);
		}
		else {
			setColorf(1, 1, 1);
		}
		return true;
	}
	bool begin_facetvertex(int, FacetVertex*) { return true; }
	bool begin_facetedge(int, FacetEdge*) { return true; }

	virtual void begin_connections(void) {
	}
	virtual void begin_connection(int i) {
	}
	virtual bool begin_connection_edge(int i) {
		if (++idx == target_idx) {
			setColorf(1, 0, 0);
		}
		else {
			setColorf(0, 1, 1);
		}
		return true;
	}
	virtual bool begin_connection_vertex(int i) {
		if (++idx == target_idx) {
			setColorf(1, 0, 0);
		}
		else {
			setColorf(0, 1, 1);
		}
		return true;
	}

	virtual void begin_connection_on_opposite_facet(int i) {
		
	}
	virtual bool begin_connection_edge_on_opposite_facet(int i) {
		if (++idx == target_idx) {
			setColorf(1, 0, 0);
		}
		else {
			setColorf(1, 1, 0);
		}
		return true;
	}
	virtual bool begin_connection_vertex_on_opposite_facet(int) {
		if (++idx == target_idx) {
			setColorf(1, 0, 0);
		}else {
			setColorf(1, 1, 0);
		}
		return true;
	}

protected:
	int target_idx;
	int idx;
};



ConnectionBasicViewer::ConnectionBasicViewer(Controller * ctrl)
	: Controller(ctrl), current_selected_obj(-1), next(0) {
	;
}

int ConnectionBasicViewer::get_current_stage(void) const {
	return 1;
}

void ConnectionBasicViewer::make_ui(void) {
	
}

void ConnectionBasicViewer::draw(ConnectionEditorDrawSetting* d, double ptsize, int linewidth) {
	std::vector<Facet*> &facets = Controller::world->facets;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glColor3f(1, 1, 1);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw(d);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	glDepthMask(GL_FALSE);
	glLineWidth(1);
	glColor3f(0, 0, 0);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw_edge(d);
	}

	glDepthMask(GL_TRUE);

	if (d) d->begin_connections();
	for (int i = 0; i < world->connections.size(); ++i) {
		glPointSize(ptsize);
		glLineWidth(linewidth);
		glColor3f(0, 1, 1);
		if (d) d->begin_connection(i);
		glBegin(GL_LINES);
		for (int j = 0; j < world->connections[i].ring.size(); ++j) {
			if (d && !d->begin_connection_edge(j)) continue;
			Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
			Point3D y = world->connections[i].ring[((j + 1) % world->connections[i].ring.size())];
			glVertex3f(x.x, x.y, x.z);
			glVertex3f(y.x, y.y, y.z);
		}
		glEnd();

		//glBegin(GL_POINTS);
		for (int j = 0; j < world->connections[i].ring.size(); ++j) {
			if (d && !d->begin_connection_vertex(j)) continue;
			Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
			//glVertex3f(x.x, x.y, x.z);
			draw_sphere(x.x, x.y, x.z, ptsize);
		}
		//glEnd();

		if (world->connections[i].fbase_opposite) {
			glColor3f(1, 1, 0);
			if (d) d->begin_connection_on_opposite_facet(i);
			glBegin(GL_LINES);
			for (int j = 0; j < world->connections[i].ring.size(); ++j) {
				if (d && !d->begin_connection_edge_on_opposite_facet(j)) continue;
				Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
				x = world->connections[i].fbase_opposite->get_plane().project(x);
				Point3D y = world->connections[i].ring[((j + 1) % world->connections[i].ring.size())];
				y = world->connections[i].fbase_opposite->get_plane().project(y);
				glVertex3f(x.x, x.y, x.z);
				glVertex3f(y.x, y.y, y.z);
			}
			glEnd();

			//glBegin(GL_POINTS);
			for (int j = 0; j < world->connections[i].ring.size(); ++j) {
				if (d && !d->begin_connection_vertex_on_opposite_facet(j)) continue;
				Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
				x = world->connections[i].fbase_opposite->get_plane().project(x);
				draw_sphere(x.x, x.y, x.z, ptsize);
			}
			//glEnd();
		}
	}
}
void ConnectionBasicViewer::draw_select_scene(void) {
	ConnectionEditorIndexer d;
	draw(&d, 0.1, 5);
}
void ConnectionBasicViewer::draw_scene(void) {
	ConnectionEditorHighlighter d(current_selected_obj);
	draw(&d, 0.1, 5);
}
void ConnectionBasicViewer::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionBasicViewer::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionBasicViewer::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {
	current_selected_obj = current_obj;
}
void ConnectionBasicViewer::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionBasicViewer::post_draw(void) {
	if (next) {
		Controller::current_controller = next;
		next = 0;
	}
}
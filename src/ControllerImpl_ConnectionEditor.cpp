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
	bool begin_facet(int i, Facet*) { 
		if (++idx == target_idx) {
			selection_type = CONNECTIONEDITOR_SELECTION_TYPE_FACET;
			selected_i = i;
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
	bool begin_facet(int, Facet* f) {
		if (++idx == target_idx) {
			setColorf(0.5, 0, 0);
		}
		else {
			if (abs(f->get_plane().h.normalized().z) > 0.8) {
				glColor3f(0.4, 0.4, 0.4);
			}
			else {
				glColor3f(0.5, 0.5, 0.5);
			}
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
			setColorf(0.5, 0, 0);
		}
		else {
			setColorf(0, 1, 1);
		}
		return true;
	}
	virtual bool begin_connection_vertex(int i) {
		if (++idx == target_idx) {
			setColorf(0.5, 0, 0);
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
			setColorf(0.5, 0, 0);
		}
		else {
			setColorf(1, 1, 0);
		}
		return true;
	}
	virtual bool begin_connection_vertex_on_opposite_facet(int) {
		if (++idx == target_idx) {
			setColorf(0.5, 0, 0);
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
	: Controller(ctrl), current_selected_obj(-1), next(0), f_front(0), f_back(0), valid_connection(false) {
	;
}

int ConnectionBasicViewer::get_current_stage(void) const {
	return 1;
}


float ConnectionBasicViewer::door_baseoffset = 0.1501;
float ConnectionBasicViewer::door_width = 1;
float ConnectionBasicViewer::door_height = 2.0;

void ConnectionBasicViewer::make_ui(void) {
	ImGui::Begin("Connection Editor");
	ImGui::DragFloat("Base Offset", &door_baseoffset, 0.01);
	ImGui::DragFloat("Width", &door_width, 0.01);
	ImGui::DragFloat("Height", &door_height, 0.01);

	ImGui::Text("");
	if (ImGui::Button("Undo", ImVec2(300, 0))) {
		if (!this->world->connections.empty()) {
			this->world->connections.pop_back();
		}
	}
	ImGui::Text("");
	if (this->f_front && this->f_back) {
		if (ImGui::Button("Make a connection from intersection", ImVec2(300, 0))) {
			Plane plane = this->f_back->get_plane();
			Polygon2D p1 = this->f_front->get_polygon(plane);
			Polygon2D p2 = this->f_back->get_polygon();

			p2.exterior = std::vector<Point2D>(p2.exterior.rbegin(), p2.exterior.rend());
			std::vector<Polygon2D> p = p1.intersection(p2);

			Vector3D u, v;
			plane.get_basis(&u, &v);
			for (int j = 0; j < p.size(); ++j) {
				std::vector<Point3D> pts;
				
				for (int k = 0; k+1 < p[j].exterior.size(); ++k) {
					Point3D p_on_back = plane.p + (u * p[j].exterior[k].x + v * p[j].exterior[k].y);
					Line3D l(p_on_back, plane.h);
					scalar_t a;
					interp(l, this->f_front->get_plane(), &a);
					Point3D p_proj = l.p + l.v * a;
					pts.push_back(p_proj);
				}

				Connection conn;
				conn.fbase = this->f_front;
				conn.fbase_opposite = this->f_back;
				conn.ring = pts;
				this->world->connections.push_back(conn);
			}
		}
	}
	ImGui::End();
}

static void draw_connection(const Connection &c, ConnectionEditorDrawSetting *d, double ptsize, int linewidth, int idx = 0) {
	glPointSize(ptsize);
	glLineWidth(linewidth);
	//glColor3f(0, 1, 1);
	if (d) d->begin_connection(idx);
	glBegin(GL_LINES);
	for (int j = 0; j < c.ring.size(); ++j) {
		if (d && !d->begin_connection_edge(j)) continue;
		Point3D x = c.ring[(j%c.ring.size())];
		Point3D y = c.ring[((j + 1) % c.ring.size())];
		glVertex3f(x.x, x.y, x.z);
		glVertex3f(y.x, y.y, y.z);
	}
	glEnd();

	//glBegin(GL_POINTS);
	for (int j = 0; j < c.ring.size(); ++j) {
		if (d && !d->begin_connection_vertex(j)) continue;
		Point3D x = c.ring[(j%c.ring.size())];
		//glVertex3f(x.x, x.y, x.z);
		draw_sphere(x.x, x.y, x.z, ptsize);
	}
	//glEnd();

	if (c.fbase_opposite) {
		//glColor3f(1, 1, 0);
		if (d) d->begin_connection_on_opposite_facet(idx);
		glBegin(GL_LINES);
		for (int j = 0; j < c.ring.size(); ++j) {
			if (d && !d->begin_connection_edge_on_opposite_facet(j)) continue;
			Point3D x = c.ring[(j%c.ring.size())];
			x = c.fbase_opposite->get_plane().project(x);
			Point3D y = c.ring[((j + 1) % c.ring.size())];
			y = c.fbase_opposite->get_plane().project(y);
			glVertex3f(x.x, x.y, x.z);
			glVertex3f(y.x, y.y, y.z);
		}
		glEnd();

		//glBegin(GL_POINTS);
		for (int j = 0; j < c.ring.size(); ++j) {
			if (d && !d->begin_connection_vertex_on_opposite_facet(j)) continue;
			Point3D x = c.ring[(j%c.ring.size())];
			x =c.fbase_opposite->get_plane().project(x);
			draw_sphere(x.x, x.y, x.z, ptsize);
		}
		//glEnd();
	}
}

void ConnectionBasicViewer::draw(ConnectionEditorDrawSetting* d, double ptsize, int linewidth) {
	std::vector<Facet*> &facets = Controller::world->facets;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glColor3f(0.5, 0.5, 0.5);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw(d,i);
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

	glColor3f(1, 1, 0);
	if (d) d->begin_connections();
	for (int i = 0; i < world->connections.size(); ++i) {
		draw_connection(world->connections[i], d, ptsize, linewidth, i);
	}
}
void ConnectionBasicViewer::draw_select_scene(void) {
	ConnectionEditorIndexer d;
	draw(&d, 0.1, 5);
}
void ConnectionBasicViewer::draw_scene(void) {
	ConnectionEditorHighlighter d(current_selected_obj);
	draw(&d, 0.1, 5);

	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glLineWidth(2);
		glColor3f(1, 0.5, 0.5);

		if (f_back) f_back->draw_edge(0);
		if (f_front) f_front->draw_edge(0);

		glColor3f(1, 0.5, 0.5);
		glDepthFunc(GL_GREATER);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xFF00);
		glLineWidth(1);

		if (f_back) f_back->draw_edge(0);
		if (f_front) f_front->draw_edge(0);

		glDepthFunc(GL_LESS);
		glDisable(GL_LINE_STIPPLE);
		glDepthMask(GL_TRUE);
	}

	if (!current_connection.ring.empty()) {
		if(valid_connection) glColor3f(1, 1, 1);
		else glColor3f(1, 0.5, 0.5);
		draw_connection(current_connection, 0, 0.1, 5);
	}
}
#include <iostream>
using namespace std;
static Connection make_connection(Facet *f_front, Facet *f_back, const Point3D& p_on_f) {
	Connection c;
	c.fbase = f_front;
	c.fbase_opposite = f_back;

	Vector3D side_vec = Vector3D(0, 0, 1).cross_product(f_front->get_plane().h).normalized();
	Point3D bottom_center = p_on_f;
	Vector3D upvector = f_front->get_plane().h.cross_product(side_vec).normalized();
	{
		int max_i = -1;
		float max_dot = 0;
		for (int i = 0; i < f_front->num_edges(); ++i) {
			
			Vector3D v = f_front->get_edge(i)->to_vector3();
			if (v.is_zero()) continue;
			v = v.normalized();

			scalar_t dot = v.dot_product(Vector3D(0, 0, 1));

			if (max_i == -1 || max_dot < abs(dot)) {
				max_i = i;
				max_dot = abs(dot);
				if (dot > 0) { upvector = v; }
				else { upvector = -1 * v; }
			}
			/*if (max_i == -1 || max_dot < 1-abs(dot)) {
				max_i = i;
				max_dot = 1-abs(dot);

				upvector = v.cross_product(Vector3D(0,0,1)).normalized();
				if (upvector.z < 0) upvector = -1 * upvector;
			}*/
		}
		if (max_i != -1) {
			side_vec = upvector.cross_product(f_front->get_plane().h).normalized();
		}
	}
	//upvector = ConnectionBasicViewer::door_height * upvector;

	{
		int min_i = -1;
		float min_alpha = 0;
		for (int i = 0; i < f_front->num_edges(); ++i) {
			float alpha;
			float beta;
			if (!interp(Line3D(p_on_f, upvector), Line3D(f_front->get_edge(i)->get_u()->to_point3(), f_front->get_edge(i)->to_vector3()), &alpha, &beta)) continue;

			if (alpha > 0) continue;
			if (beta < 0 || 1 < beta) continue;
			if (min_i == -1 || alpha < min_alpha) {
				min_i = i;
				min_alpha = alpha;
			}
		}
		if (min_i != -1) {
			bottom_center = p_on_f + (min_alpha + ConnectionBasicViewer::door_baseoffset) * upvector;
		}
	}
	//bottom_center.z = ConnectionBasicViewer::door_baseoffset;
	bottom_center = f_front->get_plane().project(bottom_center);
	upvector = ConnectionBasicViewer::door_height * upvector;
	side_vec = ConnectionBasicViewer::door_width / 2 * side_vec;

	//c.ring.push_back(p_on_f + side_vec - Vector3D(0, 0, 1));
	//c.ring.push_back(p_on_f + side_vec + Vector3D(0, 0, 1));
	//c.ring.push_back(p_on_f - side_vec + Vector3D(0, 0, 1));
	//c.ring.push_back(p_on_f - side_vec - Vector3D(0, 0, 1));
	c.ring.push_back(bottom_center + side_vec);
	c.ring.push_back(bottom_center + side_vec + upvector);
	c.ring.push_back(bottom_center - side_vec + upvector);
	c.ring.push_back(bottom_center - side_vec);
	
	return c;
}
void ConnectionBasicViewer::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	current_selected_obj = current_obj;
	ConnectionEditorSelector sel(current_obj);
	draw(&sel, 0.1, 5);
	
	Facet *f_front_old = f_front, *f_back_old = f_back;
	f_front = f_back = 0;
	World& world = *Controller::world;
	std::vector<Facet*> &facets = Controller::world->facets;
	if (sel.selection_type == ConnectionEditorSelector::CONNECTIONEDITOR_SELECTION_TYPE_EDGE) {
		//cout << sel.selected_i << endl;
	}
	if (sel.selection_type == ConnectionEditorSelector::CONNECTIONEDITOR_SELECTION_TYPE_FACET) {
		do {
			Facet* f = facets[sel.selected_i];
			//if (abs(f->get_plane().h.z) > 0.9) break; // skip floor or ceiling

			float dist2 = 0;
			float alpha;
			interp(ray, f->get_plane(), &alpha);

			Point3D p_on_f = ray.p + alpha * ray.v;
			Line3D ray_from_p(p_on_f, -1 * f->get_plane().h);
			for (int i = 0; i < facets.size(); ++i) {
				Facet *f2 = facets[i];
				if (f == f2) continue;
				if (f->get_plane().h.normalized().dot_product(f2->get_plane().h.normalized()) > -0.7) continue;
				alpha = 0;

				if (!interp(ray_from_p, f2->get_plane(), &alpha)) continue;
				if (alpha <= -0.001) continue;
				Point3D p_on_f2 = ray_from_p.p + alpha * ray_from_p.v;
				p_on_f2 = f2->get_plane().project(p_on_f);
				float d2 = (p_on_f2 - p_on_f).length_square();

				if (d2 > 1) continue;

				if (!f_front || d2 < dist2) {
					// check point in polygon.
					int w = f2->winding_number(p_on_f2);
					if (abs(w) % 2 == 1) {
						//cout << "CHECK " << f << '\t' << f2 << '\t' << w << '\t' << d2 << '\t' << alpha << ',' << p_on_f2.x << ',' << p_on_f2.y << ',' << p_on_f2.z << '\t' << f->get_plane().h.normalized().dot_product(f2->get_plane().h.normalized()) << endl;
						f_front = f;
						f_back = f2;
						dist2 = d2;
					}
				}
			}

			if (!f_front) f_front = f;
			if (f_front && f_front == f_front_old && f_back == f_back_old && f_front != f_back) {
				Connection c = make_connection(f_front, f_back, p_on_f);

				bool failed = false;
				for (int i = 0; i < c.ring.size(); ++i) {
					Point3D p = c.ring[i];

					if (f_front->winding_number(p) % 2 == 0) {
						failed = true;
						break;
					}

					if (f_back) {
						Point3D q = f_back->get_plane().project(c.ring[i]);
						if (f_back->winding_number(q) % 2 == 0) {
							failed = true;
							break;
						}
					}
				}
				if( !failed ) world.connections.push_back(c);
			}
		} while (0);
	}
}
void ConnectionBasicViewer::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionBasicViewer::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {
	//current_selected_obj = current_obj;
	valid_connection = false;
	current_connection.ring.clear();
	if (current_selected_obj == current_obj && f_front) {
		Facet *f = f_front;
		float alpha;
		interp(ray, f->get_plane(), &alpha);
		Point3D p_on_f = ray.p + alpha * ray.v;

		Connection c = make_connection(f_front, f_back, p_on_f);

		bool failed = false;
		for (int i = 0; i < c.ring.size(); ++i) {
			Point3D p = c.ring[i];

			if (f_front->winding_number(p) % 2 == 0) {
				failed = true;
				break;
			}

			if (f_back) {
				Point3D q = f_back->get_plane().project(c.ring[i]);
				if (f_back->winding_number(q) % 2 == 0) {
					failed = true;
					break;
				}
			}
			else {
				/*std::vector<Facet*> &facets = Controller::world->facets;
				for (int j = 0; j < facets.size(); ++j) {
					Facet *f2 = facets[j];
					if (f2 == f_front) continue;
					Point3D p_on_f2 = f2->get_plane().project(c.ring[i]);
					if ((p_on_f2 - c.ring[i]).dot_product(f2->get_plane().h) > 0) continue;
					if ((p_on_f2 - c.ring[i]).length_square() > 1) continue;
					int w = f2->winding_number(p_on_f2);
					if (abs(w) % 2 == 1) {
						failed = true;
						break;
					}
				}*/
			}
		}
		valid_connection = !failed;
		current_connection = c;
	}
}
void ConnectionBasicViewer::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionBasicViewer::post_draw(void) {
	if (next) {
		Controller::current_controller = next;
		next = 0;
	}
}
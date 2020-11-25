#include "ControllerImpl.h"

using namespace std;

#include <sstream>
#include <string>
#include <stack>

Point3D get_camera_pos(void);

static float vertex_size = 0.15;
static int edge_width = 1;

Indexer::Indexer(int _o)
	:offset(_o) {
	;
}
void Indexer::begin_facets(void) {}
void Indexer::begin_vertices(void) {}
void Indexer::begin_edges(void) {}

bool Indexer::begin_facet(int,Facet*) {
	assign_index();
	return true;
}
bool Indexer::begin_vertex(Vertex*) {
	assign_index();
	return true;
}
bool Indexer::begin_facetvertex(int, FacetVertex*) {
	assign_index();
	return true;
}
bool Indexer::begin_facetedge(int,FacetEdge*) {
	assign_index();
	return true;
}

void Indexer::assign_index(void) {
	GLubyte v[4];
	decompose_integer(++offset, v);
	glColor4ub(v[0], v[1], v[2], v[3]);
}


Highlighter::Highlighter(int _target, int _i)
	:target(_target), index(_i) {
	;
}
void Highlighter::begin_facets(void) {}
void Highlighter::begin_vertices(void) {}
void Highlighter::begin_edges(void) {}

bool Highlighter::begin_vertex(Vertex*) {
	if (++index == target) {
		glColor3f(0.3, 0, 0);
	}
	else {
		glColor3f(1, 1, 1);
	}
	return true;
}
bool Highlighter::begin_facet(int i, Facet* f) {
	if (++index == target) {
		glColor3f(0.3, 0, 0);
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
bool Highlighter::begin_facetvertex(int i, FacetVertex*) {
	if (++index == target) {
		glColor3f(1, 0, 0);
	}
	else {
		glColor3f(1, 1, 1);
	}
	return true;
}
bool Highlighter::begin_facetedge(int i, FacetEdge*) {
	if (++index == target) {
		glColor3f(0.3, 0, 0);
	}
	else {
		glColor3f(0.5, 1, 0.5);
	}
	return true;
}

Selector::Selector(int _target, int _i)
	:target(_target), index(_i), f_i(-1), v_i(-1), e_i(-1), selected_type(TYPE_NONE) {
	;
}
void Selector::begin_facets(void) { ++f_i; }
void Selector::begin_vertices(void) { ++v_i; }
void Selector::begin_edges(void) { ++e_i; }

bool Selector::begin_vertex(Vertex*) {
	++v_i;
	if (++index == target) {
		selected_type = TYPE_VERTEX;
		selected_i = 0;
		selected_g = v_i;
	}
	return false;
}
bool Selector::begin_facet(int i, Facet*) {
	if (++index == target) {
		selected_type = TYPE_FACET;
		selected_i = i;
		selected_g = f_i;
	}
	return false;
}
bool Selector::begin_facetvertex(int i, FacetVertex*) {
	if (++index == target) {
		selected_type = TYPE_VERTEX;
		selected_i = i;
		selected_g = v_i;
	}
	return false;
}
bool Selector::begin_facetedge(int i,FacetEdge*) {
	if (++index == target) {
		selected_type = TYPE_EDGE;
		selected_i = i;
		selected_g = e_i;
	}
	return false;
}

BasicViewSelector::BasicViewSelector(Controller *ctrl)
: Controller(ctrl), pts(Controller::world->vertices), facets( Controller::world->facets) {

}
void BasicViewSelector::draw(DrawSetting* d, float vertex_r, float edge_w) {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glColor3f(0.5, 0.5, 0.5);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw(d);
		/*glColor3f(0, 0, 1);
		Plane p = facets[i]->get_plane();
		glBegin(GL_POINTS);
		glVertex3f(p.p.x, p.p.y, p.p.z);
		glEnd();*/
	}
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	glDepthMask(GL_FALSE);
	glLineWidth(edge_w);
	glColor3f(0.5, 1, 0.5);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw_edge(d);
	}

	glDepthMask(GL_TRUE);
	glPointSize(5);
	for (int i = 0; i < pts.size(); ++i) {
		switch (pts[i]->num_shared_facets()) {
			case 0: glColor3f(1, 1, 1); break;
			case 1: glColor3f(0.8, 1, 0.8); break;
			case 2: glColor3f(0.5, 1, 0.5); break;
			default: glColor3f(0, 0, 1);
		}

		pts[i]->draw(vertex_r, d);
	}


	
	//if(d) d->begin_connections();
	for (int i = 0; i < world->connections.size(); ++i) {
		glPointSize(10);
		glLineWidth(edge_w);
		glColor3f(0, 1, 1);
		//if (d) d->begin_connection();
		glBegin(GL_LINES);
		for (int j = 0; j < world->connections[i].ring.size(); ++j) {
			Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
			Point3D y = world->connections[i].ring[((j+1)%world->connections[i].ring.size())];
			glVertex3f(x.x, x.y, x.z);
			glVertex3f(y.x, y.y, y.z);
		}
		glEnd();

		if (world->connections[i].fbase_opposite) {
			glColor3f(1, 1, 0);
			//if (d) d->begin_connection();
			glBegin(GL_LINES);
			for (int j = 0; j < world->connections[i].ring.size(); ++j) {
				Point3D x = world->connections[i].ring[(j%world->connections[i].ring.size())];
				x = world->connections[i].fbase_opposite->get_plane().project(x);
				Point3D y = world->connections[i].ring[((j+1)%world->connections[i].ring.size())];
				y = world->connections[i].fbase_opposite->get_plane().project(y);
				glVertex3f(x.x, x.y, x.z);
				glVertex3f(y.x, y.y, y.z);
			}
		}
		glEnd();
	}
	
	for (int i = 0; i < world->objects.size(); ++i) {
		world->objects[i].draw();
	}
}

void BasicViewSelector::make_ui(void) {

}

void BasicViewSelector::draw_select_scene(void) {
	Indexer idx;
	draw(&idx, vertex_size, edge_width);
}

void BasicViewSelector::draw_scene(void) {
	draw(0, vertex_size, edge_width);
}
void BasicViewSelector::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	
}
void BasicViewSelector::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void BasicViewSelector::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {

}
void BasicViewSelector::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void BasicViewSelector::post_draw(void) {

}


MainViewer::MainViewer(Controller *ctrl)
	:BasicViewSelector(ctrl), next(0), show_inspection_result(false) {
	;
}

struct SelectedObjectHighlighter : public DrawSetting {
	MainViewer *view;
	SelectedObjectHighlighter(MainViewer* _v) : view(_v) {

	}
	void begin_facets(void) {}
	void begin_vertices(void) {}
	void begin_edges(void) {}

	bool begin_vertex(Vertex* v) {
		if (view->selected_vertex.count(v)) {
			glColor3f(1, 0, 0);
		}
		else {
			//glColor3f(1, 1, 1);
		}
		return true;
	}
	bool begin_facet(int, Facet* f) {
		if (view->selected_facet.count(f)) {
			glColor3f(0.3, 0, 0);
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

	bool begin_facetedge(int, FacetEdge* e) {
		if (view->selected_edge.count(e)) {
			glColor3f(0.3, 0, 0);
		}
		else {
			glColor3f(0.5, 1, 0.5);
		}
		return true;
	}
	bool begin_facetvertex(int, FacetVertex*) { return true; }
	
	void assign_index(void) {}
};

void MainViewer::clear_selection(void) {
	selected_facet.clear();
	selected_edge.clear();
	selected_vertex.clear();
}

void MainViewer::inspect_geometry(void) {
	{
		inspection_facets.clear();
		int n = facets.size();
		for (int i = 0; i < n; ++i) {
			int m = facets[i]->num_edges();
			bool failed = false;
			for (int j = 0; j < m; ++j) {
				FacetEdge* e =facets[i]->get_edge(j);
				if (!e->is_shared()) {
					failed = true;
					break;
				}
			}
			
			if (!facets[i]->is_good()) { failed = true; }

			if (failed) {
				inspection_facets.push_back(facets[i]);
			}
		}
	}

	{
		inspection_vertex.clear();
		int n = pts.size();
		for (int i = 0; i < n; ++i) {
			if (pts[i]->num_shared_facets() == 0) {
				inspection_vertex.push_back(pts[i]);
				continue;
			}
			Point3D p = pts[i]->to_point();
			if (!isfinite(p.x) || !isfinite(p.y) || !isfinite(p.z)) {
				inspection_vertex.push_back(pts[i]);
				continue;
			}
		}
	}

	show_inspection_result = true;
}

void MainViewer::make_ui(void) {
	{
		ImGui::Begin("Edit");
		/*ImGui::Text("Selected Facets : %d", selected_facet.size());
		ImGui::Text("Selected Edges : %d", selected_edge.size());
		ImGui::Text("Selected Vertices : %d", selected_vertex.size());*/

		/*if (selected_facet.size() == 1) {
			Facet*f = (*selected_facet.begin());
			Plane p = f->get_plane();
			ImGui::Text("Clicked Pos : %f %f %f", cpos.x, cpos.y, cpos.z);
			ImGui::Text("Selected Facet Normal : %f %f %f", p.h.x, p.h.y, p.h.z);
			ImGui::Text("Selected Facet Center : %f %f %f", p.p.x, p.p.y, p.p.z);
			ImGui::Text("Selected Facet #Vertices : %d", f->num_vertices());
		}
		if (selected_vertex.size() == 1) {
			Vertex*v = (*selected_vertex.begin());
			Point3D p = v->to_point();
			ImGui::Text("Selected Vertex Pos: %f %f %f", p.x, p.y, p.z);
		}
		if (selected_vertex.size() == 2) {
			Vertex*u = (*selected_vertex.begin());
			Vertex*v = (*selected_vertex.rbegin());
			Point3D p = u->to_point();
			Point3D q = v->to_point();
			ImGui::Text("Selected Vertex1 Pos: %f %f %f", p.x, p.y, p.z);
			ImGui::Text("Selected Vertex2 Pos: %f %f %f", q.x, q.y, q.z);
			ImGui::Text("Point-Point Distance: %f", (float)(p - q).length());
		}
		if (selected_vertex.size() == 1 && selected_edge.size() == 1) {
			Vertex* v = (*selected_vertex.begin());
			FacetEdge* e = (*selected_edge.begin());
			Point3D p = v->to_point();
			Point3D q1 = e->get_u()->to_point3();
			Point3D q2 = e->get_v()->to_point3();
			Line3D l(q1, q2 - q1);
			scalar_t alpha;
			if (interp(p, l, &alpha)) {
				if (alpha < 0) alpha = 0;
				if (alpha > 0) alpha = 1;
				Point3D q = l.p + alpha * l.v;
				ImGui::Text("Point-Line Distance: %f", (float)(p - q).length());
			}
		}

		if (ImGui::Button("Clear Selection")) {
			clear_selection();
		}*/
		if (ImGui::Button("Inspect", ImVec2(250, 0))) {
			show_inspection_result = false;
			inspect_geometry();
		}
		ImGui::Text("");

		if (ImGui::Button("Edit Polygons", ImVec2(250,0))) {
			if (!next) {
				Plane plane(Point3D(0, 0, 0), Vector3D(0, 0, 0));
				if (selected_facet.size() == 1) {
					plane = (*selected_facet.begin())->get_plane();
				}
				next = new PolygonEditor(this, plane);
				clear_selection();
			}
		}

		if (selected_facet.size() == 1 && selected_edge.size() == 0 && selected_vertex.size() == 0) {
			do {
				{
					Facet *f = *selected_facet.begin();
					if (f->has_no_adjacent_polygons()) {
						if (ImGui::Button("Extrude", ImVec2(250, 0))) {
							if (!next) {
								next = new SolidMaker(this, f);
								clear_selection();
								break;
							}
						}
					}
				}
				/*{
					Facet *f = *selected_facet.begin();
					if (ImGui::Button("Lift", ImVec2(250, 0))) {
						if (!next) {
							next = new PolygonLifter(this, f);
							clear_selection();
							break;
						}
					}
				}*/
				if (ImGui::Button("Reverse Direction", ImVec2(250, 0))) {
					Facet *f = *selected_facet.begin();
					f->reverse();
				}

				{
					Facet *f = *selected_facet.begin();
					if (f->num_holes() == 0) {
						if (ImGui::Button("Split", ImVec2(250, 0))) {
							if (!next) {
								next = new PolygonSpliter(this, f);
								clear_selection();
								break;
							}
						}
					}
				}
				ImGui::Text("");

				if (ImGui::Button("Select All Connected Planar Facets", ImVec2(250, 0))) {
					Facet* f = *selected_facet.begin();
					stack<Facet*> stk;
					stk.push(f);
					while (!stk.empty()) {
						Facet *f_top = stk.top();
						Vector3D h_norm = f_top->get_plane().h.normalized();
						stk.pop();
						for( int i = 0; i < f_top->num_edges(); ++i ) {

							FacetEdge* e = f_top->get_edge(i);
							if (e->is_shared()) {
								Facet* f_opp = e->get_opposite_facet();
								if (f_opp && selected_facet.find(f_opp) == selected_facet.end()) {
									if (f_opp->get_plane().h.normalized().dot_product(h_norm) > 0.999) {
										selected_facet.insert(f_opp);
										stk.push(f_opp);
									}
								}
							}
						}
					}
				}

				if (ImGui::Button("Select All Connected Facets", ImVec2(250, 0))) {
					Facet* f = *selected_facet.begin();
					stack<Facet*> stk;
					stk.push(f);
					while (!stk.empty()) {
						Facet* f_top = stk.top();
						Vector3D h_norm = f_top->get_plane().h.normalized();
						stk.pop();
						for (int i = 0; i < f_top->num_edges(); ++i) {
							FacetEdge* e = f_top->get_edge(i);
							Facet* f_opp = e->get_opposite_facet();
							if (f_opp && selected_facet.find(f_opp) == selected_facet.end()) {
								selected_facet.insert(f_opp);
								stk.push(f_opp);
							}
						}
					}
				}
				ImGui::Text("");

				if (ImGui::Button("Planarize", ImVec2(250, 0))) {
					Facet *f = *selected_facet.begin();
					f->planarize();
				}
				/*if (ImGui::Button("Make Connection")) {
					Facet *f = *selected_facet.begin();
					if (!next) {
						next = new ConnectionEditor(this, f, 0);
						clear_selection();
					}
				}*/

				ImGui::Text("");
				if (ImGui::Button("Delete", ImVec2(250, 0))) {
					Facet *f = *selected_facet.begin();
					for (int i = 0; i < world->facets.size(); ++i) {
						if (world->facets[i] == f) {
							f->release(&world->vertices);
							world->facets.erase(world->facets.begin() + i);
							break;
						}
					}
					clear_selection();
					break;
				}
			} while (0);
		}

		if (selected_facet.size() >= 1) {
			ImGui::Text("");
			if (ImGui::Button("Lift", ImVec2(250, 0))) {
				if (!next) {
					vector<Facet*> facets(selected_facet.begin(), selected_facet.end());
					next = new MultiPolygonLifter(this, facets);
					clear_selection();
				}
			}

			if (ImGui::Button("Copy Reverse", ImVec2(250, 0))) {
				map<Vertex*, Vertex*> vertex_map;

				// create vertex map
				for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
					Facet* f = (*i);
					for (int j = 0; j < f->num_vertices(); ++j) {
						Vertex* v = f->get_vertex(j)->get_vertex();
						if (vertex_map.find(v) == vertex_map.end()) {
							vertex_map[v] = new Vertex(v->to_point());
						}
					}
				}

				// create new facets
				vector<Facet*> facets;
				for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
					Facet* f = (*i);
					Facet* f_new = 0;
					{
						vector<Vertex*> exterior;
						FacetEdge* e_begin = f->get_exteior_edge();
						FacetEdge* e = e_begin;
						do {
							FacetVertex *v = e->get_u();
							exterior.push_back(vertex_map.at(v->get_vertex()));
							e = e->next();
						} while (e != e_begin);
						f_new = Facet::create_facet(exterior);
					}

					for (int j = 0; j < f->num_holes(); ++j) {
						vector<Vertex*> hole;
						FacetEdge* e_begin = f->get_hole_edge(j);
						FacetEdge* e = e_begin;
						do {
							FacetVertex* v = e->get_u();
							hole.push_back(vertex_map.at(v->get_vertex()));
							e = e->next();
						} while (e != e_begin);
						f_new->make_hole(hole);
					}

					facets.push_back(f_new);
				}

				// reverse
				for (int i = 0; i < facets.size(); ++i) {
					facets[i]->reverse();
				}

				// add newly created vertices and facets
				for (auto i = vertex_map.begin(); i != vertex_map.end(); ++i) {
					world->vertices.push_back(i->second);
				}
				for (int i = 0; i < facets.size(); ++i) {
					world->facets.push_back(facets[i]);
				}

				// update selection
				selected_facet.clear();
				for (int i = 0; i < facets.size(); ++i) {
					selected_facet.insert(facets[i]);
				}
			}
		}

		if (selected_facet.size() > 1 && selected_edge.size() == 0 && selected_vertex.size() == 0) {
			if (ImGui::Button("Delete All", ImVec2(250, 0))) {
				ImGui::OpenPopup("Delete All Facets");
			}

			if (ImGui::BeginPopupModal("Delete All Facets")) {
				ImGui::Text("Do you really want to delete the facets? This task cannot be undone.");
				bool c = false;
				if (ImGui::Button("No", ImVec2(100, 0))) {
					c = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Yes", ImVec2(100, 0))) {
					c = true;
					for (int i = 0; i < world->facets.size(); ++i) {
						vector<Facet*> after_remove;
						after_remove.reserve(world->facets.size());
						for( int i = 0; i < world->facets.size(); ++i ) {
							if (selected_facet.find(world->facets[i]) == selected_facet.end()) {
								after_remove.push_back(world->facets[i]);
							}
						}
						for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
							(*i)->release(&world->vertices);
						}
						world->facets = after_remove;
						clear_selection();
					}
				}
				if (c) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			if (ImGui::Button("Merge All", ImVec2(250, 0))) {
				ImGui::OpenPopup("Merge All Facets");
			}

			if (ImGui::BeginPopupModal("Merge All Facets")) {
				ImGui::Text("Do you really want to merge all the adjacent co-planar facets? This task cannot be undone.");
				bool c = false;
				if (ImGui::Button("No", ImVec2(100, 0))) {
					c = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Yes", ImVec2(100, 0))) {
					c = true;

					while (!selected_facet.empty()) {
						Facet* fa = *selected_facet.begin();
						for (int j = 0; j < fa->num_edges(); ++j) {
							Facet* fb = fa->get_edge(j)->get_opposite_facet();
							if (!fb) continue;
							if (selected_facet.find(fb) == selected_facet.end()) continue;

							if (fa->get_plane().h.normalized().dot_product(fb->get_plane().h.normalized()) > 0.999) {
								Facet* f_new = Facet::merge(fa, fb, &pts, &facets);
								if (f_new) {
									selected_facet.erase(fa);
									selected_facet.erase(fb);
									selected_facet.insert(f_new);
									break;
								}
							}
						}
						selected_facet.erase(fa);
					}
				}
				if (c) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImGui::Text("");
			if (ImGui::Button("Select Facets Inside", ImVec2(250, 0))) {
					set<Facet*> boundary(selected_facet.begin(), selected_facet.end());
					selected_facet.clear();

					map<Vertex*,bool> inside;
					for(int k = 0; k < pts.size(); ++k ) {
						Point3D o = pts[k]->to_point();
						scalar_t omega = 0.0;
						for (auto i = boundary.begin(); i != boundary.end(); ++i) {
							Facet* f = *i;
							Point3D p = f->get_vertex(0)->to_point3();
							for (int j = 1; j < f->num_vertices(); ++j) {
								FacetVertex* v = f->get_vertex(j);
								Point3D q = v->to_point3();
								if (v->next() == f->get_vertex(0)) continue;
								Point3D r = v->next()->to_point3();

								omega += solid_angle(o, p, q, r);
							}
						}
						inside[pts[k]] = !isfinite(omega) || ( omega < -1.999* PI );
					}

					for (int k = 0; k < facets.size(); ++k) {
						if (boundary.find(facets[k]) != boundary.end()) continue;
						bool flag = true;
						for (int i = 0; i < facets[k]->num_vertices(); ++i) {
							Vertex* v = facets[k]->get_vertex(i)->get_vertex();
							if (!inside.at(v)) {
								flag = false;
								break;
							}
						}
						if (flag) {
							selected_facet.insert(facets[k]);
						}
					}
			}
		}

		if (selected_facet.size() > 0 && selected_edge.size() == 0 && selected_vertex.size() == 1) {
			ImGui::Text("");
			if (ImGui::Button("Compute Solid Angle",ImVec2(250,0))) {
				Point3D o = (*selected_vertex.begin())->to_point();
				vector<Facet*> boundary(selected_facet.begin(), selected_facet.end());
				scalar_t omega = 0.0;
				//selected_facet.clear();
				for (int i = 0; i < boundary.size(); ++i) {
					Facet* f = boundary[i];
					Point3D p = f->get_vertex(0)->to_point3();
					for (int j = 1; j < f->num_vertices(); ++j) {
						FacetVertex* v = f->get_vertex(j);
						Point3D q = v->to_point3();
						if (v->next() == f->get_vertex(0)) continue;
						Point3D r = v->next()->to_point3();
						
						omega += solid_angle(o, p, q, r);
					}
				}
				cout << "OMEGA " << omega << endl;
			}
		}

		if (selected_facet.size() == 2 && selected_edge.size() == 0 && selected_vertex.size() == 0) {
			{
				Facet *fa = *selected_facet.begin();
				Facet *fb = *selected_facet.rbegin();
				if (fa->num_shared_edge(fb) >= 1) {
					if (ImGui::Button("Merge", ImVec2(250, 0))) {
						Facet::merge(fa, fb, &pts, &facets);
						clear_selection();
					}
				}

				if( fa->get_plane().h.dot_product( fb->get_plane().h) < 0 ) {
					if (ImGui::Button("Touch", ImVec2(250, 0))) {
						Plane pa = fa->get_plane();
						Plane pb = fb->get_plane();

						Vector3D h = (pa.h + -1 * pb.h).normalized();
						Point3D  p = ((pa.p.to_vector() + pb.p.to_vector()) / 2).to_point();

						fa->set_plane(Plane(p,h));
						fb->set_plane(Plane(p,-1*h));

						clear_selection();

						fa->planarize();
						fb->planarize();
					}
				}

				{
					if (ImGui::Button("Split by each other", ImVec2(250, 0))) {
						Line3D l;
						interp(fa->get_plane(), fb->get_plane(), &l, 0, 0);
						vector<pair<FacetEdge*, Point3D>> evs;
						fa->make_splitting_vertex_by_line(l, &evs);
						vector<FacetVertex*> vs;
						for (int i = 0; i < evs.size(); ++i) {
							if ((evs[i].first->get_u()->to_point3() - evs[i].second).length() < 0.01) {
								vs.push_back(evs[i].first->get_u());
								continue;
							}
							if ((evs[i].first->get_v()->to_point3() - evs[i].second).length() < 0.01) {
								vs.push_back(evs[i].first->get_v());
								continue;
							}
							Vertex* v = new Vertex(evs[i].second);
							world->vertices.push_back(v);
							fa->split_edge(evs[i].first, v);
							vs.push_back(fa->get_vertex(v));
						}

						if (vs.size() == 2) {
							fa->split_by_edge(vs[0], vs[1], &facets);
						}

						evs.clear();
						vs.clear();
						fb->make_splitting_vertex_by_line(l, &evs);
						for (int i = 0; i < evs.size(); ++i) {
							if ((evs[i].first->get_u()->to_point3() - evs[i].second).length() < 0.01) {
								vs.push_back(evs[i].first->get_u());
								continue;
							}
							if ((evs[i].first->get_v()->to_point3() - evs[i].second).length() < 0.01) {
								vs.push_back(evs[i].first->get_v());
								continue;
							}
							Vertex* v = new Vertex(evs[i].second);
							world->vertices.push_back(v);
							fb->split_edge(evs[i].first, v);
							vs.push_back(fb->get_vertex(v));
						}

						if (vs.size() == 2) {
							fb->split_by_edge(vs[0], vs[1], &facets);
						}
					}
				}

				/*if (fa->get_plane().h.dot_product(fb->get_plane().h) < 0) {
					if (ImGui::Button("Make Connection", ImVec2(250, 0))) {
						if (!next) {
							next = new ConnectionEditor(this, fa, fb);
							clear_selection();
						}
					}
				}*/
			}

		}

		if (selected_facet.size() == 0 && selected_edge.size() == 1 && selected_vertex.size() == 0) {
			FacetEdge *e = *selected_edge.begin();
			if (e->get_u()->get_vertex()->num_shared_facets(e->get_v()->get_vertex()) == 1) {
				if (ImGui::Button("Make Wall", ImVec2(250, 0))) {
					if (!next) {
						next = new WallMaker(this, e);
						clear_selection();
					}
				}
			}
			{
				FacetEdge *e_i = e;
				Facet *f = e->get_f();
				int hole_idx = -1;
				do {
					for (int i = 0; i < f->num_holes(); ++i) {
						if (f->get_hole_edge(i) == e_i) {
							hole_idx = i;
							break;
						}
					}
					e_i = e_i->next();
				} while (hole_idx == -1 && e_i != e);
				if (hole_idx != -1) {
					if (ImGui::Button("Remove Hole", ImVec2(250, 0))) {
						f->remove_hole(hole_idx);
						clear_selection();
					}
				}
			}
		}
		if (selected_facet.size() == 0 && selected_edge.size() == 1 && selected_vertex.size() == 1) {
			Vertex* u = (*selected_vertex.begin());
			FacetEdge* e = (*selected_edge.begin());

			if (u->num_shared_facets() == 1) {
				Facet *f = u->get_facet(0);
				//if (e->get_f() == f) {
				if (e->get_v()->next()->get_vertex() == u
					|| e->get_u()->prev()->get_vertex() == u) {

					if (ImGui::Button("Make Perpendicular", ImVec2(250, 0))) {
						Point3D p = u->to_point();
						Point3D e1 = e->get_u()->to_point3();
						Point3D e2 = e->get_v()->to_point3();
						Vector3D direction = (e2 - e1).normalized();
						Line3D l(e1, direction);
						scalar_t alpha;
						if (interp(p, l, &alpha)) {
							Point3D q = l.p + alpha * l.v;
							Vector3D perpdirection = p - q;
							if (e->get_v()->next()->get_vertex() == u) {
								p = e->get_v()->to_point3() + perpdirection;
							}
							else {
								p = e->get_u()->to_point3() + perpdirection;
							}

							Plane plane = f->get_plane();

							u->move_to(plane.project(p));
							f->triangulate();
						}

					}
				}
				//}
				else if (e->get_u()->get_vertex()->num_shared_facets(u) == 0) {
					if (ImGui::Button("Align Point to Line", ImVec2(250, 0))) {
						Point3D p = u->to_point();
						Point3D e1 = e->get_u()->to_point3();
						Point3D e2 = e->get_v()->to_point3();
						Vector3D direction = (e2 - e1);
						Line3D l(e1, direction);
						scalar_t alpha;
						if (interp(p, l, &alpha)) {
							p = l.p + alpha * l.v;
							Plane plane = f->get_plane();
							u->move_to(plane.project(p));
							f->triangulate();
						}
					}
				}
			}
		}
		if (selected_facet.size() == 0 && selected_edge.size() == 1 && selected_vertex.size() == 2) {
			Vertex* u = (*selected_vertex.begin());
			Vertex* v = (*selected_vertex.rbegin());
			FacetEdge* e = (*selected_edge.begin());

			if (u->num_shared_facets() == 1 && v->num_shared_facets() == 1 && u->get_facet(0) == v->get_facet(0) && (u->form_an_edge(v) || v->form_an_edge(u))) {

				if (ImGui::Button("Make Parallel", ImVec2(250, 0))) {
					Point3D p = u->to_point();
					Point3D q = v->to_point();

					Point3D c = ((u->to_vector() + v->to_vector()) / 2).to_point();
					length_t d = (q - p).length();
					Point3D e1 = e->get_u()->to_point3();
					Point3D e2 = e->get_v()->to_point3();
					Vector3D direction = (e2 - e1).normalized();

					Point3D p_new, q_new;

					if ((p - c).dot_product(direction) > 0) {
						p_new = c + direction * d / 2;
						q_new = c - direction * d / 2;
					}
					else {
						p_new = c - direction * d / 2;
						q_new = c + direction * d / 2;
					}

					Plane plane = u->get_facet(0)->get_plane();
					u->move_to(plane.project(p_new));
					v->move_to(plane.project(q_new));
					u->get_facet(0)->triangulate();
				}
			}
		}

		if (selected_facet.size() == 0 && selected_edge.size() == 0 && selected_vertex.size() == 2) {
			vector<Vertex*> vs(selected_vertex.begin(), selected_vertex.end());
			if (vs[0]->num_shared_facets(vs[1]) == 1) {
				Facet *f = vs[0]->get_a_shared_facet(vs[1]);
				if (f->num_holes() == 0) {
					if (ImGui::Button("Split", ImVec2(250, 0))) {
						f->split_by_edge(f->get_vertex(vs[0]), f->get_vertex(vs[1]), &facets);
						clear_selection();
					}
				}
			}
		}

		if (selected_facet.size() == 0 && selected_edge.size() == 0 && selected_vertex.size() == 3) {
			vector<Vertex*> vs(selected_vertex.begin(), selected_vertex.end());

			if (ImGui::Button("Make Triangle", ImVec2(250, 0))) {
				Facet* f = 0;

				Vector3D u = (vs[1]->to_point() - vs[0]->to_point());
				Vector3D v = (vs[2]->to_point() - vs[0]->to_point());
				Vector3D c(get_camera_pos() - ((vs[0]->to_vector() + vs[1]->to_vector() + vs[2]->to_vector()) / 3).to_point());
				
				if (!u.cross_product(v).is_zero()) {
					if (u.cross_product(v).dot_product(c) > 0) {
						f = Facet::create_facet(vs[0], vs[1], vs[2]);
					}
					else {
						f = Facet::create_facet(vs[0], vs[2], vs[1]);
					}

					f->triangulate();
					facets.push_back(f);
					clear_selection();
					//selected_facet.insert(f);
				}
			}
		}
		if (selected_facet.size() + selected_edge.size() + selected_vertex.size() >= 2) {
			if (ImGui::Button("Make a Bounding Box", ImVec2(250, 0))) {
				vector<Point3D> pts;
				
				for (auto j = selected_facet.begin(); j != selected_facet.end(); ++j) {
					Facet* f = *j;
					int n = f->num_vertices();
					for (int k = 0; k < n; ++k) {
						pts.push_back(f->get_vertex(k)->to_point3());
					}
				}
				for (auto j = selected_edge.begin(); j != selected_edge.end(); ++j) {
					FacetEdge* e = *j;
					pts.push_back(e->get_u()->to_point3());
					pts.push_back(e->get_v()->to_point3());
				}
				for (auto j = selected_vertex.begin(); j != selected_vertex.end(); ++j) {
					Vertex* v = *j;
					pts.push_back(v->to_point());
				}

				XYRotatedBoundingBox3D b;
				b.compute(pts);
				
				Point3D p = b.min;
				Point3D q = b.min + b.axis_u * b.xy.x;
				Point3D r = b.min + b.axis_v * b.xy.y;
				
				vector<Vertex*> vs;
				{
					vs.push_back(new Vertex(p));
					vs.push_back(new Vertex(p + b.axis_u * b.xy.x));
					vs.push_back(new Vertex(p + b.axis_u * b.xy.x + b.axis_v * b.xy.y));
					vs.push_back(new Vertex(p + b.axis_v * b.xy.y));
				}
				for (int i = 0; i < vs.size(); ++i) {
					this->pts.push_back(vs[i]);
				}
				Facet* f = Facet::create_facet(vs);
				f->triangulate();
				
				facets.push_back(f);

				if (!next) {
					cout << b.height << endl;
					next = new SolidMaker(this, f, b.height);
					clear_selection();
				}
			}
		}

		ImGui::Text("");
		ImGui::SliderFloat("Vertex Size", &vertex_size, 0.01, 0.2);
		ImGui::SliderInt("Edge Width", &edge_width, 1, 5);
		ImGui::End();
	}

	if (show_inspection_result) {
		ImGui::Begin("Inspection Report");
		if (!inspection_facets.empty()) {
			ImGui::Text("Facets");
			for (int i = 0; i < inspection_facets.size(); ++i) {
				//ImGui::Text("%X", inspection_facets[i]);
				//ImGui::SameLine();
				string label;
				{
					stringstream ss;
					ss << inspection_facets[i] << "##InspectSelection";
					label = ss.str();
				}

				if (ImGui::Button(label.c_str())) {
					for (int j = 0; j < facets.size(); ++j) {
						if (facets[j] == inspection_facets[i]) {
							
							if (!selected_facet.insert(inspection_facets[i]).second) {
								selected_facet.erase(inspection_facets[i]);
							}
							break;
						}
					}
				}
			}
		}

		if (!inspection_vertex.empty()) {
			ImGui::Text("Vertices");
			for (int i = 0; i < inspection_vertex.size(); ++i) {
				string label;
				{
					stringstream ss;
					ss << inspection_vertex[i] << "##InspectSelection";
					label = ss.str();
				}

				if (ImGui::Button(label.c_str())) {
					for (int j = 0; j < pts.size(); ++j) {
						if (pts[j] == inspection_vertex[i]) {
							if (!selected_vertex.insert(inspection_vertex[i]).second) {
								selected_vertex.erase(inspection_vertex[i]);
							}
							break;
						}
					}
				}
			}
		}

		if (ImGui::Button("Close")) {

			show_inspection_result = false;
		}
		ImGui::End();
	}






	ImGui::Begin("Selection");
	//if (ImGui::TreeNode("Selected Objects")) {
		if (ImGui::Button("Clear Selection")) {
			clear_selection();
		}
		ImGui::Text("Selected Facets : %d", selected_facet.size());
		if (!selected_facet.empty()) {
			ImGui::Columns(4);
			static bool facet_list_table_column = (ImGui::SetColumnWidth(0, 80.0f), ImGui::SetColumnWidth(1, 90.0f), ImGui::SetColumnWidth(2, 200.0f), false);

			ImGui::Text("*");
			ImGui::NextColumn();
			ImGui::Text("#Vertices");
			ImGui::NextColumn();
			ImGui::Text("Normal");
			ImGui::NextColumn();
			ImGui::Text("");
			ImGui::NextColumn();

			for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
				ImGui::Text("%X", (void*)(*i));
				ImGui::NextColumn();
				ImGui::Text("%d", (*i)->num_vertices());
				ImGui::NextColumn();
				ImGui::Text("(%0.3f,%0.3f,%0.3f)", (*i)->get_plane().h.x, (*i)->get_plane().h.y, (*i)->get_plane().h.z);
				ImGui::NextColumn();
				ImGui::Text("area=%0.3f", (*i)->area());
				/*
				if ((*i)->num_holes() == 0) {
					if (ImGui::Button("Split")) {
						if (!next) {
							next = new PolygonSpliter(this, (*i));
							clear_selection();
						}
					}
				}*/

				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		ImGui::Text("Selected Edges : %d", selected_edge.size());
		if(!selected_edge.empty()){
			ImGui::Columns(6);
			static bool edge_list_table_column = (ImGui::SetColumnWidth(0, 80.0f), ImGui::SetColumnWidth(1, 200.0f), ImGui::SetColumnWidth(2, 200.0f),
				ImGui::SetColumnWidth(3, 200.0f), ImGui::SetColumnWidth(4, 80.0f), false);

			ImGui::Text("*");
			ImGui::NextColumn();
			ImGui::Text("V1");
			ImGui::NextColumn();
			ImGui::Text("V2");
			ImGui::NextColumn();
			ImGui::Text("Direction");
			ImGui::NextColumn();
			ImGui::Text("Length");
			ImGui::NextColumn();
			ImGui::Text("");
			ImGui::NextColumn();

			for (auto i = selected_edge.begin(); i != selected_edge.end(); ++i) {
				ImGui::Text("%X", (void*)(*i));
				ImGui::NextColumn();
				Point3D u = (*i)->get_u()->to_point3();
				ImGui::Text("(%0.3f,%0.3f,%0.3f)", u.x, u.y, u.z);
				ImGui::NextColumn();
				Point3D v = (*i)->get_v()->to_point3();
				ImGui::Text("(%0.3f,%0.3f,%0.3f)", v.x, v.y, v.z);
				ImGui::NextColumn();
				Vector3D d = v - u;
				ImGui::Text("(%0.3f,%0.3f,%0.3f)", d.x, d.y, d.z);
				ImGui::NextColumn();
				ImGui::Text("%0.3f", d.length());
				ImGui::NextColumn();

				//ImGui::Button("Hello1");

				ImGui::SameLine();

				//ImGui::Button("Hello1");
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		ImGui::Text("Selected Vertices : %d", selected_vertex.size());
		if (!selected_vertex.empty()) {

			ImGui::Columns(4);
			static bool vertex_list_table_column = (ImGui::SetColumnWidth(0, 80.0f), ImGui::SetColumnWidth(1, 200.0f), ImGui::SetColumnWidth(2, 200.0f), false);

			ImGui::Text("*");
			ImGui::NextColumn();
			ImGui::Text("Pos");
			ImGui::NextColumn();
			ImGui::Text("#Facets");
			ImGui::NextColumn();
			ImGui::Text("");
			ImGui::NextColumn();

			for (auto i = selected_vertex.begin(); i != selected_vertex.end(); ++i) {
				ImGui::Text("%X", (void*)(*i));
				ImGui::NextColumn();
				Point3D u = (*i)->to_point();
				ImGui::Text("(%0.3f,%0.3f,%0.3f)", u.x, u.y, u.z);
				ImGui::NextColumn();
				ImGui::Text("%d", (*i)->num_shared_facets());

				/*
				for (int j = 0; j < (*i)->num_shared_facets(); ++j) {
					stringstream ss;
					ss << (j + 1);
					string label = ss.str();
					ImGui::SameLine();
					if (ImGui::Button(label.c_str())) {
						selected_facet.insert((*i)->get_facet(j));
					}
				}*/
				ImGui::NextColumn();

				//ImGui::Button("Hello1");

				ImGui::SameLine();

				//ImGui::Button("Hello1");
				ImGui::NextColumn();
			}

			ImGui::Columns(1);
		}
		//ImGui::TreePop();
	//}

	/*if (ImGui::TreeNode("Available Actions")) {

		if (ImGui::Button("Edit Polygons")) {
			if (!next) {
				Plane plane(Point3D(0, 0, 0), Vector3D(0, 0, 0));
				if (selected_facet.size() == 1) {
					plane = (*selected_facet.begin())->get_plane();
				}
				next = new PolygonEditor(this, plane);
				clear_selection();
			}
		}

		ImGui::TreePop();
	}*/
	ImGui::End();
}

struct ColorDrawer : public DrawSetting {
	float r, g, b, a;
	ColorDrawer(float _r, float _g, float _b, float _a = 1.0)
	:r(_r),g(_g),b(_b),a(_a){

	}
	void begin_facets(void) {}
	void begin_vertices(void) {}
	void begin_edges(void) {}

	bool begin_vertex(Vertex* v) {
		glColor4f(r, g, b, a);
		return true;
	}
	bool begin_facet(int, Facet* f) {
		glColor4f(r, g, b, a);
		return true;
	}
	bool begin_facetedge(int, FacetEdge* e) {
		glColor4f(r, g, b, a);
		return true;
	}
	bool begin_facetvertex(int, FacetVertex*) { 
		glColor4f(r, g, b, a);
		return true; 
	}
};

void MainViewer::draw_scene(void) {
	{
		SelectedObjectHighlighter ds(this);
		draw(&ds, vertex_size, edge_width);
	}

	{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		{
			ColorDrawer ds(1, 0, 0, 0.1);
			for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
				(*i)->draw(&ds);
			}
		}
		{
			ColorDrawer ds(0, 0, 1, 0.1);
			for (auto i = selected_vertex.begin(); i != selected_vertex.end(); ++i) {
				for (int j = 0; j < (*i)->num_shared_facets(); ++j) {
					(*i)->get_facet(j)->draw(&ds);
				}
			}
		}
		glPopAttrib();
	}
	{
		
		glPushAttrib(GL_ENABLE_BIT);
		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glLineWidth(1);

		{
			ColorDrawer ds(1, 0, 0, 1);
			for (auto i = selected_facet.begin(); i != selected_facet.end(); ++i) {
				(*i)->draw_edge(&ds);
			}
		}
		{
			ColorDrawer ds(0, 0, 1, 1);
			for (auto i = selected_vertex.begin(); i != selected_vertex.end(); ++i) {
				for (int j = 0; j < (*i)->num_shared_facets(); ++j) {
					(*i)->get_facet(j)->draw_edge(&ds);
				}
			}
		}
		glDepthFunc(GL_LESS);
		glPopAttrib();
	}
	{
		glPushAttrib(GL_ENABLE_BIT);
		{
			glEnable(GL_DEPTH_TEST);
			ColorDrawer ds(1, 0, 0, 1);
			for (auto i = selected_vertex.begin(); i != selected_vertex.end(); ++i) {
				(*i)->draw(vertex_size, &ds);
			}
		}
		
		{
			glDisable(GL_DEPTH_TEST);
			ColorDrawer ds(1, 0, 0, 0.1);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			for (auto i = selected_vertex.begin(); i != selected_vertex.end(); ++i) {
				(*i)->draw(vertex_size, &ds);
			}
		}
		glPopAttrib();
	}

	{
		glPushAttrib(GL_ENABLE_BIT);
		{
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xF0F0);
			glColor4f(0.3, 0, 0, 0.2);
			glLineWidth(edge_width);
			for (auto i = selected_edge.begin(); i != selected_edge.end(); ++i) {
				Point3D u = (*i)->get_u()->to_point3();
				Point3D v = (*i)->get_v()->to_point3();
				glBegin(GL_LINES);
				glVertex3f(u.x, u.y, u.z);
				glVertex3f(v.x, v.y, v.z);
				glEnd();
			}
		}
		glPopAttrib();
	}
}
void MainViewer::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	Selector sel(current_obj);
	draw(&sel, -1, -1);
	Facet*f;
	FacetEdge*e;
	
	switch (sel.selected_type) {
	case Selector::TYPE_VERTEX: 
		if (selected_vertex.count(pts[sel.selected_g]) == 0) {
			selected_vertex.insert(pts[sel.selected_g]);
		}
		else {
			selected_vertex.erase(pts[sel.selected_g]);
		}
		break;

	case Selector::TYPE_EDGE: 
		if (0 <= sel.selected_g && sel.selected_g < facets.size() &&
			0 <= sel.selected_i && sel.selected_i < facets[sel.selected_g]->num_edges()) {
			e = facets[sel.selected_g]->get_edge(sel.selected_i);
			if (selected_edge.count(e) == 0) {
				selected_edge.insert(e);
			}
			else {
				selected_edge.erase(e);
			}
		}
		break;

	case Selector::TYPE_FACET:
		if (selected_facet.count(facets[sel.selected_g]) == 0) {
			selected_facet.insert(facets[sel.selected_g]);
		}
		else {
			selected_facet.erase(facets[sel.selected_g]);
		}
		
	default:;
	}
}

void MainViewer::post_draw(void) {
	if (next) {
		Controller::current_controller = next;
		next = 0;
	}
}



PolygonSpliter::PolygonSpliter(Controller *ctrl, Facet *f)
	:Controller(ctrl), target_facet(f), finalized(false), canceled(false), selected_i(-1), start_v(0), start_e(0), end_v(0), end_e(0), facet_alpha(0.2) {

}
void PolygonSpliter::make_ui(void) {
	ImGui::Begin("Polygon Split");

	ImGui::SliderFloat("BgFacet Opacity", &facet_alpha, 0.0, 1.0);

	if ((start_v || start_e) && (end_v || end_e)) {
		if (ImGui::Button("Finalize", ImVec2(250, 0))) {
			finalized = true;
		}
	}

	if (ImGui::Button("Cancel", ImVec2(250, 0))) {
		canceled = true;
	}

	ImGui::End();
}
void PolygonSpliter::draw(DrawSetting* d, float vertex_r, float edge_w) {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	target_facet->draw(d);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glDepthMask(GL_FALSE);
	glLineWidth(edge_w);
	glColor3f(1, 0.5, 0.5);
	target_facet->draw_edge(d);
	glDepthMask(GL_TRUE);
	glPointSize(5);
	int n = target_facet->num_vertices();
	for (int i = 0; i < n; ++i) {
		Vertex *v = target_facet->get_vertex(i)->get_vertex();

		glColor3f(1, 1, 1);
		v->draw(vertex_r, d);
	}
}
void PolygonSpliter::draw_select_scene(void) {
	Indexer ds;
	draw(&ds, vertex_size, edge_width);
}
void PolygonSpliter::draw_scene(void) {
	{
		Highlighter ds(selected_i);
		draw(&ds, vertex_size, edge_width);
	}

	{
		ColorDrawer ds_g(0, 0.5, 0, facet_alpha);
		ColorDrawer ds_b(0, 0, 0, facet_alpha);
		const vector<Facet*>& facets = Controller::world->facets;
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glLineWidth(1);
		for (int i = 0; i < facets.size(); ++i) {
			if (facets[i] == target_facet) continue;
			facets[i]->draw(&ds_g);
			facets[i]->draw_edge(&ds_b);
		}
		glDepthMask(GL_TRUE);
		glPopAttrib();
	}

	{
		Highlighter ds(-1);
		for (int i = 0; i < mid_points.size(); ++i) {
			Vertex v(mid_points[i].x, mid_points[i].y, mid_points[i].z);
			v.draw(vertex_size, &ds);
		}

		glColor3f(1, 1, 1);
		glLineWidth(edge_width);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < mid_points.size(); ++i) {
			glVertex3f(mid_points[i].x, mid_points[i].y, mid_points[i].z);
		}
		glEnd();
	}
	if (selected_i != -1) {
		Highlighter ds(-1);
		Vertex v(hover_p.x, hover_p.y, hover_p.z);
		v.draw(vertex_size, &ds);

		if (!mid_points.empty()) {
			glColor3f(1, 1, 1);
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1, 0xF0F0);
			glBegin(GL_LINES);
			int m = mid_points.size() - 1;
			glVertex3f(mid_points[m].x, mid_points[m].y, mid_points[m].z);
			glVertex3f(hover_p.x, hover_p.y, hover_p.z);
			glEnd();
			glDisable(GL_LINE_STIPPLE);
		}
	}
}
void PolygonSpliter::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	Selector sel(current_obj);
	draw(&sel, vertex_size, edge_width);

	if (sel.selected_type == Selector::TYPE_VERTEX || sel.selected_type == Selector::TYPE_EDGE) {
		if (!start_v && !start_e) {
			if (sel.selected_type == Selector::TYPE_VERTEX) { 
				start_v = target_facet->get_vertex(sel.selected_g)->get_vertex();

				mid_points.push_back(start_v->to_point());
			}
			else {
				start_e = target_facet->get_edge(sel.selected_i);

				scalar_t alpha;
				interp(ray, target_facet->get_plane(), &alpha);
				Point3D p = ray.p + alpha * ray.v;
				mid_points.push_back(p);
			}
		}
		else if (!end_v && !end_e) {
			if (sel.selected_type == Selector::TYPE_VERTEX) {
				end_v = target_facet->get_vertex(sel.selected_g)->get_vertex();;
				
				mid_points.push_back(end_v->to_point());
			}
			else {
				end_e = target_facet->get_edge(sel.selected_i);

				scalar_t alpha;
				interp(ray, target_facet->get_plane(), &alpha);
				Point3D p = ray.p + alpha * ray.v;
				mid_points.push_back(p);
			}
		}
	}

	else if (sel.selected_type == Selector::TYPE_FACET) {
		if ((start_v || start_e) && (!end_v && !end_e)) {
			scalar_t alpha;
			interp(ray, target_facet->get_plane(), &alpha);
			Point3D p = ray.p + alpha * ray.v;
			mid_points.push_back(p);
		}
	}
}
void PolygonSpliter::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void PolygonSpliter::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {
	Selector sel(current_obj);
	draw(&sel, vertex_size, edge_width);

	selected_i = -1;

	if (sel.selected_type == Selector::TYPE_VERTEX || sel.selected_type == Selector::TYPE_EDGE) {
		if ((!start_v && !start_e) || (!end_v && !end_e)) {
			selected_i = current_obj;
			if (sel.selected_type == Selector::TYPE_VERTEX) {
				hover_p = target_facet->get_vertex(sel.selected_g)->to_point3();
			}
			else {
				scalar_t alpha;
				interp(ray, target_facet->get_plane(), &alpha);
				hover_p = ray.p + alpha * ray.v;
			}
		}
	}

	if (sel.selected_type == Selector::TYPE_FACET) {
		if ((start_v || start_e) && (!end_v && !end_e)) {
			selected_i = current_obj;

			scalar_t alpha;
			interp(ray, target_facet->get_plane(), &alpha);
			hover_p = ray.p + alpha * ray.v;
		}
	}
}
void PolygonSpliter::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}

void PolygonSpliter::post_draw(void) {
	if (finalized) {
		if (start_e) {
			Vertex *v_s = new Vertex(mid_points[0].x, mid_points[0].y, mid_points[0].z);
			target_facet->split_edge(start_e, v_s);
			world->vertices.push_back(v_s);
			start_v = v_s;
		}
		if (end_e) {
			int m = mid_points.size() - 1;
			Vertex *v_e = new Vertex(mid_points[m].x, mid_points[m].y, mid_points[m].z);
			target_facet->split_edge(end_e, v_e);
			world->vertices.push_back(v_e);
			end_v = v_e;
		}

		vector<Facet*> ret;
		if (!target_facet->split_by_edge(target_facet->get_vertex(start_v), target_facet->get_vertex(end_v), &ret)) {
			end();
			return;
		}
		world->facets.push_back(ret[0]);
		
		vector<Vertex*> mps;
		for (int i = 1; i < mid_points.size() - 1; ++i) {
			Vertex *v = new Vertex(mid_points[i].x, mid_points[i].y, mid_points[i].z);
			mps.push_back(v);
			world->vertices.push_back(v);
		}
		{
			FacetVertex * v = target_facet->get_vertex(end_v);
			for (int i = 0; i < mps.size(); ++i) {
				target_facet->split_edge(v->outgoing_edge(), mps[i]);
			}
		}
		target_facet->triangulate();

		target_facet = ret[0];
		{
			FacetVertex * v = target_facet->get_vertex(end_v);
			for (int i = 0; i < mps.size(); ++i) {
				target_facet->split_edge(v->incoming_edge(), mps[i]);
			}
		}
		target_facet->triangulate();
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}



WallMaker::WallMaker(Controller *ctrl, FacetEdge *e)
	:BasicViewSelector(ctrl), base_edge(e), finalized(false), canceled(false), height(0.1) {
	;
}
void WallMaker::make_ui(void) {
	ImGui::Begin("Making Wall");
	if( ImGui::Button("OK", ImVec2(250,0)) ) finalized = true;
	if( ImGui::Button("Cancel", ImVec2(250, 0)) ) canceled = true;
	ImGui::End();
}
void WallMaker::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	last_y = y;
}
void WallMaker::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	height += (last_y-y)*0.01;
	last_y = y;
}
void WallMaker::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void WallMaker::post_draw(void) {
	if (finalized) {
		Point3D p = base_edge->get_u()->to_point3();
		Point3D q = base_edge->get_v()->to_point3();
		Vector3D planeh = base_edge->get_f()->get_plane().h;
		Point3D ph = p + height * planeh;
		Point3D qh = q + height * planeh;

		Vertex *up = base_edge->get_u()->get_vertex();
		Vertex *vp = base_edge->get_v()->get_vertex();
		Vertex *new_v1 = new Vertex(ph.x, ph.y, ph.z);
		Vertex *new_v2 = new Vertex(qh.x, qh.y, qh.z);

		Controller::world->vertices.push_back(new_v1);
		Controller::world->vertices.push_back(new_v2);

		Facet *new_f = Facet::create_facet(up, new_v1, new_v2, vp);
		new_f->triangulate();
		Controller::world->facets.push_back(new_f);

		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

void WallMaker::draw_scene(void) {
	BasicViewSelector::draw_scene();
	Point3D p = base_edge->get_u()->to_point3();
	Point3D q = base_edge->get_v()->to_point3();
	Vector3D planeh = base_edge->get_f()->get_plane().h;

	Point3D ph = p + height * planeh;
	Point3D qh = q + height * planeh;

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_BACK, GL_LINE);
	glColor3f(0.5, 1, 0.5);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(p.x, p.y, p.z);
	glVertex3f(ph.x, ph.y, ph.z);
	glVertex3f(qh.x, qh.y, qh.z);
	glVertex3f(q.x, q.y, q.z);
	glEnd();
	glEnable(GL_CULL_FACE);
}




PolygonLifter::PolygonLifter(Controller *ctrl, Facet *f)
	:BasicViewSelector(ctrl), base_facet(f), finalized(false), canceled(false), height(0) {
	;
}
void PolygonLifter::make_ui(void) {
	ImGui::Begin("Polygon Lifting");
	ImGui::DragFloat("Height##Lift", &this->height, 0.01);
	if (ImGui::Button("OK", ImVec2(250, 0))) finalized = true;
	if (ImGui::Button("Cancel", ImVec2(250, 0))) canceled = true;
	ImGui::End();
}
void PolygonLifter::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	last_y = y;
}
void PolygonLifter::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	height += (last_y - y)*0.01;
	last_y = y;
}
void PolygonLifter::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void PolygonLifter::post_draw(void) {
	if (finalized) {
		Vector3D h = height * base_facet->get_plane().h;
		//base_facet->make_solid(h, &Controller::world->vertices, &Controller::world->facets);
		Plane newplane(base_facet->get_plane().p + h, base_facet->get_plane().h);
		vector<Point3D> pts_before; pts_before.reserve(base_facet->num_vertices());
		vector<Point3D> pts_after; pts_after.reserve(base_facet->num_vertices());
		bool failed = false;
		for (int i = 0; i < base_facet->num_vertices(); ++i) {
			FacetVertex *fv = base_facet->get_vertex(i);
			pts_before.push_back(fv->to_point3());
			SubspaceFilter filter;
			Vertex *v = fv->get_vertex();
			
			for (int j = 0; j < v->num_shared_facets(); ++j) {
				Facet *f = v->get_facet(j);
				//cout << "SHARED FACET " << (f == base_facet) << endl;
				if (f == base_facet) continue;
				filter.add_plane(f->get_plane(), true);
			}
			
			filter.add_plane(newplane, true);
			Point3D newpt = fv->to_point3();
			//cout << filter.dof << endl;
			if (!filter.projection(fv->to_point3(), &newpt)) {
				cout << "FAILED" << endl;
				failed = true;
				break;
			}
			//cout << h.z << '\t' << fv->to_point3().z << ' ' << newpt.z << endl;
			pts_after.push_back(newpt);
		}

		if (!failed) {
			//cout << "LIFTED" << endl;
			set<Facet*> adj_facets;
			for (int i = 0; i < base_facet->num_vertices(); ++i) {
				FacetVertex *fv = base_facet->get_vertex(i);
				Vertex *v = fv->get_vertex();
				v->move_to(pts_after[i]);
				for (int j = 0; j < v->num_shared_facets(); ++j) {
					Facet *f = v->get_facet(j);
					adj_facets.insert(f);
				}
			}

			for (auto i = adj_facets.begin(); i != adj_facets.end(); ++i) {
				(*i)->triangulate();
			}

			base_facet->set_plane(newplane);
		}

		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

void PolygonLifter::draw_scene(void) {
	BasicViewSelector::draw_scene();
	Vector3D h = height * base_facet->get_plane().h;
	glPushMatrix();
	glTranslatef(h.x, h.y, h.z);
	base_facet->draw_edge(0);
	glPopMatrix();
}







MultiPolygonLifter::MultiPolygonLifter(Controller* ctrl, const vector<Facet*>& fs)
	:BasicViewSelector(ctrl), base_facet(fs), finalized(false), canceled(false), height(0) {
	Vector3D h_(0, 0, 0);
	for (int i = 0; i < fs.size(); ++i) {
		Facet* f = fs[i];
		h_ = h_ + f->area() * f->get_plane().h;
	}
	h = h_.normalized();
}
void MultiPolygonLifter::make_ui(void) {
	ImGui::Begin("Polygon Lifting");
	ImGui::Text("Direction");
	ImGui::DragFloat("X##Lift", &this->h.x, 0.01);
	ImGui::DragFloat("Y##Lift", &this->h.y, 0.01);
	ImGui::DragFloat("Z##Lift", &this->h.z, 0.01);
	ImGui::Text("Height");
	ImGui::DragFloat("Height##Lift", &this->height, 0.01);
	ImGui::Text("");
	if (ImGui::Button("OK", ImVec2(250, 0))) finalized = true;
	if (ImGui::Button("Cancel", ImVec2(250, 0))) canceled = true;
	ImGui::End();
}
void MultiPolygonLifter::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	last_y = y;
}
void MultiPolygonLifter::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	height += (last_y - y) * 0.01;
	last_y = y;
}
void MultiPolygonLifter::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void MultiPolygonLifter::post_draw(void) {
	if (finalized) {
		set<Facet*> lifting_facets(base_facet.begin(), base_facet.end());

		for (int i = 0; i < base_facet.size(); ++i) {
			Vector3D h = height * base_facet[i]->get_plane().h;
			//base_facet->make_solid(h, &Controller::world->vertices, &Controller::world->facets);
			Plane newplane(base_facet[i]->get_plane().p + h, base_facet[i]->get_plane().h);
			vector<Point3D> pts_before; pts_before.reserve(base_facet[i]->num_vertices());
			vector<Point3D> pts_after; pts_after.reserve(base_facet[i]->num_vertices());
			bool failed = false;
			for (int j = 0; j < base_facet[i]->num_vertices(); ++j) {
				FacetVertex* fv = base_facet[i]->get_vertex(j);
				pts_before.push_back(fv->to_point3());
				SubspaceFilter filter;
				Vertex* v = fv->get_vertex();

				for (int k = 0; k < v->num_shared_facets(); ++k) {
					Facet* f = v->get_facet(k);
					//cout << "SHARED FACET " << (f == base_facet) << endl;
					if (lifting_facets.find(f) != lifting_facets.end()) continue;
					filter.add_plane(f->get_plane(), true);
				}

				filter.add_plane(newplane, true);
				Point3D newpt = fv->to_point3();
				//cout << filter.dof << endl;
				if (!filter.projection(fv->to_point3(), &newpt)) {
					cout << "FAILED" << endl;
					failed = true;
					break;
				}
				//cout << h.z << '\t' << fv->to_point3().z << ' ' << newpt.z << endl;
				pts_after.push_back(newpt);
			}

			if (!failed) {
				//cout << "LIFTED" << endl;
				set<Facet*> adj_facets;
				for (int j = 0; j < base_facet[i]->num_vertices(); ++j) {
					FacetVertex* fv = base_facet[i]->get_vertex(j);
					Vertex* v = fv->get_vertex();
					v->move_to(pts_after[j]);
					for (int k = 0; k < v->num_shared_facets(); ++k) {
						Facet* f = v->get_facet(k);
						adj_facets.insert(f);
					}
				}

				for (auto j = adj_facets.begin(); j != adj_facets.end(); ++j) {
					(*j)->triangulate();
				}

				base_facet[i]->set_plane(newplane);
			}
		}
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

void MultiPolygonLifter::draw_scene(void) {
	BasicViewSelector::draw_scene();

	Vector3D h_ = height * h;
	glPushMatrix();
	glTranslatef(h_.x, h_.y, h_.z);
	for (int i = 0; i < base_facet.size(); ++i) {
		base_facet[i]->draw_edge(0);
	}
	glPopMatrix();
}










SolidMaker::SolidMaker(Controller *ctrl, Facet *f, length_t h)
	:BasicViewSelector(ctrl), base_facet(f), finalized(false), canceled(false), height(h) {
	;
}
void SolidMaker::make_ui(void) {
	ImGui::Begin("Extrude to 2.5D Solid");
	if (ImGui::Button("OK", ImVec2(250, 0))) finalized = true;
	if (ImGui::Button("Cancel", ImVec2(250, 0))) canceled = true;
	ImGui::End();
}
void SolidMaker::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	last_y = y;
}
void SolidMaker::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	height += (last_y - y)*0.01;
	last_y = y;
}
void SolidMaker::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void SolidMaker::post_draw(void) {
	if (finalized) {
		Vector3D h = height * base_facet->get_plane().h;
		base_facet->make_solid(h, &Controller::world->vertices, &Controller::world->facets);
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

void SolidMaker::draw_scene(void) {
	BasicViewSelector::draw_scene();
	Vector3D h = height*base_facet->get_plane().h;
	glPushMatrix();
	glTranslatef(h.x, h.y, h.z);
	base_facet->draw_edge(0);
	glPopMatrix();
}








PolygonEditor::PolygonEditor(Controller*ctrl, const Plane& _baseplane)
	: Controller(ctrl), pts(Controller::world->vertices), facets( Controller::world->facets), finalized(false), baseplane(_baseplane)
{
	selected_i = -1;

	clicked_v = 0;
	clicked_f = 0;


	if (!_baseplane.h.is_zero()) {

		coord_t minx = FLT_MAX;
		coord_t maxx = -FLT_MAX;
		coord_t miny = FLT_MAX;
		coord_t maxy = -FLT_MAX;
		coord_t minz = FLT_MAX;
		coord_t maxz = -FLT_MAX;
		for (int i = 0; i < world->vertices.size(); ++i) {
			Point3D p = world->vertices[i]->to_point();
			if (p.x < minx) minx = p.x;
			if (p.x > maxx) maxx = p.x;
			if (p.y < miny) miny = p.y;
			if (p.y > maxy) maxy = p.y;
			if (p.z < minz) minz = p.z;
			if (p.z > maxz) maxz = p.z;
		}

		{
			baseplane.get_basis(&base_u, &base_v);
		}
	}
}

void PolygonEditor::make_ui(void) {
	ImGui::Begin("Polygon Editing");
	
	
	if (ImGui::Button("OK", ImVec2(250, 0))) {
		finalized = true;
	}

	ImGui::End();
}
void PolygonEditor::draw(DrawSetting* d, float vertex_r, float edge_w) {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	for (int i = 0; i < facets.size(); ++i) {
		if (i + pts.size() == selected_i) {
			glColor3f(0.5, 0.5, 1);
		}
		else {
			glColor3f(0.5, 0.5, 0.5);
		}
		facets[i]->draw(d);
	}
	glDisable(GL_POLYGON_OFFSET_FILL);

	glDepthMask(GL_FALSE);
	glLineWidth(edge_w);
	glColor3f(1, 0.5, 0.5);
	for (int i = 0; i < facets.size(); ++i) {
		facets[i]->draw_edge(d);
	}

	glDepthMask(GL_TRUE);
	glPointSize(5);
	for (int i = 0; i < pts.size(); ++i) {
		if (i == selected_i) {
			glColor3f(1, 0, 0);
		}
		else {
			glColor3f(1, 1, 1);
		}

		if (pts[i] == clicked_v) {
			pts[i]->draw(-1, d);
		}
		else {
			pts[i]->draw(vertex_r, d);
		}
	}
}
void PolygonEditor::draw_select_scene(void) {
	Indexer idx;
	draw(&idx, vertex_size, edge_width);
}
void PolygonEditor::draw_scene(void) {
	if (!baseplane.h.is_zero()) {
		glColor3f(0, 0.5, 0);
		glLineWidth(1.0f);
		glBegin(GL_LINES);
		for (int i = -100; i <= 100; ++i) {
			{
				Point3D p = baseplane.p + i * base_u - 100 * base_v;
				Point3D q = baseplane.p + i * base_u + 100 * base_v;
				glVertex3f(p.x, p.y, p.z);
				glVertex3f(q.x, q.y, q.z);
			}
			{
				Point3D p = baseplane.p + i * base_v - 100 * base_u;
				Point3D q = baseplane.p + i * base_v + 100 * base_u;
				glVertex3f(p.x, p.y, p.z);
				glVertex3f(q.x, q.y, q.z);
			}
		}
		glEnd();
	}

	Highlighter ds(selected_i, 0);
	draw(&ds, vertex_size, edge_width);

	Selector sel(selected_i);
	draw(&sel, vertex_size, edge_width);
	if (sel.selected_type == Selector::TYPE_VERTEX) {
		Vertex *v = pts[sel.selected_g];

		for (int i = 0; i < v->num_shared_facets(); ++i) {
			Facet *f = v->get_facet(i);
			glPushAttrib(GL_ENABLE_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			ColorDrawer d_face(0, 0, 1, 0.1);
			f->draw(&d_face);
			glPopAttrib();

			ColorDrawer d_edge(0, 0, 1);
			glLineStipple(1, 0xF0F0);
			glLineWidth(1);
			glPushAttrib(GL_ENABLE_BIT);
			glEnable(GL_LINE_STIPPLE);
			glDisable(GL_DEPTH_TEST);
			f->draw_edge(&d_edge);
			glPopAttrib();
			glPushAttrib(GL_ENABLE_BIT);
			glEnable(GL_DEPTH_TEST);
			f->draw_edge(&d_edge);
			glPopAttrib();
		}

	}
	else if (sel.selected_type == Selector::TYPE_EDGE) {
		FacetEdge *e = facets[sel.selected_g]->get_edge(sel.selected_i);

		Facet *f = e->get_f();;
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		ColorDrawer d(0, 0, 1, 0.1);
		f->draw(&d);
		glPopAttrib();


		ColorDrawer d_edge(0, 0, 1);
		glLineStipple(1, 0xF0F0);
		glLineWidth(1);
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_LINE_STIPPLE);
		glDisable(GL_DEPTH_TEST);
		f->draw_edge(&d_edge);
		glPopAttrib();
		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_DEPTH_TEST);
		f->draw_edge(&d_edge);
		glPopAttrib();
	}
	else if (sel.selected_type == Selector::TYPE_FACET) {
		Facet *f = facets[sel.selected_g];
	}
}

void PolygonEditor::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	selected_i = current_obj;
	Selector sel(selected_i);
	draw(&sel, 0, 0);
	if (selected_i == 0) {
		scalar_t alpha;
		if (interp(ray, baseplane, &alpha) && alpha > 0) {
			Point3D p = ray.p + alpha * ray.v;
			p = p - (base_u + base_v) / 2;
			Point3D q = p + base_u;
			Point3D r = p + base_v;
			Point3D s = p + base_u + base_v;
			Vertex *v1 = new Vertex(p.x, p.y, p.z);
			Vertex *v2 = new Vertex(q.x, q.y, q.z);
			Vertex *v3 = new Vertex(r.x, r.y, r.z);
			Vertex *v4 = new Vertex(s.x, s.y, s.z);
			world->vertices.push_back(v1);
			world->vertices.push_back(v2);
			world->vertices.push_back(v3);
			world->vertices.push_back(v4);
			Facet*f = Facet::create_facet(v1, v2, v4, v3);
			f->triangulate();
			world->facets.push_back(f);
		}
	}
	if (sel.selected_type == Selector::TYPE_EDGE) {
		FacetEdge *e = facets[sel.selected_g]->get_edge(sel.selected_i);

		//if(!e->is_shared()) {
			scalar_t alpha;
			interp(ray, facets[sel.selected_g]->get_plane(), &alpha);
			Point3D p = ray.p + alpha * ray.v;
			Vertex *v = new Vertex(p.x, p.y, p.z);
			facets[sel.selected_g]->split_edge(sel.selected_i, v);
			facets[sel.selected_g]->triangulate();

			clicked_f = facets[sel.selected_g];
			clicked_v = v;
			pts.push_back(clicked_v);
		//}
	}
	if (sel.selected_type == Selector::TYPE_VERTEX) {
		clicked_v = pts[sel.selected_g];
	}
}
void PolygonEditor::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	selected_i = current_obj;
	scalar_t alpha;
	if (clicked_v) {
		Selector sel(current_obj);
		draw(&sel, 0, 0);
		if (sel.selected_type == Selector::TYPE_VERTEX) {
			Vertex *v = pts[sel.selected_g];
			Point3D p;
			Plane plane(clicked_v->to_point(), Vector3D(0, 0, 1));
			if (v != clicked_v && clicked_v->join(v) && clicked_v->absorb(v)) {
				clicked_v->triangulate_associated_facets();
				for (int i = 0; i < pts.size(); ++i) {
					if (v == pts[i]) {
						pts.erase(pts.begin() + i);
						delete v;
					}
				}
				return;
			}
		}
		else {
			if (sel.selected_type == Selector::TYPE_EDGE) {
				FacetEdge* e = facets[sel.selected_g]->get_edge(sel.selected_i);
				if (e->get_u()->get_vertex() != clicked_v && e->get_v()->get_vertex() != clicked_v) {

					Line3D l(e->get_u()->to_point3(), e->to_vector3());
					Point3D p = clicked_v->to_point();
					scalar_t alpha;
					if (interp(p, l, &alpha)) {
						p = l.p + alpha * l.v;
						Point3D p_ret;
						if (clicked_v->get_feasible_point_nearest_to_point(p, &p_ret)) {
							clicked_v->move_to(p_ret);
							clicked_v->triangulate_associated_facets();
						}
						return;
					}
				}
			}
		}

		{
			Plane plane(clicked_v->to_point(), Vector3D(0, 0, 1));
			clicked_v->move_to_ray(ray, &plane);

			//
			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyShift) {
				scalar_t min_cos_th = 1;
				Point3D org_ptr = clicked_v->to_point();
				Point3D ptr = clicked_v->to_point();
				if (io.KeyCtrl) {
					for (int i = 0; i < clicked_v->num_shared_facets(); ++i) {
						Facet *f = clicked_v->get_facet(i);
						FacetVertex *fv = f->get_vertex(clicked_v);

						scalar_t cos_th;

						Vector3D ppv = fv->prev()->incoming_edge()->to_vector3();
						Vector3D pv = fv->incoming_edge()->to_vector3();
						cos_th = abs(pv.normalized().dot_product(ppv.normalized()));

						Vector3D nnv = fv->next()->outgoing_edge()->to_vector3();
						Vector3D nv = fv->outgoing_edge()->to_vector3();
						cos_th += abs(nv.normalized().dot_product(nnv.normalized()));
						
						if (cos_th < min_cos_th) {
							min_cos_th = cos_th;
							
							Vector3D hp = f->get_plane().h.cross_product(ppv).normalized();
							Vector3D q_p = fv->next()->to_point3() - fv->prev()->to_point3();
							Vector3D u = nnv.normalized();
							if (abs(hp.dot_product(u)) < EPSILON) {
								ptr = fv->prev()->to_point3() + 0.5 * q_p;
							}
							else {
								alpha = q_p.dot_product(u) / hp.dot_product(u);
								ptr = fv->prev()->to_point3() + alpha * hp;
							}
						}
					}
				}
				else {
					for (int i = 0; i < clicked_v->num_shared_facets(); ++i) {
						Facet *f = clicked_v->get_facet(i);
						FacetVertex *fv = f->get_vertex(clicked_v);

						scalar_t cos_th;

						Vector3D ppv = fv->prev()->incoming_edge()->to_vector3();
						Vector3D pv = fv->incoming_edge()->to_vector3();
						cos_th = abs(pv.normalized().dot_product(ppv.normalized()));
						if (cos_th < min_cos_th) {
							min_cos_th = cos_th;
							ptr = org_ptr - pv.get_projection_component_to(ppv);
						}

						Vector3D nnv = fv->next()->outgoing_edge()->to_vector3();
						Vector3D nv = fv->outgoing_edge()->to_vector3();
						cos_th = abs(nv.normalized().dot_product(nnv.normalized()));
						if (cos_th < min_cos_th) {
							min_cos_th = cos_th;
							ptr = org_ptr + nv.get_projection_component_to(nnv);
						}
					}
				}

				if (clicked_v->get_feasible_point_nearest_to_point(ptr, &org_ptr)) {
					clicked_v->move_to(org_ptr);
				}
			}
			//
			clicked_v->triangulate_associated_facets();
		}
	}
}
void PolygonEditor::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {
	selected_i = current_obj;
}
void PolygonEditor::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {
	if (clicked_v) {
		Selector sel(current_obj);
		draw(&sel, 0, 0);
		if (sel.selected_type == Selector::TYPE_VERTEX) {
			Vertex *v = pts[sel.selected_g];
			Point3D p;
			Plane plane(clicked_v->to_point(), Vector3D(0, 0, 1));
			if (v != clicked_v && clicked_v->get_feasible_point_nearest_to_point(v->to_point(), &p, &plane)
				&& (v->to_point() - p).is_zero()) {

				if (v->absorb(clicked_v)) {
					for (int i = 0; i < pts.size(); ++i) {
						if (clicked_v == pts[i]) {
							pts.erase(pts.begin() + i);
							delete clicked_v;
							clicked_f = 0;
							clicked_v = 0;
							return;
						}
					}
				}
				else if (clicked_v->absorb(v)) {
					for (int i = 0; i < pts.size(); ++i) {
						if (v == pts[i]) {
							pts.erase(pts.begin() + i);
							delete v;
							break;
						}
					}
				}
			}
		}
	}

	clicked_f = 0;
	clicked_v = 0;
}
void PolygonEditor::post_draw(void) {
	if (clicked_f && clicked_v && ImGui::IsKeyPressed(GLFW_KEY_P)) {
		Point3D p = clicked_v->to_point();
		Point3D q = clicked_f->get_vertex(clicked_v)->next()->to_point3();
		Point3D r = p + clicked_f->get_plane().h;
		Vertex *u = new Vertex(q.x, q.y, q.z);
		Vertex *v = new Vertex(r.x, r.y, r.z);
		Facet*f = Facet::create_facet(clicked_v, v, u);
		if (f) {
			pts.push_back(u);
			pts.push_back(v);
			f->triangulate();
			facets.push_back(f);
		}
		else {
			delete u; delete v;
		}
		if (clicked_f && clicked_v) {
			pts.push_back(clicked_v);
		}
		clicked_f = 0;
		clicked_v = 0;

	}
	if (clicked_v && ImGui::IsKeyPressed(GLFW_KEY_DELETE)) {
		if (clicked_v->remove()) {
			for (int i = 0; i < pts.size(); ++i) {
				if (pts[i] == clicked_v) {
					pts.erase(pts.begin() + i);
					break;
				}
			}
		}
		else if( clicked_v->num_shared_facets() == 1 ) {
			Facet *f = clicked_v->get_a_shared_facet(0);
			for (int i = 0; i < world->facets.size(); ++i) {
				if (world->facets[i] == f) {
					f->release(&world->vertices);
					world->facets.erase(world->facets.begin() + i);
					break;
				}
			}
		}
		clicked_f = 0;
		clicked_v = 0;
	}

	if (finalized) {
		end();
	}
}




ConnectionEditor::ConnectionEditor(Controller *ctrl, Facet *f1, Facet *f2)
	:BasicViewSelector(ctrl), base_facet(f1), opposite_facet(f2), finalized(false), canceled(false), selected_i(-1), door(true) {
	c.fbase = f1;
	c.fbase_opposite = f2;
}
void ConnectionEditor::make_ui(void) {
	ImGui::Begin("Connection Editor");

	//ImGui::Checkbox("Door", &door);

	if (ImGui::Button("OK", ImVec2(250, 0))) finalized = true;
	if (ImGui::Button("Cancel", ImVec2(250, 0))) canceled = true;
	ImGui::End();
}
void ConnectionEditor::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	scalar_t alpha;
	if (!interp(ray, base_facet->get_plane(), &alpha)) alpha = 0;

	if (!opposite_facet) {
		c.ring.push_back(ray.p + alpha * ray.v);
		return;
	}

	scalar_t beta;
	if (!interp(ray, opposite_facet->get_plane(), &beta)) beta = 0;

	if (alpha <= 0.1 && beta <= 0.1) return;

	if (beta <= 0.1 || alpha < beta) {
		c.ring.push_back(ray.p + alpha * ray.v);
	}

	if (alpha <= 0.1 || beta < alpha) {
		Point3D x = ray.p + beta * ray.v;
		Line3D l; l.p = x; l.v = opposite_facet->get_plane().h;
		interp(l, base_facet->get_plane(), &beta);
		c.ring.push_back(l.p + beta * l.v);
	}
}
void ConnectionEditor::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionEditor::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}
void ConnectionEditor::post_draw(void) {
	if (finalized) {
		Controller::world->connections.push_back(c);
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

void ConnectionEditor::draw_scene(void) {
	BasicViewSelector::draw_scene();
	if (c.ring.empty()) return;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);

	glPointSize(10);
	glLineWidth(10);
	glColor3f(0, 1, 1);
	glBegin(GL_LINES);
	for (int j = 0; j < c.ring.size(); ++j) {
		Point3D x = c.ring[(j%c.ring.size())];
		Point3D y = c.ring[((j + 1) % c.ring.size())];
		glVertex3f(x.x, x.y, x.z);
		glVertex3f(y.x, y.y, y.z);
	}
	glEnd();

	if (c.fbase_opposite) {
		glColor3f(1, 1, 0);
		glBegin(GL_LINES);
		for (int j = 0; j < c.ring.size(); ++j) {
			Point3D x = c.ring[(j%c.ring.size())];
			Point3D y = c.ring[((j + 1) % c.ring.size())];
			x = c.fbase_opposite->get_plane().project(x);
			glVertex3f(x.x, x.y, x.z);
			y = c.fbase_opposite->get_plane().project(y);
			glVertex3f(y.x, y.y, y.z);
		}
	}
	glEnd();
}

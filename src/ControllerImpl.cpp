#include "ControllerImpl.h"

using namespace std;

Point3D cpos(0,0,0);

#include <string>
map<Facet*, string> __temp_facet_tag;
char __temp_current_tag[1024];

static float vertex_size = 0.1;
static int edge_width = 5;

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
bool Highlighter::begin_facet(int i, Facet*) {
	if (++index == target) {
		glColor3f(0.3, 0, 0);
	}
	else {
		glColor3f(0.5, 0.5, 0.5);
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
	:BasicViewSelector(ctrl), next(0) {
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
			glColor3f(0.5, 0.5, 0.5);
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

void MainViewer::make_ui(void) {
	ImGui::Begin("Edit");
	ImGui::Text("Selected Facets : %d", selected_facet.size());
	ImGui::Text("Selected Edges : %d", selected_edge.size());
	ImGui::Text("Selected Vertices : %d", selected_vertex.size());

	if( selected_facet.size() == 1){
		Facet*f = (*selected_facet.begin());
		Plane p = f->get_plane();
		ImGui::Text("Clicked Pos : %f %f %f", cpos.x, cpos.y, cpos.z);
		ImGui::Text("Selected Facet Normal : %f %f %f", p.h.x, p.h.y, p.h.z );
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
	}
	
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

	ImGui::Text("==== Available Action ====");

	if (selected_facet.size() == 1 && selected_edge.size() == 0 && selected_vertex.size() == 0) {
		{
			Facet *f = *selected_facet.begin();			
			/*
			if (ImGui::Button("LOAD")) {
				if (__temp_facet_tag.count(f)) {
					strcpy(__temp_current_tag, __temp_facet_tag.at(f).c_str());
				}
				else {
					__temp_current_tag[0] = 0;
				}
			}
			ImGui::SameLine();
			if (ImGui::InputText("", __temp_current_tag, 1024)) {

			}
			ImGui::SameLine();
			if (ImGui::Button("SAVE")) {
				__temp_facet_tag.insert(make_pair(f, string(__temp_current_tag)));
			}
			ImGui::SameLine();
			if (ImGui::Button("Del")) {
				__temp_facet_tag.erase(f);
			}
			*/
		}
		{
			Facet *f = *selected_facet.begin();
			if (f->has_no_adjacent_polygons()) {
				if (ImGui::Button("Make 2.5D Solid")) {
					if (!next) {
						next = new SolidMaker(this, f);
						clear_selection();
					}
				}
			}
		}
		if (ImGui::Button("Reverse Direction")) {
			Facet *f = *selected_facet.begin();
			f->reverse();
		}
		if (ImGui::Button("Planarize")) {
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
		if (ImGui::Button("Delete")) {
			Facet *f = *selected_facet.begin();
			for (int i = 0; i < world->facets.size(); ++i) {
				if (world->facets[i] == f) {
					f->release(&world->vertices);
					world->facets.erase(world->facets.begin() + i);
					break;
				}
			}
			clear_selection();
		}
	}

	if (selected_facet.size() == 2 && selected_edge.size() == 0 && selected_vertex.size() == 0) {
		{
			Facet *fa = *selected_facet.begin();
			Facet *fb = *selected_facet.rbegin();
			if (fa->num_shared_edge(fb) >= 1) {
				if (ImGui::Button("Merge")) {
					Facet::merge(fa, fb, &pts, &facets);
					clear_selection();
				}
			}

			if (fa->get_plane().h.dot_product(fb->get_plane().h) < 0) {
				if (ImGui::Button("Make Connection")) {
					if (!next) {
						next = new ConnectionEditor(this, fa, fb);
						clear_selection();
					}
				}
			}
		}
		
	}

	if (selected_facet.size() == 0 && selected_edge.size() == 1 && selected_vertex.size() == 0) {
		FacetEdge *e = *selected_edge.begin();
		if (e->get_u()->get_vertex()->num_shared_facets(e->get_v()->get_vertex()) == 1) {
			if (ImGui::Button("Make Wall")) {
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
				if (ImGui::Button("Remove Hole")) {
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

				if (ImGui::Button("Make Perpendicular")) {
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
			else if(e->get_u()->get_vertex()->num_shared_facets(u) == 0) {
				if (ImGui::Button("Align Point to Line")) {
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

		if (u->num_shared_facets() == 1 && v->num_shared_facets() == 1 && u->get_facet(0) == v->get_facet(0) && ( u->form_an_edge(v) || v->form_an_edge(u) )) {
			
			if (ImGui::Button("Make Parallel")) {
				Point3D p = u->to_point();
				Point3D q = v->to_point();

				Point3D c = ((u->to_vector() + v->to_vector()) / 2).to_point();
				length_t d = (q-p).length();
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
			if (ImGui::Button("Split")) {
				Facet *f = vs[0]->get_a_shared_facet(vs[1]);
				f->split_by_edge(f->get_vertex(vs[0]), f->get_vertex(vs[1]), &facets);
				clear_selection();
			}
		}
	}

	if (selected_facet.size() == 0 && selected_edge.size() == 0 && selected_vertex.size() == 3) {
		vector<Vertex*> vs(selected_vertex.begin(), selected_vertex.end());
		
		if (ImGui::Button("Make Triangle")) {
			Facet *f = Facet::create_facet(vs[0], vs[1], vs[2]);
			f->triangulate();
			facets.push_back(f);
			clear_selection();
			selected_facet.insert(f);
		}
	}

	ImGui::Text("");
	ImGui::Text("==========================");
	ImGui::SliderFloat("Vertex Size", &vertex_size, 0.01, 0.2);
	ImGui::SliderInt("Edge Width", &edge_width, 1, 5);
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
		/*
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xFF00);
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
		glPopAttrib();*/
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
			
			scalar_t alpha;
			interp(ray, facets[sel.selected_g]->get_plane(), &alpha);
			cpos = ray.p + alpha * ray.v;
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





WallMaker::WallMaker(Controller *ctrl, FacetEdge *e)
	:BasicViewSelector(ctrl), base_edge(e), finalized(false), canceled(false), height(0.1) {
	;
}
void WallMaker::make_ui(void) {
	ImGui::Begin("Making Wall");
	if( ImGui::Button("OK") ) finalized = true;
	if( ImGui::Button("Cancel") ) canceled = true;
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

SolidMaker::SolidMaker(Controller *ctrl, Facet *f)
	:BasicViewSelector(ctrl), base_facet(f), finalized(false), canceled(false), height(3.0) {
	;
}
void SolidMaker::make_ui(void) {
	ImGui::Begin("Making 2.5D Solid");
	if (ImGui::Button("OK")) finalized = true;
	if (ImGui::Button("Cancel")) canceled = true;
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
	
	
	if (ImGui::Button("OK")) {
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
			glLineStipple(1, 0xFF00);
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
		glLineStipple(1, 0xFF00);
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

	if (ImGui::Button("OK")) finalized = true;
	if (ImGui::Button("Cancel")) canceled = true;
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

#include "ControllerImpl_StateEditor.h"
#include "Util.h"
#include <vector>
#include <set>
#include <imgui.h>
using namespace std;

struct Color {
	Color(float _r = 0, float _g = 0, float _b = 0, float _a = 1.0) 
		:r(_r), g(_g), b(_b), a(_a) {
		;
	}

	float r;
	float g;
	float b;
	float a;
};

void StateEditorDrawSetting::begin_facets(void) { return; }
void StateEditorDrawSetting::begin_vertices(void) { return; }
void StateEditorDrawSetting::begin_edges(void) { return; }

bool StateEditorDrawSetting::begin_vertex(Vertex*) { return true; }
bool StateEditorDrawSetting::begin_facet(int, Facet*) { return true; }
bool StateEditorDrawSetting::begin_facetvertex(int, FacetVertex*) { return true; }
bool StateEditorDrawSetting::begin_facetedge(int, FacetEdge*) { return true; }

void StateEditorDrawSetting::end_cellspace(int) { ; }
void StateEditorDrawSetting::end_layer(int) { ; }
void StateEditorDrawSetting::end_state(int, State*) { ; }
void StateEditorDrawSetting::end_transition(int, Transition*) { ; }

bool StateEditorDrawSetting::begin_interlayerconnection(int, InterlayerConnection*) { return false; }
void StateEditorDrawSetting::end_interlayerconnection(int, InterlayerConnection*) { ; }

struct StateEditorIndexer : public StateEditorDrawSetting {
	StateEditorIndexer(void)
		:idx(0) {
		;
	}
	bool begin_facet(int, Facet*) { 
		setColori(++idx);
		return true; 
	}
	bool begin_facetedge(int, FacetEdge*) { 
		return false; 
	}
	virtual void begin_cellspace(int) {

	}
	virtual void begin_layer(int) {

	}
	virtual bool begin_state(int, State*) {
		setColori(++idx);
		return true;
	}
	virtual bool begin_transition(int, Transition*) {
		setColori(++idx);
		return true;
	}

protected:
	int idx;
};
struct StateEditorSelector : public StateEditorDrawSetting {
	StateEditorSelector(int _i)
		:idx(0) , target_idx(_i){
		;
	}
	bool begin_vertex(Vertex*) { return false; }
	bool begin_facetvertex(int, FacetVertex*) { return false; }
	bool begin_facetedge(int, FacetEdge*) { return false; }


	bool begin_facet(int i, Facet*) { 
		if (++idx == target_idx) {
			selection_type = STATEEDITORSELECTION_TYPE_FACET;
			selected_g = current_g;
			selected_i = i;
		}
		return false; 
	}

	virtual void begin_cellspace(int i) {
		current_g = i;
	}
	virtual void begin_layer(int i) {
		current_g = i;
	}
	virtual bool begin_state(int i, State*) {
		if (++idx == target_idx) {
			selection_type = STATEEDITORSELECTION_TYPE_STATE;
			selected_g = current_g;
			selected_i = i;
		}
		return false;
	}
	virtual bool begin_transition(int i, Transition*) {
		if (++idx == target_idx) {
			selection_type = STATEEDITORSELECTION_TYPE_TRANSITION;
			selected_g = current_g;
			selected_i = i;
		}
		return false;
	}

	enum STATEEDITORSELECTION_TYPE {
		STATEEDITORSELECTION_TYPE_NONE,
		STATEEDITORSELECTION_TYPE_FACET,
		STATEEDITORSELECTION_TYPE_STATE,
		STATEEDITORSELECTION_TYPE_TRANSITION,
	} selection_type;

	int current_g;
	int selected_g;
	int selected_i;

protected:
	int idx;
	int target_idx;
};

struct StateEditorHighlighter : public StateEditorDrawSetting {
	StateEditorHighlighter(void) {
		facet_color = Color(0.5, 0.5, 0.5);
		state_color = Color(1, 0, 0);
		transition_color = Color(1, 1, 1);
		selected_facet_color = Color(1, 0, 0);
		selected_state_color = Color(1, 1, 1);
		selected_transition_color = Color(1, 0, 0);
	}

	bool begin_facet(int i, Facet* f) {
		if (selected_facet.count(f) == 1) {
			setColorf(selected_facet_color.r, selected_facet_color.g, selected_facet_color.b, selected_facet_color.a);
		}
		else {
			setColorf(facet_color.r, facet_color.g, facet_color.b, facet_color.a);
		}
		return true;
	}

	virtual void begin_cellspace(int i) {}
	virtual void begin_layer(int i) {}
	virtual bool begin_state(int i, State * s) {
		if (s->is_public_safty_feature()) return false;
		if (selected_state.count(s) == 1) {
			setColorf(selected_state_color.r, selected_state_color.g, selected_state_color.b, selected_state_color.a);
		}
		else {
			setColorf(state_color.r, state_color.g, state_color.b, state_color.a);
		}
		return true;
	}
	virtual bool begin_transition(int i, Transition *t) {
		if (selected_transition.count(t) == 1) {
			setColorf(selected_transition_color.r, selected_transition_color.g, selected_transition_color.b, selected_transition_color.a);
		}
		else {
			setColorf(transition_color.r, transition_color.g, transition_color.b, transition_color.a);
		}
		return true;
	}

	virtual bool begin_interlayerconnection(int, InterlayerConnection*) { glColor3f(1, 1, 0);  return true; }
	virtual void end_interlayerconnection(int, InterlayerConnection*) { ; }

	set<Facet*> selected_facet;
	set<State*> selected_state;
	set<Transition*> selected_transition;

	Color facet_color;
	Color state_color;
	Color transition_color;
	Color selected_facet_color;
	Color selected_state_color;
	Color selected_transition_color;
};

StateBasicViewer::StateBasicViewer(Controller * ctrl)
	: Controller(ctrl) {
	;
}

int StateBasicViewer::get_current_stage(void) const {
	return 2;
}

void StateBasicViewer::make_ui(void) {

}

void StateBasicViewer::draw(StateEditorDrawSetting* d, double state_size, int icon_size, int transition_linewidth) {
	std::vector<CellSpace*> &cellspaces = Controller::world->cellspaces;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glColor3f(1, 1, 1);

	for (int i = 0; i < cellspaces.size(); ++i) {
		if (d) d->begin_cellspace(i);

		std::vector<Facet*> &facets = cellspaces[i]->facets;
		for (int j = 0; j < facets.size(); ++j) {
			facets[j]->draw(d, j);
		}

		if (d) d->end_cellspace(i);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDepthMask(GL_FALSE);
	glLineWidth(1);
	glColor3f(0, 0, 0);

	for (int i = 0; i < cellspaces.size(); ++i) {
		if (d) d->begin_cellspace(i);

		std::vector<Facet*> &facets = cellspaces[i]->facets;
		for (int j = 0; j < facets.size(); ++j) {
			facets[j]->draw_edge(d);
		}
		
		if (d) d->end_cellspace(i);
	}
	glDepthMask(GL_TRUE);

	std::vector<Layer*> &layers = Controller::world->layers;
	for (int i = 0; i < layers.size(); ++i) {
		if (d) d->begin_layer(i);
		glLineWidth(transition_linewidth);
		std::vector<Transition*> &transitions = layers[i]->transitions;
		glColor3f(1, 1, 1);
		for (int j = 0; j < transitions.size(); ++j) {
			if (d && !d->begin_transition(j,transitions[j])) continue;
			Point3D p = transitions[j]->u->p;
			glBegin(GL_LINES);
			for (int k = 0; k < transitions[j]->midpoints.size(); ++k) {
				glVertex3f(p.x, p.y, p.z);
				p = transitions[j]->midpoints[k];
				glVertex3f(p.x, p.y, p.z);
			}
			Point3D q = transitions[j]->v->p;
			glVertex3f(p.x, p.y, p.z);
			glVertex3f(q.x, q.y, q.z);
			glEnd();
			if (d) d->end_transition(j, transitions[j]);
		}

		glPointSize(icon_size);
		std::vector<State*> &states = layers[i]->states;
		for (int j = 0; j < states.size(); ++j) {
			if (d && !d->begin_state(j, states[j])) continue;
			Point3D p = states[j]->p;
			if (states[j]->is_public_safty_feature()) {
				glBegin(GL_POINTS);
				glVertex3f(p.x, p.y, p.z);
				glEnd();
			}
			else {
				draw_sphere(p.x, p.y, p.z, state_size);
			}
			if (d) d->end_state(j, states[j]);
		}
		if (d) d->end_layer(i);
	}

	for (int i = 0; i < world->interlayer_connections.size(); ++i) {
		if (d && !d->begin_interlayerconnection(i, world->interlayer_connections[i])) continue;
		glLineWidth(1);
		State *s = world->interlayer_connections[i]->container;
		State *t = world->interlayer_connections[i]->containee;
		glBegin(GL_LINES);
		glVertex3f(s->p.x, s->p.y, s->p.z);
		glVertex3f(t->p.x, t->p.y, t->p.z);
		glEnd();
		if (d) d->end_interlayerconnection(i, world->interlayer_connections[i]);
	}
}
void StateBasicViewer::draw_select_scene(void) {
	//StateEditorIndexer d;
	//draw(&d, 0.5);
}
void StateBasicViewer::draw_scene(void) {
	StateEditorHighlighter d;
	draw(&d);
}
void StateBasicViewer::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	/*StateEditorSelector d(current_obj);
	draw(&d, 0.5);
	if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_FACET) {

	}
	else if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_STATE) {

	}
	else if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_TRANSITION) {

	}*/
}
void StateBasicViewer::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {

}
void StateBasicViewer::on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) {

}
void StateBasicViewer::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {

}

//////////////////////////////////////////////////////////////////////////////////////////////


static vector<Texture2D> signs;
static vector<State::TYPE_STATE> sign_type;
static map<State::TYPE_STATE, int> sign_icon_idx_map;

void  __temp_load_signs(void) {
	{
		Texture2D texture;
		texture.load_jpg("res/icons/fireext.jpg");
		signs.push_back(texture);
		sign_type.push_back(State::TYPE_STATE::STATE_FIREEXTINGUISHER);
		sign_icon_idx_map[State::TYPE_STATE::STATE_FIREEXTINGUISHER] = 0;
	}
	{
		Texture2D texture;
		texture.load_jpg("res/icons/indoorhyd.jpg");
		signs.push_back(texture);
		sign_type.push_back(State::TYPE_STATE::STATE_INDOORHYDRANT);
		sign_icon_idx_map[State::TYPE_STATE::STATE_INDOORHYDRANT] = 1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
StateMainController::StateMainController(Controller * ctrl)
	: StateBasicViewer(ctrl), next(0) {
	clear_selection();
	if (signs.empty()) __temp_load_signs();
}

void StateMainController::draw_select_scene(void) {
	StateEditorIndexer d;
	draw(&d);
}
struct StateEditorMainDrawSetting : public StateEditorHighlighter {
	virtual bool begin_state(int i, State * s) {
		if (s->is_public_safty_feature()) {
			setColorf(0, 0, 1);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_POINT_SPRITE);
			signs[sign_icon_idx_map.at(s->state_type)].bind();
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		} else if( s->is_nonnavigable()) {
			setColorf(0.7, 0.7, 0.7);
		}
		else {
			setColorf(1, 0, 0);
		}
		if (selected_state.count(s)) setColorf(0, 1, 0);
		return true;
	}

	virtual void end_state(int i, State* s) {

		if (s->is_public_safty_feature()) {
			
			signs[sign_icon_idx_map.at(s->state_type)].unbind();
			glDisable(GL_POINT_SPRITE);
			glEnable(GL_DEPTH_TEST);
		}
		else if (s->is_nonnavigable()) {

		}
		else {

		}
	}
};
void StateMainController::draw_scene(void) {
	//StateEditorHighlighter d;
	StateEditorMainDrawSetting d;
	if (selected_facet) {
		d.selected_facet.insert(selected_facet);
		d.selected_facet_color = Color(0.5, 0, 0);
	}
	if (selected_state) {
		if (selected_state->duality) {
			for (int i = 0; i < selected_state->duality->facets.size(); ++i) {
				d.selected_facet.insert(selected_state->duality->facets[i]);
			}
		}
		d.selected_state.insert(selected_state);
		d.selected_facet_color = Color(0.0, 0.5, 0);
	}
	if (selected_transition) {
		for (int i = 0; i < selected_transition->u->duality->facets.size(); ++i) {
			d.selected_facet.insert(selected_transition->u->duality->facets[i]);
		}
		for (int i = 0; i < selected_transition->v->duality->facets.size(); ++i) {
			d.selected_facet.insert(selected_transition->v->duality->facets[i]);
		}
		d.selected_state.insert(selected_transition->u);
		d.selected_state.insert(selected_transition->v);
		d.selected_transition.insert(selected_transition);
		d.selected_facet_color = Color(0.0, 0.5, 0);
	}
	draw(&d);
}
void StateMainController::clear_selection(void) {
	selected_facet = 0;
	selected_state = 0;
	selected_transition = 0;
}
void StateMainController::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	StateEditorSelector d(current_obj);
	draw(&d);
	clear_selection();
	if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_FACET) {
		clear_selection();
		selected_facet = world->cellspaces[d.selected_g]->facets[d.selected_i];
	}
	else if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_STATE) {
		clear_selection();
		selected_state = world->layers[d.selected_g]->states[d.selected_i];
	}
	else if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE::STATEEDITORSELECTION_TYPE_TRANSITION) {
		clear_selection();
		selected_transition = world->layers[d.selected_g]->transitions[d.selected_i];
	}
}
void StateMainController::make_ui(void) {
	ImGui::Begin("State Editor Menu");
	if (ImGui::Button("Edit State/Transition Geometry")) {
		if (!next) {
			next = new StateLocationEditor(this);
		}
	}
	if (ImGui::Button("Place a Public Safety Feature")) {
		if (!next) {
			next = new StatePSFeaturePlacer(this);
		}
	}

	if (selected_state) {
		ImGui::Text("======================");
		if (!selected_state->is_nonnavigable() && !selected_state->is_public_safty_feature()) {
			if (ImGui::Button("Interlayer Connection Editor")) {
				if (!next) {
					next = new StateInterlayerConnectionEstablisher(this, selected_state);
				}
			}

			bool room = (selected_state->duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_ROOM);
			bool corridor = (selected_state->duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_CORRIDOR);
			bool door = (selected_state->duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_DOOR);
			bool extdoor = (selected_state->duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_EXTERIORDOOR);
			int room_type = 0;
			if (room) { room_type = 1; }
			if (corridor) { room_type = 2; }
			if (door) { room_type = 3; }
			if (extdoor) { room_type = 4; }
			if (ImGui::RadioButton("Room", &room_type, 1)) {
				selected_state->duality->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_ROOM;
			}
			if (ImGui::RadioButton("Corridor", &room_type, 2)) {
				selected_state->duality->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_CORRIDOR;
			}
			if (ImGui::RadioButton("Door", &room_type, 3)) {
				selected_state->duality->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_DOOR;
			}
			if (ImGui::RadioButton("Exterior Door", &room_type, 4)) {
				selected_state->duality->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_EXTERIORDOOR;
			}
		}
		if (ImGui::Button("Set to Default Location")) {
			selected_state->p = selected_state->duality->get_centroid();
		}
	}
	if (selected_transition) {
		ImGui::Text("======================");
		if (ImGui::Button("Remove all mid points")) {
			selected_transition->midpoints.clear();
		}
	}

	ImGui::End();
}

void StateMainController::post_draw(void) {
	if (next) {
		Controller::current_controller = next;
		next = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

StatePSFeaturePlacer::StatePSFeaturePlacer(Controller *ctrl)
	:StateBasicViewer(ctrl),
on_facet (0),
base_pt (0,0,0),
height (0.0),
ps_type(0),
finalized(false),
canceled(false)
{
	;
}

struct StateEditorPSFeatureDrawSetting : public StateEditorDrawSetting {
	virtual void begin_facets(void) { return; }
	virtual void begin_vertices(void) { return; }
	virtual void begin_edges(void) { return; }

	virtual bool begin_vertex(Vertex*) { return false; }
	virtual bool begin_facet(int, Facet*) { return true; }
	virtual bool begin_facetvertex(int, FacetVertex*) { return false; }
	virtual bool begin_facetedge(int, FacetEdge*) { return false; }

	virtual void begin_cellspace(int) { return; }
	virtual void end_cellspace(int) { return; }
	virtual void begin_layer(int) { return; }
	virtual void end_layer(int) { return; }
	virtual bool begin_state(int, State*) { return false; }
	virtual void end_state(int, State*) { return; }
	virtual bool begin_transition(int, Transition*) { return false; }
	virtual void end_transition(int, Transition*) { return; }
};

struct StateEditorPSFeatureIndexer : public StateEditorPSFeatureDrawSetting {
	StateEditorPSFeatureIndexer() :idx(0) {

	}
	virtual bool begin_facet(int i, Facet* f) { 
		if (StateEditorPSFeatureDrawSetting::begin_facet(i, f)) {
			setColori(++idx);
			return true;
		}
		return false; 
	}

	int idx;
};
struct StateEditorPSFeatureSelector : public StateEditorPSFeatureDrawSetting {
	StateEditorPSFeatureSelector(int _target) :idx(0), target_idx(_target), selected_g(-1), selected_i(-1) {

	}
	virtual bool begin_facet(int i, Facet* f) {
		if (StateEditorPSFeatureDrawSetting::begin_facet(i, f)) {
			if (++idx == target_idx) {
				selected_g = current_g;
				selected_i = i;
			}
		}
		return true;
	}

	virtual void begin_cellspace(int i) { current_g = i; return; }

	int idx;
	int target_idx;
	int current_g;
	int selected_g;
	int selected_i;
};

void StatePSFeaturePlacer::draw_scene(void) {
	StateEditorHighlighter d;
	if (on_facet) {
		d.selected_facet.insert(on_facet);
		d.selected_facet_color = Color(0, 0.5, 0, 1);
	}
	draw(&d);
	if (on_facet) {
		Point3D p = base_pt + on_facet->get_plane().h * height;
		
		glDisable(GL_DEPTH_TEST);
		glPointSize(5);
		glColor3f(1, 0, 0);
		glBegin(GL_POINTS);
		glVertex3f(base_pt.x, base_pt.y, base_pt.z);
		glEnd();

		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex3f(base_pt.x, base_pt.y, base_pt.z);
		glVertex3f(p.x, p.y, p.z);
		glEnd();

		glPointSize(40);
		glColor3f(1, 1, 0);
		glBegin(GL_POINTS);
		glVertex3f(p.x, p.y, p.z);
		glEnd();

		glPointSize(32);
		signs[ps_type].bind();
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		

		glBegin(GL_POINTS);
		glVertex3f(p.x, p.y, p.z);
		glEnd();
		
		signs[ps_type].unbind();
		glDisable(GL_POINT_SPRITE);

		glEnable(GL_DEPTH_TEST);
	}
}

void StatePSFeaturePlacer::draw_select_scene(void) {
	StateEditorPSFeatureIndexer d;
	draw(&d);
}

void StatePSFeaturePlacer::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	StateEditorPSFeatureSelector d(current_obj);
	draw(&d);
	if (d.selected_i != -1) {
		Facet *f = world->cellspaces[d.selected_g]->facets[d.selected_i];
		Plane plane = f->get_plane();
		scalar_t alpha;
		if (!interp(ray, plane, &alpha)) return;

		on_facet = f;
		base_pt = ray.p + alpha * ray.v;
	}
}

void StatePSFeaturePlacer::make_ui(void) {
	ImGui::Begin("Public Safety Feature");
	ImGui::Text("Type");
	for (int i = 0; i < signs.size(); ++i) {
		if (ImGui::ImageButton((ImTextureID)signs[i].texture_id, ImVec2(32, 32))) {
			ps_type = i;
		}
	}
	ImGui::SliderFloat("Height", &this->height, 0, 1);

	ImGui::Text("");
	if (ImGui::Button("OK", ImVec2(250, 0))) {
		finalized = true;
	}
	if (ImGui::Button("Cancel", ImVec2(250, 0))) {
		canceled = true;
	}
	ImGui::End();
}

void StatePSFeaturePlacer::post_draw(void) {
	if (finalized) {
		State *s = new State();
		
		s->p = base_pt + on_facet->get_plane().h * height;
		s->state_type = sign_type[ps_type];
		s->duality = 0;

		world->layers[0]->states.push_back(s);
		
		InterlayerConnection *e = new InterlayerConnection();
		e->container = world->belongs_to.at(on_facet)->duality;
		e->containee = s;

		world->interlayer_connections.push_back(e);
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

StateLocationEditor::StateLocationEditor(Controller * ctrl)
	:StateBasicViewer(ctrl), finalized(false), ptr(0) {

}

void StateLocationEditor::draw_select_scene(void) {
	StateEditorIndexer d;
	draw(&d);
}

void StateLocationEditor::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	StateEditorSelector d(current_obj);
	draw(&d);
	if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE_STATE) {
		State* s = world->layers[d.selected_g]->states[d.selected_i];
		if (!ptr) {
			ptr = &s->p;
			return;
		}
	}
	else if (d.selection_type == StateEditorSelector::STATEEDITORSELECTION_TYPE_TRANSITION) {
		if (ptr) return;
		Transition* t = world->layers[d.selected_g]->transitions[d.selected_i];
		{ // POINT
			length_t min_dist = FLT_MAX;
			int min_i = -1;
			for (int i = 0; i < t->midpoints.size(); ++i) {
				Point3D p = t->midpoints[i];
				scalar_t alpha;
				if (!interp(p, ray, &alpha)) continue;
				Point3D p_nearest = ray.p + alpha * ray.v;
				length_t dist = (p - p_nearest).length();
				if (dist < min_dist) {
					min_dist = dist;
					min_i = i;
				}
			}

			if (min_dist < 0.1) {
				ptr = &t->midpoints[min_i];
				return;
			}
		}


		{ // EDGE
			length_t min_dist = FLT_MAX;
			int min_i = -1;
			Point3D min_v;
			Point3D p = t->u->p;
			for (int i = 0; i < t->midpoints.size(); ++i) {
				Point3D q = t->midpoints[i];
				Line3D l(p, q - p);
				scalar_t alpha, beta;
				if (!interp(l, ray, &alpha, &beta)) continue;

				if (alpha < 0) alpha = 0;
				if (alpha > 1) alpha = 1;
				if (beta < 0) beta = 0;

				Point3D l_v = l.p + alpha * l.v;
				Point3D ray_v = ray.p + beta * ray.v;

				length_t dist = (l_v - ray_v).length();
				if (dist < min_dist) {
					min_dist = dist;
					min_i = i;
					min_v = ray_v;
				}
				p = q;
			}
			do {
				Point3D q = t->v->p;
				Line3D l(p, q - p);
				scalar_t alpha, beta;
				if (!interp(l, ray, &alpha, &beta)) continue;

				if (alpha < 0) alpha = 0;
				if (alpha > 1) alpha = 1;
				if (beta < 0) beta = 0;

				Point3D l_v = l.p + alpha * l.v;
				Point3D ray_v = ray.p + beta * ray.v;

				length_t dist = (l_v - ray_v).length();
				if (dist < min_dist) {
					min_dist = dist;
					min_i = t->midpoints.size();
					min_v = ray_v;
				}
			} while (0);

			if (min_dist < 0.1) {
				t->midpoints.insert(t->midpoints.begin() + min_i, min_v);
				ptr = &t->midpoints[min_i];
			}
		}
	}
}

void StateLocationEditor::make_ui(void) {
	ImGui::Begin("State/Transition Geometry Editor");
	if (ImGui::Button("OK", ImVec2(250, 0))) {
		finalized = true;
	}
	ImGui::End();
}
void StateLocationEditor::post_draw(void) {
	if (finalized) {
		end();
		return;
	}
	if (ptr) {
		Controller::current_controller = new StatePointPlacer(this, *ptr);
		ptr = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

StatePointPlacer::StatePointPlacer(Controller * ctrl, Point3D& pt)
:StateBasicViewer(ctrl), base_pt(pt), init_pt(pt), finalized(false), canceled(false){

}

void StatePointPlacer::on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) {
	Plane plane(init_pt, Vector3D(0, 0, 1));
	scalar_t alpha;
	if (!interp(ray, plane, &alpha)) return;
	base_pt = ray.p + alpha * ray.v;
}
void StatePointPlacer::on_mouse_up(int x, int y, const Line3D& ray, int current_obj) {
	finalized = true;
}
void StatePointPlacer::post_draw(void) {
	if (finalized) {
		end();
		return;
	}
	if (canceled) {
		base_pt = init_pt;
		end();
		return;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////

StateInterlayerConnectionEstablisher::StateInterlayerConnectionEstablisher(Controller * ctrl, State* s)
	:StateBasicViewer(ctrl), container(s), finalized(false), canceled(false) {
	for (int i = 0; i < world->interlayer_connections.size(); ++i) {
		if (world->interlayer_connections[i]->container == s) {
			selected.insert(world->interlayer_connections[i]->containee);
		}
	}
}

struct StateInterlayerConnectionEstablisherIndexer : public StateEditorDrawSetting {
	StateInterlayerConnectionEstablisherIndexer(void)
		:idx(0) {
	}
	bool begin_facet(int, Facet*) { setColori(0); return true;}
	bool begin_facetedge(int, FacetEdge*) { return false; }
	virtual void begin_cellspace(int) { ;  }
	virtual void begin_layer(int) { ; }
	virtual bool begin_state(int, State* s) {
		if (s->is_nonnavigable()) { setColori(++idx); }
		return true;
	}
	virtual bool begin_transition(int, Transition*) { setColori(0); return true; }
protected:
	int idx;
};
struct StateInterlayerConnectionEstablisherSelector : public StateEditorDrawSetting {
	StateInterlayerConnectionEstablisherSelector(int t)
		:idx(0), target_idx(t), selected_s(0) {
	}
	bool begin_facet(int, Facet*) { return true; }
	bool begin_facetedge(int, FacetEdge*) { return false; }
	virtual void begin_cellspace(int) { ; }
	virtual void begin_layer(int) { ; }
	virtual bool begin_state(int, State* s) {
		if (s->is_nonnavigable()) {
			if (++idx == target_idx) {
				selected_s = s;
			}
		}
		return true;
	}
	virtual bool begin_transition(int, Transition*) { return true; }
	State* selected_s;
protected:
	int idx;
	int target_idx;
};
struct StateInterlayerConnectionEstablisherHighlighter : public StateEditorHighlighter {
	StateInterlayerConnectionEstablisherHighlighter(State*s)
		:container(s) {
		for (int i = 0; i < s->duality->facets.size(); ++i) {
			selected_facet.insert(s->duality->facets[i]);
		}
	}
	virtual bool begin_state(int i, State* s) {
		if (s == container) {
			setColorf(1, 0, 0);
			return true;
		}
		if (!s->is_nonnavigable()) return false;
		return StateEditorHighlighter::begin_state(i, s);
	}
	virtual bool begin_transition(int, Transition*) { return false; }
	virtual bool begin_interlayerconnection(int, InterlayerConnection*) { return false; }
protected:
	State* container;
};

void StateInterlayerConnectionEstablisher::draw_select_scene(void) {
	StateInterlayerConnectionEstablisherIndexer d;
	draw(&d);
}
void StateInterlayerConnectionEstablisher::draw_scene(void) {
	StateInterlayerConnectionEstablisherHighlighter d(container);
	d.selected_state = selected;
	d.selected_facet_color = Color(0, 0.5, 0);
	d.state_color = Color(0.5, 0.5, 0.5);
	d.selected_state_color = Color(1, 1, 0);
	draw(&d);
}
void StateInterlayerConnectionEstablisher::on_mouse_down(int x, int y, const Line3D& ray, int current_obj) {
	StateInterlayerConnectionEstablisherSelector d(current_obj);
	draw(&d);
	if (d.selected_s) {
		State *s = d.selected_s;
		if (selected.count(s)) selected.erase(s);
		else selected.insert(s);
	}
}
void StateInterlayerConnectionEstablisher::make_ui(void) {
	ImGui::Begin("Interlayer Connection Establisher");
	if (ImGui::Button("OK", ImVec2(250, 0))) { finalized = true; }
	if (ImGui::Button("Cancel", ImVec2(250, 0))) { canceled = true; }
	ImGui::End();
}

void StateInterlayerConnectionEstablisher::post_draw(void) {
	if (finalized) {
		vector<InterlayerConnection*> new_iledges;
		for (int i = 0; i < world->interlayer_connections.size(); ++i) {
			if (world->interlayer_connections[i]->container != container) {
				new_iledges.push_back(world->interlayer_connections[i]);
			}
			else {
				delete world->interlayer_connections[i];
			}
		}
		for (auto i = selected.begin(); i != selected.end(); ++i) {
			InterlayerConnection *e = new InterlayerConnection();
			e->container = container;
			e->containee = *i;

			new_iledges.push_back(e);
		}
		world->interlayer_connections = new_iledges;
		end();
		return;
	}
	if (canceled) {
		end();
		return;
	}
}
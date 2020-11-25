#include "World.h"
#include <vector>
#include <set>
#include <map>
using namespace std;

Vector3D Connection::get_normal_of_ring(void) const {
	Vector3D ret(0, 0, 0);
	for (int i = 0; i < ring.size(); ++i) {
		ret = ret + (ring[i] - ring[0]).cross_product(ring[(i + 1) % ring.size()] - ring[0]);
	}
	return ret;
}

void Connection::reverse(void) {
	ring = vector<Point3D>(ring.rbegin(), ring.rend());
}

bool State::is_nonnavigable(void) const {
	return duality && duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_NONNAVIGABLE;
}
bool State::is_public_safty_feature(void) const {
	switch (state_type) {
	case State::TYPE_STATE::STATE_NORMALSTATE:
		return false;
	case State::TYPE_STATE::STATE_ALARM:
	case State::TYPE_STATE::STATE_DETECTOR:
	case State::TYPE_STATE::STATE_FIREEXTINGUISHER:
	case State::TYPE_STATE::STATE_INDOORHYDRANT:
	case State::TYPE_STATE::STATE_SPRINKLER:
		return true;
	default: throw("Undefined State type)");
	}
	return false;
}

Point3D CellSpace::get_centroid(void) const {
	set<Vertex*> vs;
	for (int i = 0; i < facets.size(); ++i) {
		for (int j = 0; j < facets[i]->num_vertices(); ++j) {
			vs.insert(facets[i]->get_vertex(j)->get_vertex());
		}
	}

	Vector3D v_centroid(0, 0, 0);
	for (auto i = vs.begin(); i != vs.end(); ++i) {
		v_centroid = v_centroid + (*i)->to_vector();
	}
	return (v_centroid / vs.size()).to_point();
}

bool CellSpace::is_room(void) const { return cellspace_type == TYPE_CELLSPACE::TYPE_ROOM; }
bool CellSpace::is_door(void) const { return cellspace_type == TYPE_CELLSPACE::TYPE_DOOR || cellspace_type == TYPE_CELLSPACE::TYPE_EXTERIORDOOR; }
bool CellSpace::is_corridor(void) const { return cellspace_type == TYPE_CELLSPACE::TYPE_CORRIDOR; }

Point3D CellSpaceBoundary::get_centroid(void) const {
	Vector3D v_centroid(0, 0, 0);
	for (auto i = ring.begin(); i != ring.end(); ++i) {
		v_centroid = v_centroid + (*i)->to_vector();
	}
	return (v_centroid / ring.size()).to_point();
}


static void dfs_get_connected_facets(Facet* f, set<Facet*>& unvisited, vector<Facet*>& fs) {
	fs.push_back(f);
	unvisited.erase(f);
	for (int i = 0; i < f->num_vertices(); ++i) {
		Vertex *v = f->get_vertex(i)->get_vertex();
		for (int j = 0; j < v->num_shared_facets(); ++j) {
			Facet *f_next = v->get_facet(j);
			if (unvisited.count(f_next)==0) continue;
			dfs_get_connected_facets(f_next, unvisited, fs);
		}
	}
}

static vector<Facet*> dfs_get_connected_facets(Facet* f, set<Facet*>& unvisited) {
	vector<Facet*> fs;
	dfs_get_connected_facets(f, unvisited, fs);
	return fs;
}

void World::clear(void) {
	vertices.clear();
	facets.clear();
	objects.clear();
	connections.clear();;

	belongs_to.clear();
	cellspaces.clear();
	layers.clear();
	interlayer_connections.clear();
}

void World::make_cellspaces(void) {

	Layer* base_layer = new Layer();
	base_layer->id = "base";
	layers.push_back(base_layer);

	set<Facet*> unvisited(facets.begin(), facets.end());
	vector< vector<Facet*> > connected_facets;
	while (!unvisited.empty()) {
		Facet* f = *unvisited.begin();
		connected_facets.push_back(dfs_get_connected_facets(f, unvisited));
	}

	// ROOM & CORRIDORS
	for (int i = 0; i < connected_facets.size(); ++i) {
		CellSpace *c = new CellSpace();
		c->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_ROOM;
		for (int j = 0; j < connected_facets[i].size(); ++j) {
			c->facets.push_back(connected_facets[i][j]);
			belongs_to[connected_facets[i][j]] = c;
		}

		State *s = new State();
		s->state_type = State::STATE_NORMALSTATE;
		s->p = c->get_centroid();

		c->duality = s;
		s->duality = c;

		cellspaces.push_back(c);
		base_layer->states.push_back(s);
	}

	// CONNECTIONS
	for (int i = 0; i < connections.size(); ++i) {
		if (connections[i].get_normal_of_ring().dot_product(connections[i].fbase->get_plane().h) < 0) {
			connections[i].reverse();
		}
		vector<Vertex*> boundary_to_base_facet;
		for (int j = 0; j < connections[i].ring.size(); ++j) {
			coord_t x, y, z;
			x = connections[i].ring[j].x;
			y = connections[i].ring[j].y;
			z = connections[i].ring[j].z;
			Vertex* v = new Vertex(x, y, z);
			boundary_to_base_facet.push_back(v);
		}
		reverse(boundary_to_base_facet.begin(), boundary_to_base_facet.end());
		Facet *f_boundary_to_base_facet = Facet::create_facet(boundary_to_base_facet);
		f_boundary_to_base_facet->triangulate();

		vector<Vertex*> boundary_to_opposite_facet;
		if (connections[i].fbase_opposite) {
			for (int j = 0; j < connections[i].ring.size(); ++j) {
				Plane plane = connections[i].fbase_opposite->get_plane();
				Point3D p(connections[i].ring[j].x, connections[i].ring[j].y, connections[i].ring[j].z);
				p = plane.project(p);
				Vertex* v = new Vertex(p.x, p.y, p.z);
				boundary_to_opposite_facet.push_back(v);
			}
		}
		else {
			for (int j = 0; j < connections[i].ring.size(); ++j) {
				//Plane plane = connections[i].fbase_opposite->get_plane();
				Vector3D plane_normal = connections[i].fbase->get_plane().h.normalized();
				Plane plane(connections[i].fbase->get_plane().p - plane_normal * 0.1, -1 * plane_normal);
				Point3D p(connections[i].ring[j].x, connections[i].ring[j].y, connections[i].ring[j].z);
				p = plane.project(p);
				Vertex* v = new Vertex(p.x, p.y, p.z);
				boundary_to_opposite_facet.push_back(v);
			}
		}
		Facet *f_boundary_to_opposite_facet = Facet::create_facet(boundary_to_opposite_facet);
		f_boundary_to_opposite_facet->triangulate();

		reverse(boundary_to_base_facet.begin(), boundary_to_base_facet.end());
		vector<Facet*> side_facets;
		for (int i = 0; i < boundary_to_base_facet.size(); ++i) {
			int i_next = (i + 1) % boundary_to_base_facet.size();
			Facet *f = Facet::create_facet(boundary_to_base_facet[i], boundary_to_base_facet[i_next], boundary_to_opposite_facet[i_next], boundary_to_opposite_facet[i]);
			side_facets.push_back(f);
			f->triangulate();
		}
		reverse(boundary_to_opposite_facet.begin(), boundary_to_opposite_facet.end());

		// 이 시점에서 boundary_to_base_facet와 boundary_to_opposite_facet는 서로 반대편을 바라보고 있다.

		bool cell_space_constructed = true;
		{
			Plane p = f_boundary_to_base_facet->get_plane();
			Point3D v = f_boundary_to_opposite_facet->get_plane().p;
			length_t d;
			p.project(v, &d);
			if (abs(d) < 0.001) {
				cell_space_constructed = false;
			}
		}

		State* s = 0;

		if (cell_space_constructed) {
			CellSpace* c = new CellSpace();
			if (connections[i].fbase_opposite) {
				c->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_DOOR;
			}
			else {
				c->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_EXTERIORDOOR;
			}
			c->facets.push_back(f_boundary_to_base_facet);
			c->facets.push_back(f_boundary_to_opposite_facet);
			for (int i = 0; i < side_facets.size(); ++i) {
				c->facets.push_back(side_facets[i]);
			}

			for (int i = 0; i < boundary_to_base_facet.size(); ++i) {
				vertices.push_back(boundary_to_base_facet[i]);
				vertices.push_back(boundary_to_opposite_facet[i]);
			}

			cellspaces.push_back(c);

			s = new State();
			s->p = c->get_centroid();
			s->duality = c;
			c->duality = s;
			base_layer->states.push_back(s);
		}
		else {
			s = belongs_to.at(connections[i].fbase_opposite)->duality;
		}

		if (1) {
			Transition* t_bm = new Transition();
			t_bm->u = s;
			t_bm->v = belongs_to.at(connections[i].fbase)->duality;
			belongs_to.at(connections[i].fbase)->duality->connects.push_back(t_bm);
			s->connects.push_back(t_bm);
			base_layer->transitions.push_back(t_bm);

			// corresponding CellSpaceBoundary's
			CellSpaceBoundary* boundary_on_base_towards_middle = new CellSpaceBoundary();
			boundary_on_base_towards_middle->cellspace = t_bm->v->duality;
			boundary_on_base_towards_middle->ring = boundary_to_base_facet;
			boundary_on_base_towards_middle->duality = t_bm;
			t_bm->forward_duality = boundary_on_base_towards_middle;
			boundary_on_base_towards_middle->cellspace->boundaries.push_back(boundary_on_base_towards_middle);

			CellSpaceBoundary* boundary_on_middle_towards_base = new CellSpaceBoundary();
			boundary_on_middle_towards_base->cellspace = t_bm->u->duality;
			boundary_on_middle_towards_base->ring = boundary_to_base_facet;
			reverse(boundary_on_middle_towards_base->ring.begin(), boundary_on_middle_towards_base->ring.end());
			boundary_on_middle_towards_base->duality = t_bm;
			t_bm->reverse_duality = boundary_on_middle_towards_base;
			boundary_on_middle_towards_base->cellspace->boundaries.push_back(boundary_on_middle_towards_base);

			t_bm->midpoints.push_back(boundary_on_base_towards_middle->get_centroid());
		}

		if (cell_space_constructed && connections[i].fbase_opposite) {
			Transition* t_mo = new Transition();
			t_mo->u = s;
			t_mo->v = belongs_to.at(connections[i].fbase_opposite)->duality;
			belongs_to.at(connections[i].fbase_opposite)->duality->connects.push_back(t_mo);
			s->connects.push_back(t_mo);
			base_layer->transitions.push_back(t_mo);

			CellSpaceBoundary* boundary_on_opposite_towards_middle = new CellSpaceBoundary();
			boundary_on_opposite_towards_middle->cellspace = t_mo->v->duality;
			boundary_on_opposite_towards_middle->ring = boundary_to_opposite_facet;
			boundary_on_opposite_towards_middle->duality = t_mo;
			t_mo->forward_duality = boundary_on_opposite_towards_middle;
			boundary_on_opposite_towards_middle->cellspace->boundaries.push_back(boundary_on_opposite_towards_middle);

			CellSpaceBoundary* boundary_on_middle_towards_opposite = new CellSpaceBoundary();
			boundary_on_middle_towards_opposite->cellspace = t_mo->u->duality;
			boundary_on_middle_towards_opposite->ring = boundary_to_opposite_facet;
			reverse(boundary_on_middle_towards_opposite->ring.begin(), boundary_on_middle_towards_opposite->ring.end());
			boundary_on_middle_towards_opposite->duality = t_mo;
			t_mo->reverse_duality = boundary_on_middle_towards_opposite;
			boundary_on_middle_towards_opposite->cellspace->boundaries.push_back(boundary_on_middle_towards_opposite);

			t_mo->midpoints.push_back(boundary_on_opposite_towards_middle->get_centroid());
		}
	}

	// NON-NAVIGABLES
	Layer* nonnavi_layer = new Layer();
	for (int i = 0; i < objects.size(); ++i) {
		//cout << "OBJECT" << endl;
		Point3D p = objects[i].bounding_box.min;
		Point3D q = objects[i].bounding_box.max;
		
		Vertex *v1 = new Vertex(p.x, p.y, p.z);
		Vertex *v2 = new Vertex(q.x, p.y, p.z);
		Vertex *v3 = new Vertex(p.x, q.y, p.z);
		Vertex *v4 = new Vertex(q.x, q.y, p.z);
		Vertex *v5 = new Vertex(p.x, p.y, q.z);
		Vertex *v6 = new Vertex(q.x, p.y, q.z);
		Vertex *v7 = new Vertex(p.x, q.y, q.z);
		Vertex *v8 = new Vertex(q.x, q.y, q.z);

		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		vertices.push_back(v4);
		vertices.push_back(v5);
		vertices.push_back(v6);
		vertices.push_back(v7);
		vertices.push_back(v8);

		Facet* f1 = Facet::create_facet(v1, v3, v4, v2);
		Facet* f2 = Facet::create_facet(v5, v6, v8, v7);
		Facet* f3 = Facet::create_facet(v5, v1, v2, v6);
		Facet* f4 = Facet::create_facet(v5, v7, v3, v1);
		Facet* f5 = Facet::create_facet(v7, v8, v4, v3);
		Facet* f6 = Facet::create_facet(v8, v6, v2, v4);

		/*f1->triangulate();
		f2->triangulate();
		f3->triangulate();
		f4->triangulate();
		f5->triangulate();
		f6->triangulate();*/

		facets.push_back(f1);
		facets.push_back(f2);
		facets.push_back(f3);
		facets.push_back(f4);
		facets.push_back(f5);
		facets.push_back(f6);

		CellSpace *c = new CellSpace();
		c->facets.push_back(f1);
		c->facets.push_back(f2);
		c->facets.push_back(f3);
		c->facets.push_back(f4);
		c->facets.push_back(f5);
		c->facets.push_back(f6);
		c->cellspace_type = CellSpace::TYPE_CELLSPACE::TYPE_NONNAVIGABLE;
		c->tag = objects[i].tag;

		State *s = new State();
		s->p = c->get_centroid();
		s->state_type = State::TYPE_STATE::STATE_NORMALSTATE;
		s->duality = c;
		c->duality = s;
		
		this->cellspaces.push_back(c);
		this->layers[0]->states.push_back(s);
	}

}
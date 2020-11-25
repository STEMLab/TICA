#include <string>
#include <vector>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include "World.h"
#include "InFactoryQueryBuilder.h"

struct KeyValue {
	KeyValue(const std::string& _k, const std::string& _v, char _open = '\"', char _close = '\"')
		:key(_k), value(_v), o(_open), c(_close) {
		;
	}
	std::string key;
	std::string value;
	char o;
	char c;

	friend std::ostream& operator << (std::ostream& os, const KeyValue& kv) {
		os << '\"' << kv.key << '\"' << ':';
		os << kv.o << kv.value << kv.c;
		return os;
	}
};


InFactoryPostQuery::InFactoryPostQuery(const std::string& _u, const std::string& _d)
	:url(_u), data(_d) {
	;
}

static std::string generate_uuid(void) {
	std::string s;
	do {
		s = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
		if (!s.empty() && s[0] < '0' || '9' < s[0]) break;
	} while (1);
	return s;
}

using namespace std;

bool InFactoryQueryBuilder::initialize(void) {
	if (server_url.empty()) {
		server_url = "http://127.0.0.1:9797/";
	}
	if (server_url[server_url.size() - 1] != '/') server_url.push_back('/');

	doc_id = generate_uuid();
	string document_url = server_url + "documents/" + doc_id;
	{
		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("id", doc_id);
		json_data << '}';
		query.push_back(InFactoryPostQuery(document_url, json_data.str()));
	}


	ifs_id = generate_uuid();
	//http://127.0.0.1:9797/documents/f4235629-e190-707c-11c0-3b1afc37cdc5/indoorfeatures/bd522086-7e40-e768-d787-f58b3d87b3fc 
	string ifs_url = server_url + "documents/" + doc_id + "/indoorfeatures/" + ifs_id;
	{
		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("id", ifs_id);
		json_data << '}';
		query.push_back(InFactoryPostQuery(ifs_url, json_data.str()));
	}

	psf_id = generate_uuid();
	string psf_url = server_url + "documents/" + doc_id + "/primalspacefeatures/" + psf_id;
	{
		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", ifs_id) << ',';
		json_data << KeyValue("id", psf_id);
		json_data << '}';
		query.push_back(InFactoryPostQuery(psf_url, json_data.str()));
	}
	return true;
}

bool InFactoryQueryBuilder::add_world(const World& w) {
	for (int i = 0; i < w.cellspaces.size(); ++i) {
		add_cellspace(w.cellspaces[i]);
	}
	for (int i = 0; i < w.layers.size(); ++i) {
		for (int j = 0; j < w.layers[i]->states.size(); ++j) {
			add_state(w.layers[i]->states[j]);
		}
		for (int j = 0; j < w.layers[i]->transitions.size(); ++j) {
			add_transition(w.layers[i]->transitions[j]);
		}
	}
	for (int i = 0; i < w.interlayer_connections.size(); ++i) {
		add_interlayerconnection(w.interlayer_connections[i]);
	}
	return true;
}

bool InFactoryQueryBuilder::add_cellspace(CellSpace* c) {
	string id = "";
	switch (c->cellspace_type) {
	case CellSpace::TYPE_CELLSPACE::TYPE_CORRIDOR: id = "CORRIDOR";  break;
	case CellSpace::TYPE_CELLSPACE::TYPE_ROOM: id = "ROOM"; break;
	case CellSpace::TYPE_CELLSPACE::TYPE_DOOR: id = "DOOR"; break;
	case CellSpace::TYPE_CELLSPACE::TYPE_EXTERIORDOOR: id = "EXTERIORDOOR"; break;
	case CellSpace::TYPE_CELLSPACE::TYPE_NONNAVIGABLE: id = "NON-NAVI" + (c->tag.empty()?"":"-"+c->tag); break;
	default: return false;
	}
	cellspace[id].push_back(c);
	int id_no = cellspace[id].size();

	stringstream ss;
	ss << id << id_no;
	cellspace_id[c] = ss.str();
	return true;
}
bool InFactoryQueryBuilder::add_state(State* s) {
	string id = "";
	switch (s->state_type) {
	case State::TYPE_STATE::STATE_NORMALSTATE: id = "S"; break;
	case State::TYPE_STATE::STATE_FIREEXTINGUISHER: id = "EXTINGUISHER"; break;
	case State::TYPE_STATE::STATE_SPRINKLER: id = "SPRINKLER"; break;
	case State::TYPE_STATE::STATE_INDOORHYDRANT: id = "INDOOR-HYDRANT"; break;
	case State::TYPE_STATE::STATE_ALARM: id = "ALARM"; break;
	case State::TYPE_STATE::STATE_DETECTOR: id = "DETECTOR"; break;
	default: return false;
	}
	
	int id_no = 1;
	if (s->state_type == State::TYPE_STATE::STATE_NORMALSTATE) {
		if (s->duality->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_NONNAVIGABLE) {
			nonnavi_state.push_back(s);
		}
		else {
			baselayer_state.push_back(s);
		}
		id_no = baselayer_state.size() + nonnavi_state.size();
	}
	else {
		ps_state[id].push_back(s);
		id_no = ps_state[id].size();
	}

	stringstream ss;
	ss << id << id_no;
	state_id[s] = ss.str();

	return true;
}
bool InFactoryQueryBuilder::add_transition(Transition* t) {
	// [NOTE]
	// in the case of door-to-(Room || corridor):

	//   t->u 는 door or exteriordoor 이어야 함!
	//   t->v 는 room or corridor
	
	//   u에 있는 boundary가 reverse
	//   v에 있는 boundary가 정방향
	
	//   u <-- v 가 reverse
	//   u --> v 가 정방향
	
	/* disabled checking function
	if (!t->u->duality->is_door()) {
		cerr << "NOT A DOOR" << endl;
		return false;
	}
	*/
	
	string id = "";
	if (t->v->duality->is_corridor()){
		id = "CORRIDORSIDE-DOOR";
	}
	else if(t->v->duality->is_room()) {
		id = "ROOMSIDE-DOOR";
	}
	else {
		id = "DOOR";
		//cerr << "NO COR OR ROOM DETECTED" << endl;
		//return false;
	}	

	{
		transition.push_back(t);
		int id_no = transition.size();
		stringstream ss;
		ss << "T" << id_no;
		transition_id[t] = ss.str();
	}

	// add boundary surface
	{
		cellspace_boundary[id].push_back(t->forward_duality);
		cellspace_boundary_reverse[id].push_back(t->reverse_duality);
		int id_no = cellspace_boundary[id].size();
		stringstream ss;
		ss << id << id_no;
		cellspace_boundary_id[t->forward_duality] = ss.str();
		ss << "-REVERSE";
		cellspace_boundary_id[t->reverse_duality] = ss.str();
	}

	return true;
}

void InFactoryQueryBuilder::add_interlayerconnection(InterlayerConnection* i) {
	interlayerconnection.push_back(i);
	stringstream id;
	id << "I" << interlayerconnection.size();
	interlayerconnection_id[i] = id.str();
}

void InFactoryQueryBuilder::query_cellspace(CellSpace* c) {
	string id = cellspace_id.at(c);
	string post_url = server_url + "/documents/" + doc_id + "/cellspace/" + id;

	bool reversed = false;
	if (c->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_NONNAVIGABLE) {
		reversed = true;
	}

	stringstream json_data;
	json_data << "{";
	json_data << KeyValue("id", id) << ',';
	json_data << KeyValue("parentId", psf_id) << ',';
	json_data << KeyValue("docId", doc_id) << ',';
	json_data << KeyValue("type", "CellSpace") << ',';
	
	{
		stringstream geometry;
		geometry << KeyValue("type", "Solid") << ',';
		{
			stringstream coordinates;
			coordinates << "SOLID ((";

			for (int i = 0; i < c->facets.size(); ++i) {
				if (i > 0) { coordinates << ", "; }
				coordinates << "(";
				{
					coordinates << "(";
					FacetEdge* e_start = c->facets[i]->get_exteior_edge();
					FacetEdge* e = e_start;
					{
						Point3D p = e->get_u()->to_point3();
						coordinates << p.x << ' ' << p.y << ' ' << p.z;
					}
					do {
						//MODIFIED_2019_06_12
						//e = e->next();
						if (reversed) e = e->next();
						else e = e->prev();

						Point3D p = e->get_u()->to_point3();
						coordinates << ", " << p.x << ' ' << p.y << ' ' << p.z;
					} while (e != e_start);
					coordinates << ")";

					for (int j = 0; j < c->facets[i]->num_holes(); ++j) {
						coordinates << ",(";
						FacetEdge* e_start = c->facets[i]->get_hole_edge(j);
						FacetEdge* e = e_start;
						{
							Point3D p = e->get_u()->to_point3();
							coordinates << p.x << ' ' << p.y << ' ' << p.z;
						}
						do {
							//MODIFIED_2019_06_12
							//e = e->next();
							if (reversed) e = e->next();
							else e = e->prev();

							Point3D p = e->get_u()->to_point3();
							coordinates << ", " << p.x << ' ' << p.y << ' ' << p.z;
						} while (e != e_start);
						coordinates << ")";
					}
				}
				coordinates << ")";
			}

			coordinates << "))";
			geometry << KeyValue("coordinates", coordinates.str()) << ',';
		}
		{
			stringstream geometry_property;
			geometry_property << KeyValue("id", "CG-" + id) << ',';
			geometry_property << KeyValue("type", "wkt") << ',';
			geometry_property << KeyValue("extrude", "true");

			geometry << KeyValue("properties", geometry_property.str(), '{', '}');
		}

		json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
	}

	{
		stringstream properties;
		properties << KeyValue("name", id) << ',';

		{
			stringstream partialboundedBy;
			for (int i = 0; i < c->boundaries.size(); ++i) {
				if (i > 0) partialboundedBy << ',';
				partialboundedBy << '\"' << cellspace_boundary_id.at(c->boundaries[i]) << '\"';
			}

			// Texture Surface
			if (textured) {
				if (c->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_CORRIDOR) {
					for (int k = 0; k < c->facets.size(); ++k) {
						stringstream id;
						id << cellspace_id.at(c) << "-TEXTURE-" << (k + 1);
						partialboundedBy << ",\"" << id.str() << '\"';
					}
				}
			}

			properties << KeyValue("partialboundedBy", partialboundedBy.str(), '[', ']') << ',';
		}
		properties << KeyValue("duality", state_id.at(c->duality));

		json_data << KeyValue("properties", properties.str(), '{', '}');
	}
	
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));
}
void InFactoryQueryBuilder::query_cellspace_boundary(CellSpaceBoundary* b) {
	string id = cellspace_boundary_id.at(b);
	string post_url = server_url + "/documents/" + doc_id + "/cellspaceboundary/" + id;

	stringstream json_data;
	json_data << "{";

	json_data << KeyValue("id", id) << ',';
	json_data << KeyValue("parentId", psf_id) << ',';
	json_data << KeyValue("docId", doc_id) << ',';
	json_data << KeyValue("type", "CellSpaceBoundary") << ',';
	
	{
		stringstream geometry;
		geometry << KeyValue("type", "Surface") << ',';
		{
			stringstream coordinates;
			coordinates << "POLYGON ((";

			//MODIFIED_2019_06_12
			//for (int i = 0; i <= b->ring.size(); ++i) {
			for (int i = b->ring.size(); i >= 0; --i) {
				Point3D p = b->ring[i%b->ring.size()]->to_point();

				//if (i > 0) coordinates << ", ";
				if (i < b->ring.size()) coordinates << ", ";
				coordinates << p.x << ' ' << p.y << ' ' << p.z;
			}

			coordinates << "))";
			geometry << KeyValue("coordinates", coordinates.str()) << ',';
		}
		{
			stringstream geometry_property;
			geometry_property << KeyValue("id", "CBG-" + id) << ',';
			geometry_property << KeyValue("type", "wkt") << ',';
			geometry_property << KeyValue("extrude", "false");

			geometry << KeyValue("properties", geometry_property.str(), '{', '}');
		}

		json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
	}

	{
		stringstream properties;
		properties << KeyValue("name", id) << ',';
		properties << KeyValue("description", "") << ',';
		string duality_id = transition_id.at(b->duality);
		if (b->duality->reverse_duality == b) {
			duality_id += "-REVERSE";
		}
		properties << KeyValue("duality", duality_id);

		json_data << KeyValue("properties", properties.str(), '{', '}');
	}
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));

}

void InFactoryQueryBuilder::query_texture_surface(const string& id, Facet* f) {
	string post_url = server_url + "/documents/" + doc_id + "/cellspaceboundary/" + id;

	stringstream json_data;
	json_data << "{";

	json_data << KeyValue("id", id) << ',';
	json_data << KeyValue("parentId", psf_id) << ',';
	json_data << KeyValue("docId", doc_id) << ',';
	json_data << KeyValue("type", "CellSpaceBoundary") << ',';

	{
		stringstream geometry;
		geometry << KeyValue("type", "Surface") << ',';
		{
			stringstream coordinates;
			coordinates << "POLYGON (";

			{ // exterior
				coordinates << "(";
				FacetEdge *e = f->get_exteior_edge();
				FacetEdge *e_start = e;
				{
					Point3D p = e->get_u()->to_point3();
					coordinates << p.x << ' ' << p.y << ' ' << p.z;
				}
				do {
					Point3D p = e->get_v()->to_point3();
					coordinates << ", ";
					coordinates << p.x << ' ' << p.y << ' ' << p.z;
					e = e->next();
				} while (e != e_start);
				coordinates << ")";
			}

			for (int i = 0; i < f->num_holes(); ++i) {
				coordinates << ",(";
				FacetEdge *e = f->get_hole_edge(i);
				FacetEdge *e_start = e;
				{
					Point3D p = e->get_u()->to_point3();
					coordinates << p.x << ' ' << p.y << ' ' << p.z;
				}
				do {
					Point3D p = e->get_v()->to_point3();
					coordinates << ", ";
					coordinates << p.x << ' ' << p.y << ' ' << p.z;
					e = e->next();
				} while (e != e_start);
				coordinates << ")";
			}

			coordinates << ")";
			geometry << KeyValue("coordinates", coordinates.str()) << ',';
		}
		{
			stringstream geometry_property;
			geometry_property << KeyValue("id", "CBG-" + id) << ',';
			geometry_property << KeyValue("type", "wkt") << ',';
			geometry_property << KeyValue("extrude", "false");

			geometry << KeyValue("properties", geometry_property.str(), '{', '}');
		}

		json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
	}

	{
		stringstream properties;
		properties << KeyValue("name", id) << ',';
		properties << KeyValue("description", "");
		
		json_data << KeyValue("properties", properties.str(), '{', '}');
	}
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));

}

void InFactoryQueryBuilder::query_make_child(const string& child_tag, const string& child_id, const string& parent_id) {
	string post_url = server_url + "/documents/" + doc_id + "/" + child_tag + "/" + child_id;

	stringstream json_data;
	json_data << '{';
	json_data << KeyValue("docId", doc_id) << ',';
	json_data << KeyValue("parentId", parent_id) << ',';
	json_data << KeyValue("id", child_id);
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));
}
void InFactoryQueryBuilder::initialize_layers(void) {
	multilayeredgraph_id = generate_uuid();
	query_make_child("multilayeredgraph", multilayeredgraph_id, ifs_id);

	spacelayers_id = generate_uuid();
	query_make_child("spacelayers", spacelayers_id, multilayeredgraph_id);

	baselayer_id = "base";
	query_make_child("spacelayer", baselayer_id, spacelayers_id);
	baselayer_nodes_id = generate_uuid();
	query_make_child("nodes", baselayer_nodes_id, baselayer_id);
	baselayer_edges_id = generate_uuid();
	query_make_child("edges", baselayer_edges_id, baselayer_id);
	
	pslayer_id = "PSInstallation";
	query_make_child("spacelayer", pslayer_id, spacelayers_id);
	pslayer_nodes_id = generate_uuid();
	query_make_child("nodes", pslayer_nodes_id, pslayer_id);
	pslayer_edges_id = generate_uuid();
	query_make_child("edges", pslayer_edges_id, pslayer_id);


	nonnavilayer_id = "Nonnavi";
	query_make_child("spacelayer", nonnavilayer_id, spacelayers_id);
	nonnavilayer_nodes_id = generate_uuid();
	query_make_child("nodes", nonnavilayer_nodes_id, nonnavilayer_id);
	nonnavilayer_edges_id = generate_uuid();
	query_make_child("edges", nonnavilayer_edges_id, nonnavilayer_id);

	base_to_pslayer_edges_id = generate_uuid();
	query_make_child("interedges", base_to_pslayer_edges_id, multilayeredgraph_id);

	base_to_nonnavi_edges_id = generate_uuid();
	query_make_child("interedges", base_to_nonnavi_edges_id, multilayeredgraph_id);
	/*
	{
		multilayeredgraph_id = generate_uuid();
		string post_url = server_url + "/documents/" + doc_id + "/multilayeredgraph/" + multilayeredgraph_id;

		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", ifs_id) << ',';
		json_data << KeyValue("id", multilayeredgraph_id);
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}

	{
		spacelayers_id = generate_uuid();
		string post_url = server_url + "/documents/" + doc_id + "/spacelayers/" + spacelayers_id;

		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", multilayeredgraph_id) << ',';
		json_data << KeyValue("id", spacelayers_id);
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}

	{
		baselayer_id = "base";
		string post_url = server_url + "/documents/" + doc_id + "/spacelayers/" + baselayer_id;

		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", spacelayers_id) << ',';
		json_data << KeyValue("id", baselayer_id);
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}

	{
		pslayer_id = "PSInstallation";
		string post_url = server_url + "/documents/" + doc_id + "/spacelayers/" + pslayer_id;

		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", spacelayers_id) << ',';
		json_data << KeyValue("id", pslayer_id);
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}

	{
		nonnavilayer_id = "Nonnavi";
		string post_url = server_url + "/documents/" + doc_id + "/spacelayers/" + nonnavilayer_id;

		stringstream json_data;
		json_data << '{';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("parentId", spacelayers_id) << ',';
		json_data << KeyValue("id", nonnavilayer_id);
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}*/
}

void InFactoryQueryBuilder::query_state(State* s) {
	string id = state_id.at(s);
	string post_url = server_url + "/documents/" + doc_id + "/state/" + id;

	string parent_id = baselayer_nodes_id;
	if (s->is_nonnavigable()) parent_id = nonnavilayer_nodes_id;
	if (s->is_public_safty_feature()) parent_id = pslayer_nodes_id;

	stringstream json_data;
	json_data << "{";
	json_data << KeyValue("id", id) << ',';
	json_data << KeyValue("parentId", parent_id) << ',';
	json_data << KeyValue("docId", doc_id) << ',';
	json_data << KeyValue("type", "State") << ',';
	{
		stringstream geometry;
		geometry << KeyValue("type", "Point") << ',';
		{
			// TODO
			stringstream coordinates;
			coordinates << "POINT (";

			coordinates << s->p.x << ' ' << s->p.y << ' ' << s->p.z;

			coordinates << ")";
			geometry << KeyValue("coordinates", coordinates.str()) << ',';
		}
		{
			stringstream geometry_property;
			geometry_property << KeyValue("id", "SG-" + id) << ',';
			geometry_property << KeyValue("type", "wkt");

			geometry << KeyValue("properties", geometry_property.str(), '{', '}');
		}

		json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
	}
	
	{
		stringstream properties;
		properties << KeyValue("name", id) << ',';
		properties << KeyValue("description", "") << ',';
		if (s->duality) {
			properties << KeyValue("duality", cellspace_id.at(s->duality)) << ',';
		}
		
		{
			stringstream connects;
			for (int i = 0; i < s->connects.size(); ++i) {
				if (i > 0) connects << ',';
				connects << '\"' << transition_id.at(s->connects[i]) << '\"';
			}
			properties << KeyValue("connects", connects.str(), '[', ']');
		}
		json_data << KeyValue("properties", properties.str(), '{', '}');
	}
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));
}
void InFactoryQueryBuilder::query_transition(Transition* t) {
	string id = transition_id.at(t);
	string post_url = server_url + "/documents/" + doc_id + "/transition/" + id;

	string parent_id = baselayer_edges_id;
	{ // T1
		stringstream json_data;
		json_data << "{";
		json_data << KeyValue("id", id) << ',';
		json_data << KeyValue("parentId", parent_id) << ',';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("type", "Transition") << ',';
		{
			stringstream geometry;
			geometry << KeyValue("type", "LineString") << ',';
			{
				// TODO
				stringstream coordinates;
				coordinates << "LINESTRING (";

				coordinates << t->v->p.x << ' ' << t->v->p.y << ' ' << t->v->p.z << ", ";

				if (t->midpoints.empty()) {
					coordinates << (t->v->p.x + t->u->p.x) / 2 << ' ' << (t->v->p.y + t->u->p.y) / 2 << ' ' << (t->v->p.z + t->u->p.z) / 2 << ", ";
				}
				else {
					for (int i = t->midpoints.size() - 1; i >= 0; --i) {
						Point3D p = t->midpoints[i];
						coordinates << p.x << ' ' << p.y << ' ' << p.z << ", ";
					}
				}
				coordinates << t->u->p.x << ' ' << t->u->p.y << ' ' << t->u->p.z;

				coordinates << ")";
				geometry << KeyValue("coordinates", coordinates.str()) << ',';
			}
			{
				stringstream geometry_property;
				geometry_property << KeyValue("id", "TG-" + id) << ',';
				geometry_property << KeyValue("type", "wkt");

				geometry << KeyValue("properties", geometry_property.str(), '{', '}');
			}

			json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
		}

		{
			stringstream properties;
			string connects = "\"" + state_id.at(t->v) + "\",\"" + state_id.at(t->u) + "\"";
			properties << KeyValue("connects", connects, '[', ']') << ',';
			properties << KeyValue("name", id) << ',';
			properties << KeyValue("description", "") << ',';
			properties << KeyValue("duality", cellspace_boundary_id.at(t->forward_duality));

			json_data << KeyValue("properties", properties.str(), '{', '}');
		}
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}

	{ // T1-REVERSE
		post_url = post_url + "-REVERSE";
		id = id + "-REVERSE";
		stringstream json_data;
		json_data << "{";
		json_data << KeyValue("id", id) << ',';
		json_data << KeyValue("parentId", parent_id) << ',';
		json_data << KeyValue("docId", doc_id) << ',';
		json_data << KeyValue("type", "Transition") << ',';
		{
			stringstream geometry;			
			geometry << KeyValue("type", "LineString") << ',';
			{
				stringstream coordinates;
				coordinates << "LINESTRING (";
				
				coordinates << t->u->p.x << ' ' << t->u->p.y << ' ' << t->u->p.z << ", ";
				if (t->midpoints.empty()) {
					coordinates << (t->v->p.x + t->u->p.x) / 2 << ' ' << (t->v->p.y + t->u->p.y) / 2 << ' ' << (t->v->p.z + t->u->p.z) / 2 << ", ";
				}
				else {

					for (int i = 0; i < t->midpoints.size(); ++i) {
						Point3D p = t->midpoints[i];
						coordinates << p.x << ' ' << p.y << ' ' << p.z << ", ";
					}
				}
				coordinates << t->v->p.x << ' ' << t->v->p.y << ' ' << t->v->p.z;
				
				coordinates << ")";
				geometry << KeyValue("coordinates", coordinates.str()) << ',';
			}
			
			{
				stringstream geometry_property;
				geometry_property << KeyValue("id", "TG-" + id) << ',';
				geometry_property << KeyValue("type", "wkt");
				
				geometry << KeyValue("properties", geometry_property.str(), '{', '}');
			}

			json_data << KeyValue("geometry", geometry.str(), '{', '}') << ',';
		}

		{
			stringstream properties;

			string connects = "\"" + state_id.at(t->u) + "\",\"" + state_id.at(t->v) + "\"";
			properties << KeyValue("connects", connects, '[', ']') << ',';
			properties << KeyValue("name", id) << ',';
			properties << KeyValue("description", "") << ',';
			properties << KeyValue("duality", cellspace_boundary_id.at(t->reverse_duality));

			json_data << KeyValue("properties", properties.str(), '{', '}');
		}
		json_data << '}';

		query.push_back(InFactoryPostQuery(post_url, json_data.str()));
	}
}

void InFactoryQueryBuilder::query_interlayerconnection(InterlayerConnection* e) {
	string parent_id;
	string layer_id;
	if (e->containee->is_nonnavigable()) {
		parent_id = base_to_nonnavi_edges_id;
		layer_id = nonnavilayer_id;
	}
	else if (e->containee->is_public_safty_feature()) {
		parent_id = base_to_pslayer_edges_id;
		layer_id = pslayer_id;
	}
	else {
		return;
	}

	string id = interlayerconnection_id.at(e);
	string post_url = server_url + "/documents/" + doc_id + "/interlayerconnection/" + id;
	
	stringstream json_data;
	json_data << "{";
	json_data << KeyValue("id", id) << ',';
	json_data << KeyValue("parentId", parent_id) << ',';
	json_data << KeyValue("docId", doc_id) << ',';
	{
		stringstream properties;
		string connects = "\"" + state_id.at(e->container) + "\",\"" + state_id.at(e->containee) + "\"";
		properties << KeyValue("interConnects", connects, '[', ']') << ',';
		
		string connectedLayers = "\"" + baselayer_id + "\",\"" + layer_id + "\"";
		properties << KeyValue("connectedLayers", connectedLayers, '[', ']') << ',';
		
		properties << KeyValue("typeOfTopoExpression", "CONTAINS");

		json_data << KeyValue("properties", properties.str(), '{', '}');
	}
	json_data << '}';

	query.push_back(InFactoryPostQuery(post_url, json_data.str()));
}

void InFactoryQueryBuilder::build(bool t) {
	textured = t;

	initialize();
	initialize_layers();
	for (auto i = this->cellspace.begin(); i != this->cellspace.end(); ++i) {
		for (int j = 0; j < i->second.size(); ++j) {
			query_cellspace(i->second[j]);

			// TEXTURE SURFACE
			if (textured) {
				if (i->second[j]->cellspace_type == CellSpace::TYPE_CELLSPACE::TYPE_CORRIDOR) {
					for (int k = 0; k < i->second[j]->facets.size(); ++k) {
						stringstream id;
						id << cellspace_id.at(i->second[j]) << "-TEXTURE-" << (k + 1);
						query_texture_surface(id.str(), i->second[j]->facets[k]);
					}
				}
			}
		}
	}
	for (auto i = this->cellspace_boundary.begin(); i != this->cellspace_boundary.end(); ++i) {
		for (int j = 0; j < i->second.size(); ++j) {
			query_cellspace_boundary(i->second[j]);
			query_cellspace_boundary(cellspace_boundary_reverse.at(i->first)[j]);
		}
	}
	for (auto i = this->baselayer_state.begin(); i != this->baselayer_state.end(); ++i) {
		query_state(*i);
	}
	for (auto i = this->ps_state.begin(); i != this->ps_state.end(); ++i) {
		for (auto j = i->second.begin(); j != i->second.end(); ++j) {
			query_state(*j);
		}
	}
	for (auto i = this->nonnavi_state.begin(); i != this->nonnavi_state.end(); ++i) {
		query_state(*i);
	}
	for (auto i = this->transition.begin(); i != this->transition.end(); ++i) {
		query_transition(*i);
	}
	for (auto i = this->interlayerconnection.begin(); i != this->interlayerconnection.end(); ++i) {
		query_interlayerconnection(*i);
	}
}

/*void InFactoryQueryBuilder::post_cellspace(CellSpace* c) {
	//http://127.0.0.1:9797/documents/f4235629-e190-707c-11c0-3b1afc37cdc5/cellspace/C1 
	string cellspace_id = "..."; // [TODO]
	string post_url = server_url + "/documents/" + doc_id + "/cellspace/" + cellspace_id;
	
	//"id":"cellspace_id
	//"parentId":"psf_id"
	//"docId":"doc_id"
	//"type":"CellSpace"
	//"geometry": WKT..
	//	geometry "property"
	//			"id":"CG-" + cellspace_id
	//			"type":"wkt"
	//			"extrude":"true"
	//"property"
	//"name":"C1"
	//"partialboundedBy":["B"]
	//"duality":"S1"
}*/
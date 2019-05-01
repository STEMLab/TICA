#pragma once
#include "World.h"
#include <string>
#include <map>
#include <vector>

struct InFactoryPostQuery {
	InFactoryPostQuery(const std::string& _u, const std::string& _d);
	std::string url;
	std::string data;
};

struct InFactoryQueryBuilder {
	bool initialize(void);

	std::string server_url;
	std::string doc_id;
	std::string ifs_id;
	std::string psf_id;

	void query_make_child(const std::string& childtag, const std::string& childid, const std::string& parent_id);
	void initialize_layers(void);
	std::string multilayeredgraph_id;
	std::string spacelayers_id;
	std::string baselayer_id;
	std::string baselayer_nodes_id;
	std::string baselayer_edges_id;
	std::string pslayer_id;
	std::string pslayer_nodes_id;
	std::string pslayer_edges_id;
	std::string nonnavilayer_id;
	std::string nonnavilayer_nodes_id;
	std::string nonnavilayer_edges_id;
	std::string base_to_pslayer_edges_id;
	std::string base_to_nonnavi_edges_id;

	bool add_world(const World& w);
	bool add_cellspace(CellSpace*);
	bool add_state(State*);
	bool add_transition(Transition*);
	void add_interlayerconnection(InterlayerConnection*);

	void build(bool t = false);

	std::map<std::string, std::vector<CellSpace*> > cellspace;
	std::map<CellSpace*, std::string> cellspace_id;

	std::map<std::string, std::vector<CellSpaceBoundary*> > cellspace_boundary;
	std::map<std::string, std::vector<CellSpaceBoundary*> > cellspace_boundary_reverse;
	std::map<CellSpaceBoundary*, std::string> cellspace_boundary_id;

	std::vector<State*> baselayer_state;
	std::map < std::string, std::vector<State*> > ps_state;
	std::vector<State*> nonnavi_state;
	std::map<State*, std::string> state_id;

	std::vector<Transition*> transition;
	std::map<Transition*, std::string> transition_id;
	
	std::vector<InterlayerConnection*> interlayerconnection;
	std::map<InterlayerConnection*, std::string> interlayerconnection_id;

	void query_cellspace(CellSpace*);
	void query_texture_surface(const std::string& id, Facet* f);
	void query_cellspace_boundary(CellSpaceBoundary*);
	void query_state(State*);
	void query_transition(Transition*);
	void query_interlayerconnection(InterlayerConnection*);

	std::vector<InFactoryPostQuery> query;

	bool textured;
};
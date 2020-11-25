#pragma once

#include "GeomObjects.h"
#include "PointCloud.h"
#include <vector>
#include <string>
#include <map>

struct Connection {
	Facet* fbase;
	Facet* fbase_opposite;
	std::vector<Point3D> ring;
	Vector3D get_normal_of_ring(void) const;
	void reverse(void);
};

struct State;
struct Transition;
struct CellSpace;
struct CellSpaceBoundary;

struct Transition {
	State* u;
	State* v;
	std::vector<Point3D> midpoints;
	CellSpaceBoundary* forward_duality;
	CellSpaceBoundary* reverse_duality;
};

struct InterlayerConnection {
	State* container;
	State* containee;
};

struct Layer {
	std::string id;
	std::vector<State*> states;
	std::vector<Transition*> transitions;
};

struct State {
	enum TYPE_STATE {
		STATE_NORMALSTATE,
		STATE_FIREEXTINGUISHER,
		STATE_INDOORHYDRANT,
		STATE_ALARM,
		STATE_SPRINKLER,
		STATE_DETECTOR,
	} state_type;
	Point3D p;
	CellSpace *duality;
	std::vector<Transition*> connects;

	bool is_nonnavigable(void) const;
	bool is_public_safty_feature(void) const;
};

struct CellSpace {
	enum TYPE_CELLSPACE {
		TYPE_ROOM,
		TYPE_CORRIDOR,
		TYPE_DOOR,
		TYPE_EXTERIORDOOR,
		TYPE_NONNAVIGABLE,
	} cellspace_type;
	std::vector< Facet* > facets;
	std::vector< CellSpaceBoundary* > boundaries;
	std::string tag;
	State *duality;

	Point3D get_centroid(void) const;
	bool is_room(void) const;
	bool is_door(void) const;
	bool is_corridor(void) const;
};

struct CellSpaceBoundary {
	CellSpace* cellspace;
	std::vector<Vertex*> ring;
	Transition *duality;

	Point3D get_centroid(void) const;
};

struct World {
	void clear(void);
	void release(void);

	std::vector<Vertex*> vertices;
	std::vector<Facet*> facets;
	std::vector<PCBox> objects;

	std::vector<Connection> connections;

	void make_cellspaces(void);

	std::map<Facet*, CellSpace*> belongs_to;
	std::vector<CellSpace*> cellspaces;
	std::vector<Layer*> layers;
	std::vector<InterlayerConnection*> interlayer_connections;
};
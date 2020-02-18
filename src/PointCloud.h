#pragma once

#include <GL/glew.h>
#include <iostream>
#include "Geometry.h"
#include <string>

class PointCloud {
public:
	PointCloud();

	void clear(void);

	void set(const float* p, size_t n);

	void draw(float p = 1.0) const;

	bool read_from_pcd(std::istream& in, float sampling = 1.0);

	AxisAlignedBoundingBox3D bounding_box;
	int buffer_id;
	int num_ptrs;

};

class PCBox {
public:
	PCBox();

	void clear(void);
	void draw(void) const;
	bool read_from_pcd(std::istream& in);
	std::string tag;

	AxisAlignedBoundingBox3D bounding_box;
};
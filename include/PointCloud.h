#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "AxisAlignedBoundingBox3D.h"

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
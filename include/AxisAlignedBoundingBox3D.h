#pragma once

#include "Point3D.h"

struct AxisAlignedBoundingBox3D {
	AxisAlignedBoundingBox3D(void);
	AxisAlignedBoundingBox3D(const Point3D& _m, const Point3D& _M);
	Point3D min;
	Point3D max;

	void add(const Point3D&);
	AxisAlignedBoundingBox3D operator + (const AxisAlignedBoundingBox3D&) const;

	void draw(void) const;
};
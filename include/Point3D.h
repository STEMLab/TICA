#pragma once

#include "Defs.h"
#include "Vector3D.h"

struct Point3D {
	Point3D();
	Point3D(coord_t _x, coord_t _y, coord_t _z);

	coord_t x;
	coord_t y;
	coord_t z;

	Point3D operator - (void) const;
	Vector3D operator - (const Point3D& p) const;
	Point3D operator + (const Vector3D& v) const;
	Point3D operator - (const Vector3D& v) const;
	bool operator == (const Point3D& v) const;
};

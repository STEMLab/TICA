#pragma once

#include "Defs.h"
#include "Vector3D.h"
#include "Point3D.h"
#include "Point2D.h"
#include "Line3D.h"

struct Plane {
	Plane();
	Plane(const Point3D& _p, const Vector3D& _x, const Vector3D& _y);
	Point3D p;
	Vector3D x;
	Vector3D y;

	Point3D convert(const Point2D& q) const;
	Point2D convert(const Point3D& q, length_t *d = 0) const;

	bool is_parallel(const Plane& plane) const;
	Line3D intersect(const Plane&) const;
};
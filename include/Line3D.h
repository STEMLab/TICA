#pragma once

#include "Point3D.h"
#include "Vector3D.h"

struct Line3D {
	Line3D();
	Line3D(const Point3D&, const Vector3D&);

	Point3D p;
	Vector3D v;
};
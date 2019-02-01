#pragma once

#include "Defs.h"
#include "Point3D.h"
#include "Vector3D.h"

struct Ray3D {
	Ray3D(const Point3D& _p, const Vector3D& _v);

	Point3D p;
	Vector3D v;
};
#pragma once

#include "Defs.h"
#include "Point3D.h"

struct Point3DAccumulator {
	Point3DAccumulator(void);

	coord_t x;
	coord_t y;
	coord_t z;

	int n;

	Point3DAccumulator& operator+= (const Point3D& v);
	Point3DAccumulator operator+ (const Point3D& v) const;

	Point3D get_average_point(void) const;
};
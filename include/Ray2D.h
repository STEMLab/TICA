#pragma once

#include "Vector2D.h"
#include "Point2D.h"

struct Ray2D {
	Ray2D(const Point2D& _p, const Vector2D& _v);
	Point2D p;
	Vector2D v;
};
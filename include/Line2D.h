#pragma once

#include "Vector2D.h"
#include "Point2D.h"

struct Line2D {
	Line2D(const Point2D& _p, const Vector2D& _v);
	Point2D p;
	Vector2D v;

	Point2D get_projection_of(const Point2D& p, length_t *alpha) const;
};
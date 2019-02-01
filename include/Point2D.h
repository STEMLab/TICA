#pragma once

#include "Defs.h"
#include "Vector2D.h"

struct Point2D {
	Point2D();
	Point2D(coord_t _x, coord_t _y);
	coord_t x;
	coord_t y;

	Vector2D operator - (const Point2D& p) const;
	Point2D operator + (const Vector2D& v) const;
	Point2D operator - (const Vector2D& v) const;
	bool operator == (const Point2D& v) const;
};
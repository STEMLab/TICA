#pragma once

#include "Defs.h"

struct Vector2D {
	Vector2D(void);
	Vector2D(coord_t _x, coord_t _y);
	coord_t x;
	coord_t y;

	Vector2D normalize(void) const;
	scalar_t dot_product(const Vector2D& v) const;
	area_t signed_area(const Vector2D& v) const;
	length_t length(void) const;
	Vector2D operator + (const Vector2D& v) const;
	Vector2D operator - (const Vector2D& v) const;
	Vector2D operator - (void) const;
	friend Vector2D operator * (scalar_t a, const Vector2D& v);
};
#pragma once

#include "Defs.h"

struct Vector3D {
	Vector3D(void);
	Vector3D(coord_t _x, coord_t _y, coord_t _z);

	coord_t x;
	coord_t y;
	coord_t z;

	scalar_t dot_product(const Vector3D& v) const;
	Vector3D cross_product(const Vector3D& v) const;
	Vector3D normalized(void) const;
	length_t length(void) const;
	Vector3D rotate(const Vector3D& q, radian_t theta) const;

	Vector3D get_projection_component_to(const Vector3D&) const;
	Vector3D get_perpendicular_component_to(const Vector3D&) const;

	Vector3D operator + (const Vector3D& v) const;
	Vector3D operator - (const Vector3D& v) const;
	friend Vector3D operator * (scalar_t a, const Vector3D& v);
	Vector3D operator * (scalar_t a) const;
	Vector3D operator / (scalar_t a) const;

	bool is_zero(void) const;
};

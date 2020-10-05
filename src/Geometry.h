#pragma once

#include "Common.h"
#include <vector>

struct Point2D;
struct Point2D;
struct Vector2D;
struct Vector3D;

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

struct Point3D {
	Point3D();
	Point3D(coord_t _x, coord_t _y, coord_t _z);

	coord_t x;
	coord_t y;
	coord_t z;

	Vector3D to_vector(void) const;

	Point3D operator - (void) const;
	Vector3D operator - (const Point3D& p) const;
	Point3D operator + (const Vector3D& v) const;
	Point3D operator - (const Vector3D& v) const;
	bool operator == (const Point3D& v) const;
};

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


struct Vector3D {
	Vector3D(void);
	Vector3D(coord_t _x, coord_t _y, coord_t _z);

	coord_t x;
	coord_t y;
	coord_t z;

	Point3D to_point(void) const;

	scalar_t dot_product(const Vector3D& v) const;
	Vector3D cross_product(const Vector3D& v) const;
	Vector3D normalized(void) const;
	length_t length(void) const;
	length_t length_square(void) const;
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


struct AxisAlignedBoundingBox3D {
	AxisAlignedBoundingBox3D(void);
	AxisAlignedBoundingBox3D(const Point3D& _m, const Point3D& _M);
	Point3D min;
	Point3D max;

	void add(const Point3D&);
	AxisAlignedBoundingBox3D operator + (const AxisAlignedBoundingBox3D&) const;
};

struct XYRotatedBoundingBox3D {
	XYRotatedBoundingBox3D(void);

	Point3D min;
	Vector3D axis_u;
	Vector3D axis_v;
	Vector2D xy;
	length_t height;

	void compute(const std::vector<Point3D>&);
};

struct Line2D {
	Line2D(const Point2D& _p, const Vector2D& _v);
	Point2D p;
	Vector2D v;

	Point2D get_projection_of(const Point2D& p, length_t *alpha) const;
};

struct Line3D {
	Line3D();
	Line3D(const Point3D&, const Point3D&);
	Line3D(const Point3D&, const Vector3D&);

	Point3D p;
	Vector3D v;
};

struct Plane {
	Plane();
	Plane(const Point3D& _p, const Vector3D& _h);
	Point3D p;
	Vector3D h;

	Point3D project(const Point3D& q, length_t *d = 0) const;
	Line3D project(const Line3D& l) const;

	bool is_parallel(const Plane& plane) const;
	Line3D intersect(const Plane&) const;

	void get_basis(Vector3D* u, Vector3D *v) const;
};

struct CoordinatedPlane : public Plane {
	CoordinatedPlane();
	CoordinatedPlane(const Point3D& _p, const Vector3D& _x, const Vector3D& _y);

	Point3D convert(const Point2D& q) const;
	Point2D convert(const Point3D& q, length_t *d = 0) const;

	Vector3D x;
	Vector3D y;
};

struct Subspace {
	virtual Point3D get_nearest_point() = 0;
	virtual int dof(void) const = 0;
};
struct Subspace0D : public Subspace {
	Point3D get_nearest_point(const Point3D&);
	int dof(void) const;
};
struct Subspace1D : public Subspace {
	Point3D get_nearest_point(const Point3D&);
	int dof(void) const;
};
struct Subspace2D : public Subspace {
	Point3D get_nearest_point(const Point3D&);
	int dof(void) const;
};
struct Subspace3D : public Subspace {
	Point3D get_nearest_point(const Point3D&);
	int dof(void) const;
};
bool interp(const Point3D& x, const Line3D& l, scalar_t *_alpha);
bool interp(const Line3D& l1, const Line3D& l2, scalar_t *_alpha, scalar_t *_beta);
bool interp(const Point3D& x, const Plane& p, scalar_t *_alpha);
bool interp(const Line3D& l, const Plane& p, scalar_t *_alpha);
bool interp(const Plane& p, const Plane& q, Line3D *_line, Point3D *p_proj, Point3D *q_proj);

float solid_angle(const Point3D& o, const Point3D& p, const Point3D& q, const Point3D& r);
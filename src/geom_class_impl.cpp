#include "Defs.h"
#include "Point2D.h"
#include "Point3D.h"
#include "Point3DAccumulator.h"
#include "AxisAlignedBoundingBox3D.h"
#include "Vector2D.h"
#include "Vector3D.h"
#include "Line2D.h"
#include "Line3D.h"
#include "Ray2D.h"
#include "Ray3D.h"
#include "Plane.h"
#include <cmath>
#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Point2D::Point2D(void) { ; }
Point2D::Point2D(coord_t _x, coord_t _y) : x(_x), y(_y) { ; }
Vector2D Point2D::operator - (const Point2D& p) const { return Vector2D(x - p.x, y - p.y); }
Point2D Point2D::operator + (const Vector2D& v) const { return Point2D(x + v.x, y + v.y); }
Point2D Point2D::operator - (const Vector2D& v) const { return Point2D(x - v.x, y - v.y); }
bool Point2D::operator == (const Point2D& v) const {
	return abs(x - v.x) <= EPSILON
		&& abs(y - v.y) <= EPSILON;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Point3D::Point3D(void) { ; }
Point3D::Point3D(coord_t _x, coord_t _y, coord_t _z) :x(_x), y(_y), z(_z) { ; }

Point3D Point3D::operator - (void) const { return Point3D(-x, -y, -z); }
Vector3D Point3D::operator - (const Point3D& p) const { return Vector3D(x - p.x, y - p.y, z - p.z); }
Point3D Point3D::operator + (const Vector3D& v) const { return Point3D(x + v.x, y + v.y, z + v.z); }
Point3D Point3D::operator - (const Vector3D& v) const { return Point3D(x - v.x, y - v.y, z - v.z); }
//bool Point3D::operator == (const Point3D& v) const { return x == v.x && y == v.y && z == v.z;  }
bool Point3D::operator == (const Point3D& v) const { 
	return abs(x - v.x) <= EPSILON
		&& abs(y - v.y) <= EPSILON
		&& abs(z - v.z) <= EPSILON;
}
/*
Point3DAccumulator Point3D::operator + (const Point3D& v) const {
	Point3DAccumulator acc;
	acc += *this;
	acc += v;
}*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Point3DAccumulator::Point3DAccumulator(void) 
	: x(0), y(0), z(0), n(0) {
	;
}

Point3DAccumulator& Point3DAccumulator::operator += (const Point3D& v) {
	x += v.x; y += v.y; z += v.z;
	++n;
	return *this;
}
Point3DAccumulator Point3DAccumulator::operator + (const Point3D& v) const {
	Point3DAccumulator acc = *this;
	acc += v;
	return acc;
}

Point3D Point3DAccumulator::get_average_point(void) const {
	if (n == 0) return Point3D(0, 0, 0);
	return Point3D(x / n, y / n, z / n);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AxisAlignedBoundingBox3D::AxisAlignedBoundingBox3D(void) { ; }
AxisAlignedBoundingBox3D::AxisAlignedBoundingBox3D(const Point3D& _m, const Point3D& _M) : min(_m), max(_M) { ; }
void AxisAlignedBoundingBox3D::add(const Point3D& p) {
	if (min.x > p.x) min.x = p.x;
	if (min.y > p.y) min.y = p.y;
	if (min.z > p.z) min.z = p.z;

	if (max.x < p.x) max.x = p.x;
	if (max.y < p.y) max.y = p.y;
	if (max.z < p.z) max.z = p.z;
}
AxisAlignedBoundingBox3D AxisAlignedBoundingBox3D::operator + (const AxisAlignedBoundingBox3D& operand) const {
	AxisAlignedBoundingBox3D ret = operand;
	ret.add(max);
	ret.add(min);
	return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector2D::Vector2D(void) { ; }
Vector2D::Vector2D(coord_t _x, coord_t _y) : x(_x), y(_y) { ; }

Vector2D Vector2D::normalize(void) const {
	length_t size = sqrt(dot_product(*this));
	return Vector2D(x / size, y / size);
}

scalar_t Vector2D::dot_product(const Vector2D& v) const { return x * v.x + y * v.y; }
area_t Vector2D::signed_area(const Vector2D& v) const { return x * v.y - y * v.x; }

length_t Vector2D::length(void) const {
	return sqrt(dot_product(*this));
}

Vector2D Vector2D::operator + (const Vector2D& v) const { return Vector2D(x + v.x, y + v.y); }
Vector2D Vector2D::operator - (const Vector2D& v) const { return Vector2D(x - v.x, y - v.y); }
Vector2D Vector2D::operator - (void) const { return Vector2D(-x, -y); }
Vector2D operator * (scalar_t a, const Vector2D& v) { return Vector2D(a*v.x, a*v.y); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3D::Vector3D(void) { ; }
Vector3D::Vector3D(coord_t _x, coord_t _y, coord_t _z) :x(_x), y(_y), z(_z) { ; }

scalar_t Vector3D::dot_product(const Vector3D& v) const { return x * v.x + y * v.y + z * v.z; }
Vector3D Vector3D::cross_product(const Vector3D& v) const {
	float xx = y * v.z - z * v.y;
	float yy = z * v.x - x * v.z;
	float zz = x * v.y - y * v.x;
	return Vector3D(xx, yy, zz);
}
Vector3D Vector3D::normalized(void) const {
	length_t size = length();
	return Vector3D(x / size, y / size, z / size);
}
length_t Vector3D::length(void) const {
	return sqrt(dot_product(*this));

}
Vector3D Vector3D::rotate(const Vector3D& q, radian_t theta) const {

	scalar_t rx = sin(theta / 2) * q.x;
	scalar_t ry = sin(theta / 2) * q.y;
	scalar_t rz = sin(theta / 2) * q.z;
	scalar_t rw = cos(theta / 2);

	scalar_t ix = rw * x + ry * z - rz * y;
	scalar_t iy = rw * y + rz * x - rx * z;
	scalar_t iz = rw * z + rx * y - ry * x;
	scalar_t iw = -rx * x - ry * y - rz * z;

	return Vector3D(
		ix * rw - iw * rx + ry * iz - rz * iy,
		iy * rw - iw * ry + rz * ix - rx * iz,
		iz * rw - iw * rz + rx * iy - ry * ix
	);
}
Vector3D Vector3D::get_projection_component_to(const Vector3D& x) const {
	return x.normalized() * dot_product(x) / sqrt(x.dot_product(x));
}
Vector3D Vector3D::get_perpendicular_component_to(const Vector3D& x) const {
	return *this - get_projection_component_to(x);
}

Vector3D Vector3D::operator + (const Vector3D& v) const { return Vector3D(x + v.x, y + v.y, z + v.z); }
Vector3D Vector3D::operator - (const Vector3D& v) const { return Vector3D(x - v.x, y - v.y, z - v.z); }
Vector3D operator * (scalar_t a, const Vector3D& v) { return Vector3D(a*v.x, a*v.y, a*v.z); }
Vector3D Vector3D::operator * (scalar_t a) const { return Vector3D(a*x, a*y, a*z); }
Vector3D Vector3D::operator / (scalar_t a) const { return Vector3D(x / a, y / a, z / a); }

bool Vector3D::is_zero(void) const {
	return x*x + y*y + z*z < EPSILON;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Line2D::Line2D(const Point2D& _p, const Vector2D& _v) : p(_p), v(_v) { ; }

Point2D Line2D::get_projection_of(const Point2D& _p, length_t *_alpha) const {
	scalar_t denominator = v.dot_product(v);
	if (denominator == 0) {
		if (_alpha) *_alpha = 0;
		return p;
	}
	length_t alpha = (_p - p).dot_product(v) / denominator;
	if (_alpha) *_alpha = alpha;
	return p + alpha * v;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Line3D::Line3D() { ; }
Line3D::Line3D(const Point3D& _p, const Vector3D& _v) : p(_p), v(_v) { ; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ray2D::Ray2D(const Point2D& _p, const Vector2D& _v) : p(_p), v(_v) { ; }
Ray3D::Ray3D(const Point3D& _p, const Vector3D& _v) : p(_p), v(_v) { ; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane(void) { ; }
Plane::Plane(const Point3D& _p, const Vector3D& _x, const Vector3D& _y) : p(_p), x(_x), y(_y) { ; }

Point3D Plane::convert(const Point2D& q) const { return p + q.x*x + q.y*y; }
Point2D Plane::convert(const Point3D& q, length_t *d) const {
	Vector3D denominator_vector = x.cross_product(y);
	scalar_t denominator = denominator_vector.dot_product(denominator_vector);
	Point2D ret;
	ret.x = denominator_vector.dot_product((q - p).cross_product(y)) / denominator;
	ret.y = denominator_vector.dot_product((p - q).cross_product(x)) / denominator;
	if (d) { *d = (q - p).dot_product(denominator_vector) / denominator; }
	return ret;
}

bool Plane::is_parallel(const Plane& plane) const {
	return x.cross_product(y).cross_product(plane.x.cross_product(plane.y)).is_zero();
}

Line3D Plane::intersect(const Plane& plane) const {
	Vector3D u = x.cross_product(y);
	Vector3D v = plane.x.cross_product(plane.y);

	Vector3D q_p = plane.p - p;

	if (q_p.is_zero()) { // planes share a point
		return Line3D(p, u.cross_product(v));
	}

	scalar_t u1v = x.dot_product(v);
	scalar_t u2v = y.dot_product(v);

	bool z1 = (abs(u1v) < EPSILON);
	bool z2 = (abs(u2v) < EPSILON);

	if (z1 && z2) return Line3D(p, Vector3D(0, 0, 0));

	if (z1) {
		return Line3D(p + y*(q_p).dot_product(v) / u2v, x);
	}
	else if (z2) {
		return Line3D(p + x*(q_p).dot_product(v) / u1v, y);
	}
	else {
		scalar_t d = (q_p).dot_product(v);
		Point3D u1 = p + x*(d / u1v);
		Point3D u2 = p + y*(d / u2v);
		return Line3D(u1, u2 - u1);
	}
}
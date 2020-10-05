#include "Geometry.h"
#include <cmath>

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


Vector3D Point3D::to_vector(void) const {
	return Vector3D(x, y, z);
}

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

XYRotatedBoundingBox3D::XYRotatedBoundingBox3D(void)
:min(0,0,0), axis_u(1,0,0), axis_v(0,1,0), xy(0,0), height(0.0) { ; }

void XYRotatedBoundingBox3D::compute(const std::vector<Point3D>& pts){
	if (pts.empty()) {
		*this = XYRotatedBoundingBox3D();
		return;
	}
	min.z = pts[0].z;
	height = 0.0;
	for (int i = 1; i < pts.size(); ++i) {
		if (min.z > pts[i].z) {
			height += min.z - pts[i].z;
			min.z = pts[i].z;
		}
		if (height < pts[i].z - min.z) {
			height = pts[i].z - min.z;
		}
	}

	// TODO: may use the convex hull to improve the complexity
	scalar_t min_area = -1;
	Point3D  ma_p(0, 0, 0);
	Vector2D ma_xy(0, 0);
	Vector3D ma_u(1, 0, 0);
	Vector3D ma_v(0, 1, 0);
	for (int i = 0; i < pts.size(); ++i) {
		for (int j = i + 1; j < pts.size(); ++j) {
			//Vector2D delta(pts[j].x - pts[i].x, pts[j].y - pts[i].y);
			Vector3D u = pts[j] - pts[i];
			u.z = 0;
			if (u.is_zero()) continue;
			u = u.normalized();
			Vector3D v = Vector3D(0, 0, 1).cross_product(u);

			scalar_t min_u, max_u;
			scalar_t min_v, max_v;
			min_u = max_u = pts[0].to_vector().dot_product(u);
			min_v = max_v = pts[0].to_vector().dot_product(v);

			for (int k = 1; k < pts.size(); ++k) {
				scalar_t x = pts[k].to_vector().dot_product(u);
				scalar_t y = pts[k].to_vector().dot_product(v);

				if (min_u > x) min_u = x;
				if (min_v > y) min_v = y;

				if (max_u < x) max_u = x;
				if (max_v < y) max_v = y;
			}

			scalar_t area = (max_u - min_u) * (max_v - min_v);
			if (min_area < 0 || min_area > area) {
				min_area = area;
				min = Point3D(0,0,min.z) + (u * min_u + v * min_v);
				xy = Vector2D(max_u - min_u, max_v - min_v);
				axis_u = u;
				axis_v = v;
			}
		}
	}
	// Post condition:
	//  "min" is the bottom-left corner, "axis_u" and "axis_v" comprise the unit vectors indicating two sides of the bounding box.
	//  "xy" is (width, height).
	
};

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

Point3D Vector3D::to_point(void) const {
	return Point3D(x, y, z);
}

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
length_t Vector3D::length_square(void) const {
	return dot_product(*this);

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
	return x * x + y * y + z * z < EPSILON;
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
Line3D::Line3D(const Point3D& _p, const Point3D& _q) : p(_p), v(_q-_p) { ; }
Line3D::Line3D(const Point3D& _p, const Vector3D& _v) : p(_p), v(_v) { ; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Plane::Plane(void) { ; }
Plane::Plane(const Point3D& _p, const Vector3D& _h) : p(_p), h(_h) { ; }
Point3D Plane::project(const Point3D& q, length_t *d) const {
	length_t l = (p - q).dot_product(h) / h.length();
	if (d) *d = l;
	return q + l * h.normalized();
}
Line3D Plane::project(const Line3D& l) const {
	Point3D proj_p = project(l.p);
	Point3D proj_q = project(l.p + l.v);
	Vector3D proj_v = proj_q - proj_p;
	return Line3D(proj_p, proj_v);
}

CoordinatedPlane::CoordinatedPlane(void) { ; }
CoordinatedPlane::CoordinatedPlane(const Point3D& _p, const Vector3D& _x, const Vector3D& _y) : Plane(_p, _x.cross_product(_y)), x(_x), y(_y) { ; }

Point3D CoordinatedPlane::convert(const Point2D& q) const { return p + q.x*x + q.y*y; }
Point2D CoordinatedPlane::convert(const Point3D& q, length_t *d) const {
	Vector3D denominator_vector = x.cross_product(y);
	scalar_t denominator = denominator_vector.dot_product(denominator_vector);
	Point2D ret;
	ret.x = denominator_vector.dot_product((q - p).cross_product(y)) / denominator;
	ret.y = denominator_vector.dot_product((p - q).cross_product(x)) / denominator;
	if (d) { *d = (q - p).dot_product(denominator_vector) / denominator; }
	return ret;
}

bool Plane::is_parallel(const Plane& plane) const {
	return h.cross_product(plane.h).is_zero();
}

Line3D Plane::intersect(const Plane& plane) const {
	const Vector3D& u = h;
	const Vector3D& v = plane.h;

	Vector3D q_p = plane.p - p;

	Vector3D uv = u.cross_product(v);

	Vector3D vec_p_to_line = u.cross_product(uv);
	scalar_t alpha = (q_p).dot_product(v) / vec_p_to_line.dot_product(v);
	Point3D start_point = p + alpha * vec_p_to_line;

	Vector3D vec_line_to_q = v.cross_product(uv);
	scalar_t beta = (q_p).dot_product(u) / vec_line_to_q.dot_product(u);
	Point3D end_point = plane.p - beta * vec_line_to_q;

	return Line3D(start_point, end_point - start_point);
}

void Plane::get_basis(Vector3D* u, Vector3D* v) const {
	Vector3D ux(1, 0, 0);
	Vector3D uy(0, 1, 0);
	Vector3D uz(0, 0, 1);
	scalar_t dx = h.dot_product(ux);
	scalar_t dy = h.dot_product(uy);
	scalar_t dz = h.dot_product(uz);

	Vector3D base_u, base_v;

	if (dx <= dy && dx <= dz) {
		base_u = ux.get_perpendicular_component_to(h).normalized();
		base_v = h.cross_product(base_u).normalized();
	}
	else if (dy <= dx && dy <= dz) {
		base_u = uy.get_perpendicular_component_to(h).normalized();
		base_v = h.cross_product(base_u).normalized();
	}
	else if (dz <= dx && dz <= dy) {
		base_u = uz.get_perpendicular_component_to(h).normalized();
		base_v = h.cross_product(base_u).normalized();
	}

	if (u) *u = base_u;
	if (v) *v = base_v;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool interp(const Point3D& x, const Line3D& l, scalar_t *_alpha) {
	// x = l.p + alpha * l.v + h
	// where h is perpendicular to l.v
	scalar_t denominator = l.v.dot_product(l.v);
	if (abs(denominator) < EPSILON) return false;
	if (_alpha) {
		*_alpha = (x - l.p).dot_product(l.v) / denominator;
	}
	return true;
}
bool interp(const Line3D& l1, const Line3D& l2, scalar_t *_alpha, scalar_t *_beta) {
	// l1.p + alpha * l1.v + h = l2.p + beta * l2.v
	// where h is perpendicular to both l1.v and l2.v
	Vector3D q_p = l2.p - l1.p;
	Vector3D uv = l1.v.cross_product(l2.v);
	scalar_t denominator = uv.dot_product(uv);
	if (abs(denominator) < EPSILON) return false;
	if (_alpha) {
		*_alpha = (q_p.cross_product(l2.v)).dot_product(uv) / denominator;
	}
	if (_beta) {
		*_beta = (q_p.cross_product(l1.v)).dot_product(uv) / denominator;
	}
	return true;
}
bool interp(const Point3D& x, const Plane& p, scalar_t *_alpha) {
	// x = p.p + alpha * p.h + v
	// where v is a vector perpendicular to p.h
	if (_alpha) {
		*_alpha = (x - p.p).dot_product(p.h) / p.h.dot_product(p.h);
	}
	return true;
}
bool interp(const Line3D& l, const Plane& p, scalar_t *_alpha) {
	// l.p + alpha * l.v = p.p + v
	// where v is a vector perpendicular to p.h
	scalar_t vh = l.v.dot_product(p.h);
	if (abs(vh) < EPSILON) return false;
	if (_alpha) {
		*_alpha = (p.p - l.p).dot_product(p.h) / vh;
	}
	return true;
}

bool interp(const Plane& p, const Plane& q, Line3D *_line, Point3D *p_proj, Point3D *q_proj) {
	Vector3D uv = p.h.cross_product(q.h);
	if (uv.is_zero()) return false;
	Vector3D uuv = p.h.cross_product(uv);
	Vector3D vuv = q.h.cross_product(uv);

	scalar_t d_alpha = uuv.dot_product(q.h);
	scalar_t d_gamma = vuv.dot_product(p.h);
	if (abs(d_alpha) < EPSILON || abs(d_gamma) < EPSILON) return false;

	Vector3D q_p = q.p - p.p;
	Point3D p_p = p.p + (q_p.dot_product(q.h) / d_alpha) * uuv;
	if (p_proj) {
		*p_proj = p_p;
	}
	if (q_proj) {
		*q_proj = q.p - (q_p.dot_product(p.h) / d_gamma) * vuv;
	}
	if (_line) {
		*_line = Line3D(p_p, uv.normalized());
	}
	return true;
}

#include <iostream>
float solid_angle(const Point3D& o, const Point3D& p, const Point3D& q, const Point3D& r) {
	Vector3D a = (p - o).normalized();
	Vector3D b = (q - o).normalized();
	Vector3D c = (r - o).normalized();

	//std::cout << "VEC " << a.x << ' ' << a.y << ' ' << a.z << std::endl;
	//std::cout << "VEC " << b.x << ' ' << b.y << ' ' << b.z << std::endl;
	//std::cout << "VEC " << c.x << ' ' << c.y << ' ' << c.z << std::endl;

	scalar_t abc = a.dot_product(b.cross_product(c));
	scalar_t div = 1 + a.dot_product(b) + a.dot_product(c) + b.dot_product(c);

	scalar_t omega = atan(abc/div);

	//std::cout << "NORM " << omega << '\t' << abc << '\t' << div << '\t';

	if (div < 0) {
		if (omega < 0) omega += PI;
		else omega -= PI;
	}

	//std::cout << omega << std::endl;
	//if ((a + b + c).dot_product((b - a).cross_product(c - a)) < 0) omega = -omega;
	return 2*omega;
}
#include "GeoUtils.h"
#include <cmath>

bool intersect(const Ray3D& r, const Plane& p, coord_t *alpha, Point2D *ptr) {
	const Point3D& pv = r.p;
	const Vector3D& v = r.v;
	scalar_t denominator = v.dot_product(p.x.cross_product(p.y));
	if (denominator == 0.0) return false;
	coord_t a = (p.p - pv).dot_product(p.x.cross_product(p.y)) / denominator;
	if (alpha) { *alpha = a; }
	if (ptr) {
		ptr->x = (p.p - pv).dot_product(v.cross_product(p.y)) / denominator;
		ptr->y = (pv - p.p).dot_product(v.cross_product(p.x)) / denominator;
	}

	if (a < 0) return false;
	return true;
}

bool intersect(const Ray3D& r, const Line3D& l, length_t threshold, coord_t *alpha, coord_t *beta, length_t *distance) {
	//alpha: on ray
	//beta : on line
	const Vector3D uv = r.v.cross_product(l.v);
	scalar_t denominator = uv.dot_product(uv);
	if (denominator == 0.0) return false;
	
	const Vector3D x = r.p - l.p;
	coord_t a = l.v.cross_product(x).dot_product(uv) / denominator;
	if (alpha) *alpha = a;
	if (beta) *beta = r.v.cross_product(x).dot_product(uv) / denominator;

	length_t d = x.dot_product(uv) / (length_t)sqrt(denominator);
	if (distance) *distance = d;

	if (a < 0) return false;
	return (abs(d) <= threshold);
}

bool intersect(const Ray3D& r, const Point3D& p, length_t threshold, coord_t *alpha, length_t *distance) {
	const Vector3D x = r.p - p;
	coord_t a = -x.dot_product(r.v) / r.v.dot_product(r.v);
	if (alpha) *alpha = a;
	length_t d = x.cross_product(r.v).length() / r.v.length();
	if (distance) *distance = d;

	if (a < 0) return false;
	return d <= threshold;
}

bool intersect(const Ray2D& r, const Line2D& l, coord_t* on_ray, coord_t* on_line) {
	area_t denominator = (l.v).signed_area(r.v);
	if (denominator == 0.0) return false;
	coord_t alpha = (l.v).signed_area(l.p - r.p) / denominator;
	coord_t beta = (r.p - l.p).signed_area(r.v) / denominator;
	if (on_ray) *on_ray = alpha;
	if (on_line) *on_line = beta;

	if (alpha < 0) return false;
	return true;

}

scalar_t det(scalar_t x11, scalar_t x12, scalar_t x21, scalar_t x22) {
	return x11*x22 - x12*x21;
}

scalar_t det(scalar_t x11, scalar_t x12, scalar_t x13, scalar_t x21, scalar_t x22, scalar_t x23, scalar_t x31, scalar_t x32, scalar_t x33) {
	return x11 * det(x22, x23, x32, x33) - x12*det(x21, x23, x31, x33) + x13*det(x21, x22, x31, x32);
}
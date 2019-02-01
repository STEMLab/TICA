#pragma once

#include "Plane.h"
#include "Ray2D.h"
#include "Ray3D.h"
#include "Line2D.h"
#include "Line3D.h"

bool intersect(const Ray3D& r, const Plane& p, coord_t *alpha, Point2D *ptr);
bool intersect(const Ray3D& r, const Line3D& l, length_t threshold, coord_t *alpha, coord_t *beta, length_t *distance);
bool intersect(const Ray3D& r, const Point3D& p, length_t threshold, coord_t *alpha, length_t *distance);
bool intersect(const Ray2D& r, const Line2D& l, coord_t* on_ray, coord_t* on_line);

scalar_t det(scalar_t x11, scalar_t x12, scalar_t x21, scalar_t x22);
scalar_t det(scalar_t x11, scalar_t x12, scalar_t x13, scalar_t x21, scalar_t x22, scalar_t x23, scalar_t x31, scalar_t x32, scalar_t x33);
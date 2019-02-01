#pragma once

#include "Defs.h"
#include "Point3D.h"
#include "Vector3D.h"
#include "Ray3D.h"
#include "Point2D.h"

struct Camera {
	Camera(void);

	Ray3D getRay(int x, int y) const;
	Ray3D getEye(void) const;

	Vector3D transform(const Vector3D&) const;
	Point2D project(const Point3D&) const;

	void applyProjectionMatrix(void) const;
	void applyViewMatrix(void) const;

	Point3D center;

	radian_t yaw; // xy plane (rot by z-axis)
	radian_t pitch; // (rot by wing)

	int w;
	int h;

	length_t th_near;
	length_t th_far;
};
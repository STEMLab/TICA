#pragma once
#include "Geometry.h"

struct Camera {
	Camera(void);

	void getRay(int x, int y, Point3D& ray_origin, Vector3D& ray_direction) const;

	Point3D cam_to_world(const Point3D& p) const;
	Point3D world_to_cam(const Point3D& p) const;

	Vector3D transform(const Vector3D&) const;

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

struct RenderBuffer {
	int width;
	int height;
	int buffer_width;
	int buffer_height;

	GLuint frameBufferID;
	GLuint textureID;
	GLuint depthBufferID;

	RenderBuffer(void);

	void clear(void);

	void create(int w, int h);
	void resize(int w, int h);

	void writeon(void);
	void writeend(void);
	void read(void);
	void readend(void);
	void bind(void);
	void unbind(void);
};

class Shader {
public:
	Shader(void);

	void clear(void);

	void create(const char *vcode = 0, const char *fcode = 0);

	int make_program(GLenum type, const char *code);

	void use(void);

	int program;
	int vs;
	int fs;
};
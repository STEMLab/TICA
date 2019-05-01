#pragma once
#include <fstream>
#include <GL/glew.h>

struct Texture2D {
	Texture2D(void);
	
	bool load(void* buf, int w, int h);
	bool load_bin(std::ifstream &);
	bool load_bin(const char *filename);
	bool load_jpg(const char *filename);
	void bind(void) const;
	void unbind(void) const;
	void release(void);
	GLuint texture_id;

	std::string filename;
	//int size;
};

struct TextureCubemap {
	enum CUBE_MAP_TEXTURE_DIRECTION {
		CUBE_MAP_RIGHT = 0,
		CUBE_MAP_LEFT = 1,
		CUBE_MAP_TOP = 2,
		CUBE_MAP_BOTTOM = 3,
		CUBE_MAP_FRONT = 4,
		CUBE_MAP_BACK = 5,
	};
	TextureCubemap(void);
	bool load(const char *filename, CUBE_MAP_TEXTURE_DIRECTION);
	bool load(std::ifstream &, CUBE_MAP_TEXTURE_DIRECTION);
	bool load(void* buf, int size, CUBE_MAP_TEXTURE_DIRECTION d);
	bool load_jpg(const char *filename);
	void postload(void);
	void bind(float dx = 0, float dy = 0, float dz = 0) const;
	void unbind(void) const;
	void release(void);
	GLuint texture_id;
	int size;

	GLfloat SplaneCoefficients[4];
	GLfloat TplaneCoefficients[4];
	GLfloat RplaneCoefficients[4];
};

#include "Geometry.h"
#include <vector>
struct __temp_texture : public Texture2D {
	Point3D p;
	Vector3D u;
	Vector3D v;

	coord_t umin;
	coord_t umax;
	coord_t vmin;
	coord_t vmax;

	void compute_param(const std::vector<Point3D>& ring);
	Point2D convert(const Point3D&) const;
};
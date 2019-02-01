#pragma once
#include <fstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct Texture2D {
	Texture2D(void);
	bool load(const char *filename);
	bool load(std::ifstream &);
	void bind(void) const;
	void unbind(void) const;
	void release(void);
	GLuint texture_id;
	int size;
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
	void postload(void);
	void bind(void) const;
	void unbind(void) const;
	void release(void);
	GLuint texture_id;
	int size;
};
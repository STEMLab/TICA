#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

struct RenderBuffer {
	int width;
	int height;

	GLuint frameBufferID;
	GLuint textureID;
	GLuint depthBufferID;

	RenderBuffer(void);

	void clear(void);

	void create(int w, int h);

	void use(void);

	void read(void);
};
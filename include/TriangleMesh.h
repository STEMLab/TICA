#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>


struct TriangleMesh {
	TriangleMesh(void);
	void load_from_tvr(std::istream& in);
	void draw(void);
	void release(void);

	GLuint point_buffer_id;
	GLuint index_buffer_id;
	int n;
	int m;
};
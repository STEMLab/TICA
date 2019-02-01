#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

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
#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include "Renderer.h"

class ObjModel {
public:
	enum DRAWING_MODE {
		MODE_LINE,
		MODE_TEXTURE,
		MODE_GEOMETRY,
		MODE_INDEX,
	};
	void parse(std::istream& in);
	void init(void);
	void release(void);
	void draw(DRAWING_MODE m, bool use_z_filter, float zmin, float zmax, float tx, float ty, float theta, bool);
	std::vector<float> visibility;

private:
	std::map<std::string, int> idmap;

	std::vector<std::string> textureFile;
	std::vector<float> Rs;
	std::vector<float> Gs;
	std::vector<float> Bs;
	std::vector<std::vector<float>> tri_v;
	std::vector<std::vector<float>> tri_t;

	GLuint  va;
	GLuint*  v_buffer;
	GLuint*  t_buffer;
	GLuint*  s_buffer;
	GLuint*  textureID;
};
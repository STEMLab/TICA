#pragma once
#include "GL/glew.h"
#include <iostream>
#include <string>

struct BinTexture {
	BinTexture();

	void load(std::istream& in);
	void set_scaler(double _a, double _x0, double _y0);

	void set(void) const;
	void convert(double x, double y, double *pu, double *pv);

	bool loaded;
	int width;
	int height;
	double fx;
	double fy;

	double a;
	double x0;
	double y0;

	GLuint texture_id;

	// NOT USED in editing
	// FOR compatibility(rsd file)
	double xmax;
	double ymax;
	std::string ID;
};
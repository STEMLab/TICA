#include "Texture.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

Texture2D::Texture2D(void)
	:texture_id(-1) {
	;
}

bool Texture2D::load(const char *filename) {
	std::ifstream in(filename, std::ios::binary);
	bool ret = load(in);
	in.close();
	return ret;
}
bool Texture2D::load(std::ifstream &in) {
	if (!in) return false;

	in.seekg(0, std::ios::end);
	int l = in.tellg();
	in.seekg(0, std::ios::beg);
	char *buf = new char[l];
	if (!in.read(buf, l)) {
		delete[] buf;
		return false;
	}
	size = (int)(sqrt(l / 3) + 0.1);
	
	release();
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	delete[] buf;

	return true;
}
void Texture2D::bind(void) const {
	if (texture_id != -1) {
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glEnable(GL_TEXTURE_2D);
		glColor3f(1, 1, 1);
	}
}
void Texture2D::unbind(void) const {
	if (texture_id != -1) {
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
void Texture2D::release(void) {
	if (texture_id == -1) {
		glDeleteTextures(1, &texture_id);
		texture_id = -1;
	}
}



TextureCubemap::TextureCubemap(void)
	:texture_id(-1) {
	;
}

bool TextureCubemap::load(const char *filename, CUBE_MAP_TEXTURE_DIRECTION d) {
	std::ifstream in(filename, std::ios::binary);
	bool ret = load(in, d);
	in.close();
	return ret;
}
bool TextureCubemap::load(std::ifstream &in, CUBE_MAP_TEXTURE_DIRECTION d) {
	if (!in) return false;

	in.seekg(0, std::ios::end);
	int l = in.tellg();
	in.seekg(0, std::ios::beg);
	char *buf = new char[l];
	if (!in.read(buf, l)) {
		delete[] buf;
		return false;
	}
	size = (int)(sqrt(l / 3) + 0.1);

	if (texture_id == -1) {
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	}
	
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+d, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	delete[] buf;

	return true;
}
void TextureCubemap::postload(void) {
	if (texture_id == -1) return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	GLfloat SplaneCoefficients[4] = { 1, 0, 0, 0 };
	glTexGenfv(GL_S, GL_OBJECT_PLANE, SplaneCoefficients);

	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	GLfloat TplaneCoefficients[4] = { 0, 0, 1, 0 };
	glTexGenfv(GL_T, GL_OBJECT_PLANE, TplaneCoefficients);

	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	GLfloat RplaneCoefficients[4] = { 0, 1, 0, 0 };
	glTexGenfv(GL_R, GL_OBJECT_PLANE, RplaneCoefficients);


}
void TextureCubemap::bind(void) const {
	if (texture_id != -1) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		glEnable(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		glColor3f(1, 1, 1);
	}
}
void TextureCubemap::unbind(void) const {
	if (texture_id != -1) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}
void TextureCubemap::release(void) {
	if (texture_id == -1) {
		glDeleteTextures(1, &texture_id);
		texture_id = -1;
	}
}
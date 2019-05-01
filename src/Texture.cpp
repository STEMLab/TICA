#include "Texture.h"
#include <GL/glew.h>
#include <cmath>
#include "common.h"
#include "Util.h"

Texture2D::Texture2D(void)
	:texture_id(-1) {
	;
}

bool Texture2D::load_bin(const char *_filename) {
	filename.assign(_filename);
	std::ifstream in(filename, std::ios::binary);
	bool ret = load_bin(in);
	in.close();
	return ret;
}
bool Texture2D::load_bin(std::ifstream &in) {
	if (!in) return false;

	in.seekg(0, std::ios::end);
	int l = in.tellg();
	in.seekg(0, std::ios::beg);
	char *buf = new char[l];
	if (!in.read(buf, l)) {
		delete[] buf;
		return false;
	}
	int img_size = (int)(sqrt(l / 3) + 0.1);
	
	load(buf, img_size, img_size);

	delete[] buf;

	return true;
}
bool Texture2D::load(void* buf, int w, int h) {
	release();
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return true;
}
bool Texture2D::load_jpg(const char *_filename) {
	filename.assign(_filename);
	Image img;
	img.load(_filename);

	unsigned char *buf = new unsigned char[img.width()*img.height() * 3];
	for (int y = 0; y < img.height(); ++y) {

		int linewidth = 3 * img.width();
		int xoffset = 0;
		int yoffset = 3 * img.width() * y;
		int boffset = yoffset + xoffset;

		int destoffset = linewidth * y;
		for (int x = 0; x < linewidth; ++x) {
			buf[destoffset + x] = img.get_image()[boffset + x];
		}
	}
	load((void*)buf, img.width(), img.height());
	delete[] buf;

	img.release();
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
	if (texture_id != -1) {
		glDeleteTextures(1, &texture_id);
		texture_id = -1;
	}
}



TextureCubemap::TextureCubemap(void)
	:texture_id(-1) {
	
	SplaneCoefficients[0] = SplaneCoefficients[2] = SplaneCoefficients[3] = 0;
	SplaneCoefficients[1] = -1;

	TplaneCoefficients[0] = TplaneCoefficients[1] = TplaneCoefficients[3] = 0;
	TplaneCoefficients[2] = 1;

	RplaneCoefficients[1] = RplaneCoefficients[2] = RplaneCoefficients[3] = 0;
	RplaneCoefficients[0] = 1;
}

bool TextureCubemap::load(const char *filename, CUBE_MAP_TEXTURE_DIRECTION d) {
	std::ifstream in(filename, std::ios::binary);
	bool ret = load(in, d);
	in.close();
	return ret;
}
bool TextureCubemap::load(void* buf, int size, CUBE_MAP_TEXTURE_DIRECTION d) {
	if (texture_id == -1) {
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	}

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + d, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	return true;
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

	bool ret = load(buf, size, d);
	
	delete[] buf;

	return ret;
}

#include <iostream>
bool TextureCubemap::load_jpg(const char *filename) {
	Image img;
	img.load(filename);
	
	unsigned char *buf = new unsigned char[(img.width() / 6)*img.height() * 3];

	for (int i = 0; i < 6; ++i) {
		for (int y = 0; y < img.height(); ++y) {

			int linewidth = (3 * img.width() / 6);
			int xoffset = linewidth * i;
			int yoffset = 3 * img.width() * y;
			int boffset = yoffset + xoffset;

			int destoffset = linewidth * y;
			for (int x = 0; x < linewidth; ++x) {
				buf[destoffset + x] = img.get_image()[boffset + x];
			}
		}
		std::cout << "load " << img.height() << " " << i << std::endl;
		load((void*)buf, img.height(), (CUBE_MAP_TEXTURE_DIRECTION)(i));
	}

	delete[] buf;

	postload();
	img.release();
	return true;
}
/*void TextureCubemap::postload(void) {
	GLfloat SplaneCoefficients[4] = { 0, -1, 0, 0 };
	GLfloat TplaneCoefficients[4] = { 0, 0, 1, 0 };
	GLfloat RplaneCoefficients[4] = { 1, 0, 0, 0 };
	postload(SplaneCoefficients, TplaneCoefficients, RplaneCoefficients);
}*/
void TextureCubemap::postload(void) {
	if (texture_id == -1) return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	//GLfloat SplaneCoefficients[4] = { 1, 0, 0, 0 };
	//GLfloat SplaneCoefficients[4] = { 0, -1, 0, -6.012068759};
	//GLfloat SplaneCoefficients[4] = { -0.161568, -0.986861, 0.000784, -5.806258};
	glTexGenfv(GL_S, GL_OBJECT_PLANE, SplaneCoefficients);

	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	//GLfloat TplaneCoefficients[4] = { 0, 0, 1, 0 };
	//GLfloat TplaneCoefficients[4] = { 0.021845, -0.002782, 0.999758, -0.297226};
	glTexGenfv(GL_T, GL_OBJECT_PLANE, TplaneCoefficients);

	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	//GLfloat RplaneCoefficients[4] = { 0, 1, 0, 0 };
	//GLfloat RplaneCoefficients[4] = { 0.98662,	-0.161545, -0.022008, -1.741139};
	glTexGenfv(GL_R, GL_OBJECT_PLANE, RplaneCoefficients);

}
void TextureCubemap::bind(float dx, float dy, float dz) const {
	if (texture_id != -1) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		if (dx || dy || dz) {
			GLfloat s[4] = {
				SplaneCoefficients[0],SplaneCoefficients[1],SplaneCoefficients[2],
				SplaneCoefficients[0] * dx + SplaneCoefficients[1] * dy + SplaneCoefficients[2] * dz + SplaneCoefficients[3]
			};
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_S, GL_OBJECT_PLANE, s);

			GLfloat t[4] = {
				TplaneCoefficients[0],TplaneCoefficients[1],TplaneCoefficients[2],
				TplaneCoefficients[0] * dx + TplaneCoefficients[1] * dy + TplaneCoefficients[2] * dz + TplaneCoefficients[3]
			};
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_T, GL_OBJECT_PLANE, t);

			GLfloat r[4] = {
				RplaneCoefficients[0],RplaneCoefficients[1],RplaneCoefficients[2],
				RplaneCoefficients[0] * dx + RplaneCoefficients[1] * dy + RplaneCoefficients[2] * dz + RplaneCoefficients[3]
			};
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
			glTexGenfv(GL_R, GL_OBJECT_PLANE, r);
		}
		glEnable(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		//glColor3f(1, 1, 1);
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
	if (texture_id != -1) {
		glDeleteTextures(1, &texture_id);
		texture_id = -1;
	}
}

void __temp_texture::compute_param(const std::vector<Point3D>& ring) {
	Vector3D n(0, 0, 0);
	for (int i = 0; i < ring.size(); ++i) {
		n = n + (ring[i] - ring[0]).cross_product(ring[(i + 1) % ring.size()] - ring[0]);
	}
	n = n.normalized();

	if (abs(n.x) >= abs(n.y) && abs(n.x) >= abs(n.z)) {
		u = Vector3D(0, 1, 0);
	}
	else if (abs(n.y) >= abs(n.x) && abs(n.y) >= abs(n.z)) {
		u = Vector3D(0, 0, 1);
	}
	else if (abs(n.z) >= abs(n.x) && abs(n.z) >= abs(n.y)) {
		u = Vector3D(1, 0, 0);
	}
	u = (u - n * u.dot_product(n)).normalized();
	v = n.cross_product(u).normalized();
	p = ring[0];

	umin = umax = vmin = vmax = 0;
	for (int i = 1; i < ring.size(); ++i) {
		coord_t u_i = u.dot_product(ring[i] - ring[0]);
		coord_t v_i = v.dot_product(ring[i] - ring[0]);
		if (u_i < umin) umin = u_i;
		if (u_i > umax) umax = u_i;
		if (v_i < vmin) vmin = v_i;
		if (v_i > vmax) vmax = v_i;
	}
}

Point2D __temp_texture::convert(const Point3D& x) const {
	return Point2D(((x - p).dot_product(u) - umin) / (umax - umin),
		((x - p).dot_product(v) - vmin) / (vmax - vmin));
}
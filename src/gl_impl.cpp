#include "Camera.h"
#include "RenderBuffer.h"
#include "Shader.h"

#include "Defs.h"
#include "Point2D.h"
#include "Point3D.h"
#include "AxisAlignedBoundingBox3D.h"
#include "Vector2D.h"
#include "Vector3D.h"
#include "Line2D.h"
#include "Line3D.h"
#include "Ray2D.h"
#include "Ray3D.h"
#include "Plane.h"

#include "GeoUtils.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

Camera::Camera(void)
	:center(0, 0, 0), yaw(0), pitch(0), th_near(0.1), th_far(1000.0)
{

}

Vector3D Camera::transform(const Vector3D& v) const {
	Vector3D u = v;
	u = u.rotate(Vector3D(0, 1, 0), pitch);
	u = u.rotate(Vector3D(0, 0, 1), yaw);
	return u;
}

Point2D Camera::project(const Point3D& p) const {
	Point3D ray_origin = center;
	Vector3D ray_bl_direction = transform(Vector3D(-1, -1, 1));
	Vector3D ray_br_direction = transform(Vector3D(-1, 1, 1));
	Vector3D ray_tl_direction = transform(Vector3D(-1, -1, -1));

	Plane pl;
	pl.p = ray_origin + ray_bl_direction;
	pl.x = ray_br_direction - ray_bl_direction;
	pl.y = ray_tl_direction - ray_bl_direction;

	Ray3D ray(ray_origin, p - ray_origin);
	Point2D ptr;
	intersect(ray, pl, 0, &ptr);
	ptr.x *= w;
	ptr.y *= h;
	return ptr;
}

Ray3D Camera::getRay( int x, int y ) const {
	coord_t rx = 2 * x / (coord_t)w - 1;
	coord_t ry = 1 - 2 * y / (coord_t)h;

	Point3D ray_origin = center;
	Vector3D ray_direction = transform(Vector3D(-1, rx, ry));
	Ray3D click_ray(ray_origin, ray_direction);
	return click_ray;
}

Ray3D Camera::getEye(void) const {
	Point3D ray_origin = center;
	Vector3D ray_direction = transform(Vector3D(-1, 0, 0));
	Ray3D click_ray(ray_origin, ray_direction);
	return click_ray;
}

void Camera::applyProjectionMatrix(void) const {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(90.0, 1.0, th_near, th_far);
	gluLookAt(0, 0, 0, -1, 0, 0, 0, 0, 1);
}

void Camera::applyViewMatrix(void) const {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef((GLfloat)(-pitch / PI * 180), 0, 1, 0);
	glRotatef((GLfloat)(-yaw / PI * 180), 0, 0, 1);
	glTranslatef(-center.x, -center.y, -center.z);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderBuffer::RenderBuffer(void) :frameBufferID(-1), textureID(-1), depthBufferID(-1) { ; }

void RenderBuffer::clear(void) {
	if (frameBufferID != -1) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
		glDeleteFramebuffers(1, &frameBufferID);
		frameBufferID = -1;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	if (textureID != -1) {
		glDeleteTextures(1, &textureID);
		depthBufferID = -1;
	}
	if (depthBufferID != -1) {
		glDeleteRenderbuffers(1, &depthBufferID);
		depthBufferID = -1;
	}
}

void RenderBuffer::create(int w, int h) {
	clear();

	width = w;
	height = h;

	glGenFramebuffers(1, &frameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferID);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferID);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenRenderbuffers(1, &depthBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureID, 0);
	GLenum db[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, db);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderBuffer::use(void) {
	if (frameBufferID != -1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferID);
		glViewport(0, 0, width, height);
	}
}

void RenderBuffer::read(void) {
	if (frameBufferID != -1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferID);
		glViewport(0, 0, width, height);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(void) :program(-1), vs(-1), fs(-1) { ; }

void Shader::clear(void) {
	if (vs != -1) {
		glDetachShader(program, vs);
		vs = -1;
	}
	if (fs != -1) {
		glDetachShader(program, fs);
		fs = -1;
	}
	if (program != -1) {
		glDeleteProgram(program);
		program = -1;
	}
}

void Shader::create(const char *vcode, const char *fcode) {
	clear();

	if (!vcode) {
		vcode = "#version 120\n"
			"void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }";
	}

	if (!fcode) {
		fcode = "#version 120\n"
			"void main() { gl_FragColor = vec4(0,1,0,1); }";
	}

	if (vcode) vs = make_program(GL_VERTEX_SHADER, vcode);
	if (fcode) fs = make_program(GL_FRAGMENT_SHADER, fcode);

	program = glCreateProgram();
	if (vcode) glAttachShader(program, vs);
	if (fcode) glAttachShader(program, fs);

	glLinkProgram(program);
}

int Shader::make_program(GLenum type, const char *code) {
	int shdr = glCreateShader(type);
	glShaderSource(shdr, 1, &code, 0);
	glCompileShader(shdr);

	GLint success = 0;
	glGetShaderiv(shdr, GL_COMPILE_STATUS, &success);

	if (success) {
		return shdr;
	}
	GLint maxLength = 0;
	glGetShaderiv(shdr, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	std::vector<GLchar> errorLog(maxLength);
	glGetShaderInfoLog(shdr, maxLength, &maxLength, &errorLog[0]);

	for (int i = 0; i < errorLog.size(); ++i) {
		std::cout << errorLog[i];
	}
	std::cout << std::endl;
	return -1;
}

void Shader::use(void) {
	if (program != -1) glUseProgram(program);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DRAW FUNCTIONS ////////////////////////////////////////////////////////////////////////////////////////////////

void AxisAlignedBoundingBox3D::draw(void) const {
	glBegin(GL_QUADS);

	glVertex3f(min.x, min.y, min.z);
	glVertex3f(min.x, max.y, min.z);
	glVertex3f(max.x, max.y, min.z);
	glVertex3f(max.x, min.y, min.z);

	glVertex3f(min.x, min.y, max.z);
	glVertex3f(max.x, min.y, max.z);
	glVertex3f(max.x, max.y, max.z);
	glVertex3f(min.x, max.y, max.z);

	glEnd();
}


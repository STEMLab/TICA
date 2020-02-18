#include <GL/glew.h>
#include "Renderer.h"
#include <vector>

Camera::Camera(void)
	:center(6.5, 3.75, 9.8), yaw(0.85), pitch(-0.9), th_near(0.1), th_far(1000.0)
{

}

Point3D Camera::cam_to_world(const Point3D& p) const {
	Vector3D u(p.x,p.y,p.z);
	u = u.rotate(Vector3D(0, 1, 0), pitch);
	u = u.rotate(Vector3D(0, 0, 1), yaw);
	return center + u;
}

Point3D Camera::world_to_cam(const Point3D& p) const {
	Vector3D u = p - center;
	u = u.rotate(Vector3D(0, 0, 1), -yaw);
	u = u.rotate(Vector3D(0, 1, 0), -pitch);
	return Point3D(0, 0, 0) + u;
}

Vector3D Camera::transform(const Vector3D& v) const {
	Vector3D u = v;
	u = u.rotate(Vector3D(0, 1, 0), pitch);
	u = u.rotate(Vector3D(0, 0, 1), yaw);
	return u;
}

void Camera::getRay(int x, int y, Point3D& ray_origin, Vector3D& ray_direction) const {
	coord_t rx = 2 * x / (coord_t)w - 1;
	coord_t ry = 1 - 2 * y / (coord_t)h;

	ray_origin = center;
	ray_direction = transform(Vector3D(-1, rx, ry));
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

///////////////////////////////////////////////////////////////////////

RenderBuffer::RenderBuffer(void) :frameBufferID(-1), textureID(-1), depthBufferID(-1), width(0), height(0), buffer_width(0), buffer_height(0) { ; }

void RenderBuffer::clear(void) {
	if (frameBufferID != -1) {
		//glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
		//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
		glDeleteFramebuffers(1, &frameBufferID);
		frameBufferID = -1;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	if (textureID != -1) {
		glDeleteTextures(1, &textureID);
		textureID = -1;
	}
	if (depthBufferID != -1) {
		//glDeleteRenderbuffers(1, &depthBufferID);
		glDeleteTextures(1, &depthBufferID);
		depthBufferID = -1;
	}

	width = height = 0;
	buffer_width = buffer_height = 0;
}

#include <iostream>
using namespace std;

void RenderBuffer::create(int w, int h) {
	clear();

	width = w;
	height = h;
	buffer_width = w;
	buffer_height = h;

	glGenFramebuffers(1, &frameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferID);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferID);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	GLenum db[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, db);

	//glGenRenderbuffers(1, &depthBufferID);
	//glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
	glGenTextures(1, &depthBufferID);
	glBindTexture(GL_TEXTURE_2D, depthBufferID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferID, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderBuffer::resize(int w, int h) {
	if (w > buffer_width || h > buffer_height) {
		create(w, h);
	}
	width = w;
	height = h;
}

void RenderBuffer::writeon(void) {
	if (frameBufferID != -1) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferID);
		glViewport(0, 0, width, height);
	}
}

void RenderBuffer::writeend(void) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void RenderBuffer::read(void) {
	if (frameBufferID != -1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferID);
		glViewport(0, 0, width, height);
	}
}

void RenderBuffer::readend(void) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void RenderBuffer::bind(void) {
	glBindTexture(GL_TEXTURE_2D, textureID);
}
void RenderBuffer::unbind(void) {
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderBuffer::bind_depthbuffer(void) {
	glBindTexture(GL_TEXTURE_2D, depthBufferID);
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Shader::Shader(void) :program(-1), vs(-1), fs(-1) { ; }

void Shader::clear(void) {
	if (vs != -1) {
		glDetachShader(program, vs);
		glDeleteShader(vs);
		vs = -1;
	}
	if (fs != -1) {
		glDetachShader(program, fs);
		glDeleteShader(fs);
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

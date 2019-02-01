#include "Button.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

Button::Button(void)
	:font(GLUT_BITMAP_9_BY_15), x(0), y(0), w(0), h(0) {
	;
}

#include <iostream>

using namespace std;
void Button::draw(void) {

	glColor3f(0.8, 0.8, 0.8);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();

	glColor3f(0,0,0);
	glBegin(GL_LINES);
	glVertex2f(x, y); glVertex2f(x+w, y);
	glVertex2f(x + w, y); glVertex2f(x + w, y + h);
	glVertex2f(x + w, y + h); glVertex2f(x, y + h);
	glVertex2f(x, y + h); glVertex2f(x, y);
	glEnd();

	if (font && !caption.empty()) {
		glColor3f(0, 0, 0);
		glRasterPos3f(x+(w-caption_width)/2, y+(h+caption_height)/2, 0);
		glutBitmapString(font, 0);
		glutBitmapString(font, (unsigned char*)caption.c_str());
	}
}

bool Button::hitTest(int _x, int _y) const {
	if (_x < x) return false;
	if (_y < y) return false;
	if (x + w < _x) return false;
	if (y + h < _y) return false;
	return true;
}

void Button::send_clickevent(void) const {
	if (callback_onclick) {
		callback_onclick();
	}
}


void Button::set_caption(const std::string& s) {
	caption = s;
	set_font(font);
}
void Button::set_font(void* f) {
	font = f;
	caption_height = glutBitmapHeight(font)*2/3;
	caption_width = glutBitmapLength(font, (unsigned char*)caption.c_str());
}
#include "BasicViewer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

BasicViewer::BasicViewer()
	: camera_moving_speed(0.05), camera_rotation_speed(0.03), rbuttondown(false) {
	camera.w = glutGet(GLUT_WINDOW_WIDTH);
	camera.h = glutGet(GLUT_WINDOW_HEIGHT);

}
void BasicViewer::onTimer(int value) {
	if (update_camera_pose()) {
		glutPostRedisplay();
	}
}
void BasicViewer::onKeyDown(unsigned char key, int x, int y, int modifier) {
	int t = glutGet(GLUT_ELAPSED_TIME);
	pressed_keys[(int)key] = t;
}
void BasicViewer::onKeyUp(unsigned char key, int x, int y, int modifier) {
	pressed_keys.erase((int)key);
}
void BasicViewer::onSpecialKeyDown(int key, int x, int y, int modifier) {
	int t = glutGet(GLUT_ELAPSED_TIME);
	pressed_special_keys[(int)key] = t;
}
void BasicViewer::onSpecialKeyUp(int key, int x, int y, int modifier) {
	pressed_special_keys.erase((int)key);
}
void BasicViewer::onResize(int w, int h) {
	camera.w = w;
	camera.h = h;
	camera.applyProjectionMatrix();
	glutPostRedisplay();
}
bool BasicViewer::update_camera_pose(void) {
	int d_forward = 0, d_left = 0, d_up = 0;
	int rotate_anticlockwise = 0;
	int rotate_upward = 0;

	if (pressed_keys.count('w')) {
		d_forward += 1;
	}
	if (pressed_keys.count('a')) {
		d_left -= 1;
	}
	if (pressed_keys.count('s')) {
		d_forward -= 1;
	}
	if (pressed_keys.count('d')) {
		d_left += 1;
	}
	if (pressed_keys.count('q')) {
		rotate_anticlockwise += 1;
	}
	if (pressed_keys.count('e')) {
		rotate_anticlockwise -= 1;
	}
	if (pressed_keys.count('t')) {
		d_up += 1;
	}
	if (pressed_keys.count('g')) {
		d_up -= 1;
	}
	if (pressed_keys.count('y')) {
		rotate_upward += 1;
	}
	if (pressed_keys.count('h')) {
		rotate_upward -= 1;
	}
	if (d_forward || d_left || d_up || rotate_anticlockwise || rotate_upward) {
		Vector3D mov_vec(-d_forward, d_left, d_up);
		camera.center = camera.center + camera.transform(mov_vec) * camera_moving_speed;
		camera.yaw += rotate_anticlockwise * camera_rotation_speed;
		camera.pitch += rotate_upward * camera_rotation_speed;
		return true;
	}
	return false;
}

#include <iostream>

void BasicViewer::onMouse(int button, int state, int x, int y, int modifier) {

	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			rbuttondown_x = x;
			rbuttondown_y = y;
			rbuttondown = true;
		}
		else if (state == GLUT_UP) {
			rbuttondown = false;
		}
	}	
}

void BasicViewer::onMouseMotion(int x, int y, int modifier) {
	
	if (rbuttondown) {
		camera.yaw += (rbuttondown_x-x)*camera_rotation_speed;
		camera.pitch += (rbuttondown_y-y)*camera_rotation_speed;

		rbuttondown_x = x;
		rbuttondown_y = y;

		glutPostRedisplay();
	}
}

void BasicViewer::onMouseMove(int x, int y, int modifier) {

}

#include <iostream>

using namespace std;
void BasicViewer::onEntry(int state) {
	if (state == GLUT_LEFT) {
		pressed_keys.clear();
		pressed_special_keys.clear();
	}
}

void BasicViewer::onDraw() {
	camera.applyViewMatrix();
}
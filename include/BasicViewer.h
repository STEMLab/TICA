#pragma once

#include "Viewer.h"

struct BasicViewer : public Viewer {
	BasicViewer(void);

	void onTimer(int value);
	void onKeyDown(unsigned char key, int x, int y, int modifier);
	void onKeyUp(unsigned char key, int x, int y, int modifier);
	void onSpecialKeyDown(int key, int x, int y, int modifier);
	void onSpecialKeyUp(int key, int x, int y, int modifier);
	void onMouse(int button, int state, int x, int y, int modifier);
	void onMouseMotion(int x, int y, int modifier);
	void onMouseMove(int x, int y, int modifier);
	void onResize(int w, int h);
	void onEntry(int state);
	void onDraw(void);

	bool update_camera_pose(void);

	std::map<int, int> pressed_keys;
	std::map<int, int> pressed_special_keys;
	std::map<int, int> button_state;

	Camera camera;
	scalar_t camera_moving_speed;
	scalar_t camera_rotation_speed;

	bool rbuttondown;
	int rbuttondown_x;
	int rbuttondown_y;
};
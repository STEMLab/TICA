#pragma once

#include <map>
#include "Camera.h"

struct Viewer {
	friend class ViewerController;
	virtual void onTimer(int value) = 0;
	virtual void onKeyDown(unsigned char key, int x, int y, int modifier) = 0;
	virtual void onKeyUp(unsigned char key, int x, int y, int modifier) = 0;
	virtual void onSpecialKeyDown(int key, int x, int y, int modifier) = 0;
	virtual void onSpecialKeyUp(int key, int x, int y, int modifier) = 0;
	virtual void onMouse(int button, int state, int x, int y, int modifier) = 0;
	virtual void onMouseMotion(int x, int y, int modifier) = 0;
	virtual void onMouseMove(int x, int y, int modifier) = 0;
	virtual void onEntry(int state) = 0;
	virtual void onResize(int w, int h) = 0;
	virtual void onDraw(void) = 0;
};

void register_viewer(Viewer* _viewer);
void init_viewer_handler(void);
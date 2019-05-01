#pragma once

#include "World.h"

struct Controller {
	Controller(Controller *prev);

	virtual void make_ui(void) = 0;

	virtual void draw_select_scene(void) = 0;
	virtual void draw_scene(void) = 0;
	virtual void post_draw(void) = 0;

	virtual void on_mouse_down(int x, int y, const Line3D& ray, int current_obj) = 0;
	virtual void on_mouse_drag(int x, int y, const Line3D& ray, int current_obj) = 0;
	virtual void on_mouse_hover(int x, int y, const Line3D& ray, int current_obj) = 0;
	virtual void on_mouse_up(int x, int y, const Line3D& ray, int current_obj) = 0;

	static World* world;
	static Controller* current_controller;

	virtual int get_current_stage(void)const;

protected:
	Controller *prev;
	void end();
};
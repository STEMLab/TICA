#pragma once

#include <string>

struct Button {
	Button(void);
	typedef void (*callback_t)(void);

	int x;
	int y;
	int w;
	int h;
	
	callback_t callback_onclick;

	void draw(void);
	bool hitTest(int _x, int _y) const;
	void send_clickevent(void) const;

	void set_caption(const std::string& s);
	void set_font(void*);
private:
	void* font;

	std::string caption;
	int caption_width;
	int caption_height;

};
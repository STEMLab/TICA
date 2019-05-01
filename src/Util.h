#pragma once
void decompose_integer(unsigned int i, unsigned char *b);
unsigned int compose_integer(unsigned char *b);

#include <string>
#include <vector>
#include <map>

std::string open_file_browser(const std::string & path, bool save, const std::string& ext);
std::string get_executable_path(void);

class Image {
public:
	typedef unsigned char value_t;

	Image(void);

	void release(void);
	bool load(const std::string& filepath);

	const value_t *get_image(void) const;
	int width(void) const;
	int height(void) const;
private:
	void alloc_image(int w, int h);
	value_t *img;
	int w, h;
	int dummy;
};
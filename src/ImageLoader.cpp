#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <jpeglib.h>
#include <string>
#include "Util.h"

using namespace std;

Image::Image(void)
	:img((value_t*)&dummy), w(1), h(1), dummy(~0) {

}

void Image::alloc_image(int _w, int _h) {
	release();
	w = _w;
	h = _h;
	img = new value_t[_w*_h * 3];
}

int Image::width(void) const { return w; }
int Image::height(void) const { return h; }

void Image::release(void) {
	if (img && img != (value_t*)(&dummy)) delete[] img;
	img = (value_t*)&dummy; w = 1; h = 1;
}

bool Image::load(const string& filepath) {
	FILE *fp = fopen(filepath.c_str(), "rb");
	if (!fp) return false;
	struct jpeg_decompress_struct cinfo = { 0, };
	struct jpeg_error_mgr pub = { 0, };
	cinfo.err = jpeg_std_error(&pub);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	int row_stride = cinfo.output_width * cinfo.output_components;
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	alloc_image(cinfo.output_width, cinfo.output_height);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(img + (cinfo.output_scanline - 1) * cinfo.output_width * 3, buffer[0], cinfo.output_width * 3);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(fp);
	return true;
}

const Image::value_t *Image::get_image(void) const {
	return img;
}
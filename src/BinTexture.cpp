#include "BinTexture.h"

BinTexture::BinTexture(void)
	:loaded(false) {
	;
}

void BinTexture::load(std::istream& in) {
	if (!in) return;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int w, h;
	in.read((char*)&w, sizeof(w));
	in.read((char*)&h, sizeof(h));

	if (!in) {
		return;
	}

	char *d = new char[w*h * 3];
	if (!in.read(d, w*h * 3)) {
		delete[] d;
		return;
	}

	int ww = 1, hh = 1;
	while (ww < w) ww <<= 1;
	while (hh < h) hh <<= 1;
	if (ww < hh) ww = hh;
	else hh = ww;
	
	char *dd = new char[ww*hh * 3];
	memset(dd, 255, ww*hh * 3);
	for (int i = 0; i < h; ++i) {
		memcpy(dd + i*ww * 3, d + i*w * 3, w * 3);
	}
	delete[] d;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ww, hh, 0, GL_BGR, GL_UNSIGNED_BYTE, dd);

	width = w;
	height = h;
	fx = w / (double)ww;
	fy = h / (double)hh;
	delete[] dd;
	loaded = true;

	glBindTexture(GL_TEXTURE_2D, 0);
}

void BinTexture::set_scaler(double _a, double _x0, double _y0) {
	a = _a;
	x0 = _x0;
	y0 = _y0;
}

void BinTexture::set(void) const {
	if (loaded) glBindTexture(GL_TEXTURE_2D, texture_id);
}

void BinTexture::convert(double x, double y, double *pu, double *pv) {
	double u, v;
	u = fx*a*(x - x0) / (double)width;
	v = fy*a*(y - y0) / (double)height;
	if (pu) *pu = u;
	if (pv) *pv = v;
}

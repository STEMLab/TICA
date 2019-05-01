void decompose_integer(unsigned int i, unsigned char *b) {
	b[0] = ((i >> 0) & 0xff);
	b[1] = ((i >> 8) & 0xff);
	b[2] = ((i >> 16) & 0xff);
	b[3] = ((i >> 24) & 0xff);
}
unsigned int compose_integer(unsigned char *b) {
	return ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]);
}
#include "PointCloud.h"
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

PointCloud::PointCloud() :buffer_id(-1), num_ptrs(0), bounding_box(Point3D(0, 0, 0), Point3D(0, 0, 0)) { ; }

void PointCloud::clear(void) {
	if (buffer_id != -1) {
		glDeleteBuffers(1, (GLuint*)&buffer_id);
		buffer_id = -1;
		num_ptrs = 0;
		bounding_box.min = bounding_box.max = Point3D(0, 0, 0);
	}
}

void PointCloud::set(const float* p, size_t n) {
	clear();
	if (n == 0) return;

	bounding_box.min = bounding_box.max = Point3D(p[0], p[1], p[2]);
	for (int i = 1; i < (int)n; ++i) {
		bounding_box.add(Point3D(p[3 * i + 0], p[3 * i + 1], p[3 * i + 2]));
	}
	glGenBuffers(1, (GLuint*)&buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
	glBufferData(GL_ARRAY_BUFFER, n * 3 * sizeof(float), p, GL_STATIC_DRAW);
	num_ptrs = (int)n;
}

void PointCloud::draw(float p) const {
	if (buffer_id == -1) return;

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glDrawArrays(GL_POINTS, 0, num_ptrs*p);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
}

bool PointCloud::read_from_pcd(std::istream& in, float sampling) {
	// ASSUMPTION:
	// all x,y,z are size-4 floats.
	std::string line;
	
	int unit_size = -1;
	int x_offset = 0;
	int y_offset = 0;
	int z_offset = 0;
	
	std::vector<int> sizes;
	std::vector<int> counts;

	int w = -1;
	int h = -1;
	int n = -1;

	enum DATA_TYPE {
		DATA_TYPE_UNKNOWN,
		DATA_TYPE_BINARY,
		DATA_TYPE_ASCII,
	} type = DATA_TYPE_UNKNOWN;
	
	while (getline(in, line)) {
		if (line.empty() || line[0] == '#') continue;
		
		std::string tag;
		std::stringstream ss(line);
		ss >> tag;
		if (tag == "VERSION") {
			continue;
		}
		else if(tag == "FIELDS" ) {
			std::string fieldname;
			int offset = -1;
			while (ss >> fieldname) {
				++offset;
				if (fieldname.size() != 1) continue;
				switch (fieldname[0]) {
				case'X':case'x': x_offset = offset; continue;
				case'Y':case'y': y_offset = offset; continue;
				case'Z':case'z': z_offset = offset; continue;
				default:; continue;
				}
			}
		}
		else if (tag == "SIZE") {
			int s;
			while (ss >> s) {
				sizes.push_back(s);
			}
		}
		else if (tag == "TYPE") {
			continue;
		}
		else if (tag == "COUNT") {
			int c;
			while (ss >> c) {
				counts.push_back(c);
			}
		}
		else if (tag == "WIDTH") {
			ss >> w;
		}
		else if (tag == "HEIGHT") {
			ss >> h;
		}
		else if (tag == "POINTS") {
			ss >> n;
		}
		else if (tag == "DATA") {
			std::string t;
			ss >> t;
			if (t == "ascii") type = DATA_TYPE_ASCII;
			else if (t == "binary") type = DATA_TYPE_BINARY;
			break;
		}
	}

	unit_size = 0;
	int x_byteoffset = 0;
	int y_byteoffset = 0;
	int z_byteoffset = 0;
	for (size_t i = 0; i < sizes.size(); ++i) {
		if (i == x_offset) x_byteoffset = unit_size;
		else if (i == y_offset) y_byteoffset = unit_size;
		else if (i == z_offset) z_byteoffset = unit_size;

		unit_size += sizes[i] * counts[i];
	}

	if (w == -1) w = 1;
	if (h == -1) h = 1;
	if (n == -1) n = w * h;
	char *buf = new char[unit_size];
	
	int sampled_n = n * sampling;
	if (sampled_n > n) sampled_n = n;
	if (sampled_n == 0) sampled_n = 1;
	float *ptrs = new float[sampled_n * 3];

	std::vector<int> permutation(sampled_n);
	for (int i = 0; i < sampled_n; ++i) {
		permutation[i] = i;
	}
	random_shuffle(permutation.begin(), permutation.end());
	
	for (int j = 0; j < n; ++j) {
		if (!in.read(buf, unit_size)) return false;
		float *x = (float*)(buf + x_byteoffset);
		float *y = (float*)(buf + y_byteoffset);
		float *z = (float*)(buf + z_byteoffset);

		int k = j;
		if (k >= sampled_n) {
			k = (((unsigned long long)rand())*(RAND_MAX+1)+rand()) % (j+1);
			if (k >= sampled_n) continue;
		}
		
		int i = permutation[k];
		ptrs[3 * i + 0] = *x;
		ptrs[3 * i + 1] = *y;
		ptrs[3 * i + 2] = *z;
	}
	delete[] buf;

	set(ptrs, sampled_n);
	delete[] ptrs;

	return true;
}
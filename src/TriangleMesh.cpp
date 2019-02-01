#include "TriangleMesh.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

static void read_vertex(const string& line, vector<float>& xyz) {
	istringstream ss(line.c_str()+1);
	float x, y, z;
	ss >> x >> y >> z;
	xyz.push_back(x);
	xyz.push_back(y);
	xyz.push_back(z);
}

static void read_face(const string& line, vector<int>& idx, int offset) {
	istringstream ss(line.c_str() + 1);
	int a, b, c;
	ss >> a >> b >> c;
	idx.push_back(a + offset);
	idx.push_back(b + offset);
	idx.push_back(c + offset);
}

TriangleMesh::TriangleMesh(void )
	:point_buffer_id(-1), index_buffer_id(-1), n(0), m(0) {
	;
}

void TriangleMesh::load_from_tvr(std::istream& in) {
	string line;
	int current_group_offset = 0;
	vector<float> xyz;
	vector<int> idx;
	n = 0;
	m = 0;
	int lineno = 0;
	int version = -1;

	while (getline(in, line)) {
		if (++lineno % 1000 == 0) cout << lineno << " LINE READ" << endl;
		if (line.empty() ) continue;
		switch (line[0]) {
		case 'T': 
			if (line == "TVR0") version = 0;
			else if (line == "TVR1") version = 1;
		case 'g': 
			if (version == 1)current_group_offset = n;
			break;
		case 'v': read_vertex(line, xyz); ++n; break;
		case 'f': read_face(line, idx, current_group_offset); ++m; break;
		default:continue;
		}
	}
	
	glGenBuffers(1, &point_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(float) * 3, xyz.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * m * sizeof(int), idx.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TriangleMesh::draw(void) {
	glEnableVertexAttribArray(0); //vertex
								  // 0: vertex
								  // 1: normal
								  // 2: texcoord

	glBindBuffer(GL_ARRAY_BUFFER, point_buffer_id);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
	glDrawElements(GL_TRIANGLES, 3*m, GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TriangleMesh::release(void) {
	if (point_buffer_id != -1) {
		glDeleteBuffers(1, &point_buffer_id);
		point_buffer_id = -1;
	}
	if (index_buffer_id != -1) {
		glDeleteBuffers(1, &index_buffer_id);
		index_buffer_id = -1;
	}
	n = 0; m = 0;
}
#include "ObjModel.h"
#include "Util.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

static Shader* objTex_shader = 0;
static Shader* objCol_shader = 0;
static GLuint objCol_shader_color;
static GLuint objTex_shader_color;
static GLuint objCol_shader_visibility;
static GLuint objTex_shader_visibility;

static GLuint objTex_zfilter;
static GLuint objTex_zmin;
static GLuint objTex_zmax;
static GLuint objCol_zfilter;
static GLuint objCol_zmin;
static GLuint objCol_zmax;

struct Mat {
	bool valid;
	string Name;
	float Ka[3];
	float Kd[3];
	float Ks[3];
	string TextureFile;
};

static map<string,Mat> parse_mtl(const string &file) {
	ifstream in(file);
	string line;

	Mat mat;
	mat.valid = false;
	map<string, Mat> ret;

	while (getline(in,line)) {
		line = line.substr(0, line.find_first_of('#'));
		if (line.empty()) continue;
		stringstream ss(line);

		string tag;
		if (!(ss >> tag)) continue;
		if (tag == "newmtl") {
			if (mat.valid) {
				ret[mat.Name] = mat;
				mat.Name.clear();
				mat.valid = false;
				mat.TextureFile.clear();
			}
			ss >> mat.Name;
			mat.valid = true;
		}
		else if (tag == "Ka") {
			ss >> mat.Ka[0] >> mat.Ka[1] >> mat.Ka[2];
		}
		else if (tag == "Kd") {
			ss >> mat.Kd[0] >> mat.Kd[1] >> mat.Kd[2];
		}
		else if (tag == "Ks") {
			ss >> mat.Ks[0] >> mat.Ks[1] >> mat.Ks[2];
		}
		else if (tag == "map_Kd") {
			ss >> mat.TextureFile;
		}
	}
	if (mat.valid) {
		ret[mat.Name] = mat;
	}
	return ret;
}

void ObjModel::parse(istream& in) {
	string line;

	map<string, Mat> matlib;
	string current_mat;
	vector<float> vs;
	vector<float> vts;
	vector<float> vns;

	map<string,vector<float>> tris_v;
	map<string, vector<float>> tris_t;

	while (getline(in, line)) {
		line = line.substr(0, line.find_first_of('#'));
		if (line.empty()) continue;
		stringstream ss(line);
		string t;
		if (!(ss >> t)) continue;

		if (t == "v") {
			float x, y, z;
			ss >> x >> y >> z;
			vs.push_back(x);
			vs.push_back(y);
			vs.push_back(z);
		}
		else if (t == "vt") {
			float u, v;
			ss >> u >> v;
			vts.push_back(u);
			vts.push_back(v);
		}
		else if (t == "vn") {
			float x, y, z;
			ss >> x >> y >> z;
			vns.push_back(x);
			vns.push_back(y);
			vns.push_back(z);
		}
		else if (t == "f") {
			string p, q, r;
			ss >> p >> q >> r;
			string x;
			if (ss >> x) {
				cerr << "Not triangulated" << endl;
			}

			size_t dp = p.find_first_of('/');
			size_t dq = q.find_first_of('/');
			size_t dr = r.find_first_of('/');

			string vp = p.substr(0, dp);
			string vq = q.substr(0, dq);
			string vr = r.substr(0, dr);

			string tp = p.substr(dp+1, p.find_first_of('/', dp+1)-dp-1);
			string tq = q.substr(dq+1, q.find_first_of('/', dq+1)-dq-1);
			string tr = r.substr(dr+1, r.find_first_of('/', dr+1)-dr-1);

			int ip = atoi(vp.c_str()) - 1;
			int iq = atoi(vq.c_str()) - 1;
			int ir = atoi(vr.c_str()) - 1;

			int jp = atoi(tp.c_str()) - 1;
			int jq = atoi(tq.c_str()) - 1;
			int jr = atoi(tr.c_str()) - 1;

			vector<float>& tv = tris_v[current_mat];
			tv.push_back(vs[3 * ip + 0]);
			tv.push_back(vs[3 * ip + 1]);
			tv.push_back(vs[3 * ip + 2]);
			tv.push_back(vs[3 * iq + 0]);
			tv.push_back(vs[3 * iq + 1]);
			tv.push_back(vs[3 * iq + 2]);
			tv.push_back(vs[3 * ir + 0]);
			tv.push_back(vs[3 * ir + 1]);
			tv.push_back(vs[3 * ir + 2]);

			vector<float>& tt = tris_t[current_mat];
			tt.push_back(vts[2 * jp + 0]);
			tt.push_back(vts[2 * jp + 1]);
			tt.push_back(vts[2 * jq + 0]);
			tt.push_back(vts[2 * jq + 1]);
			tt.push_back(vts[2 * jr + 0]);
			tt.push_back(vts[2 * jr + 1]);
		}
		else if (t == "g") {
			//current_mat.clear();
		}
		else if (t == "mtllib") {
			string fname;
			ss >> fname;
			matlib = parse_mtl(fname);
		}
		else if (t == "usemtl") {
			string matname;
			ss >> matname;
			current_mat = matname;
		}
		else {
			cerr << "Unknown tag: " << t << endl;
		}
		
	}

	for (auto i = tris_v.begin(); i != tris_v.end(); ++i) {
		const string& t = i->first;
		const Mat& m = matlib[t];
		int idx = idmap.size();
		idmap[t] = idx;
#define max(a,b) ((a)>(b))?(a):(b)
		tri_v.push_back(i->second);
		tri_t.push_back(tris_t[t]);
		textureFile.push_back(m.TextureFile);
		Rs.push_back(max(m.Ka[0], max(m.Ks[0], m.Kd[0])));
		Gs.push_back(max(m.Ka[1], max(m.Ks[1], m.Kd[1])));
		Bs.push_back(max(m.Ka[2], max(m.Ks[2], m.Kd[2])));
		/*cout << m.Ka[0] << ' ' << m.Ks[0] << ' ' << m.Kd[0] << endl;
		cout << m.Ka[1] << ' ' << m.Ks[1] << ' ' << m.Kd[1] << endl;
		cout << m.Ka[2] << ' ' << m.Ks[2] << ' ' << m.Kd[2] << endl;
		cout << endl;*/
	}
}

void ObjModel::init(void) {
	glGenVertexArrays(1, &va);
	glBindVertexArray(va);

	int n = tri_v.size();
	v_buffer = new GLuint[n];
	t_buffer = new GLuint[n];
	//s_buffer = new GLuint[n];

	glGenBuffers(n, v_buffer);
	glGenBuffers(n, t_buffer);
	//glGenBuffers(n, s_buffer);

	visibility.assign(n, 1.0);

	for (int i = 0; i < n; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, v_buffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tri_v[i].size(), &tri_v[i][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, t_buffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tri_t[i].size(), &tri_t[i][0], GL_STATIC_DRAW);

		//glBindBuffer(GL_ARRAY_BUFFER, s_buffer[i]);
		//vector<GLfloat> bools(tri_v[i].size(), 1.0);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * tri_v[i].size(), bools.data(), GL_STATIC_DRAW);
		//glBufferSubData
	}

	if (!objTex_shader) {
		const char* vs =
			"#version 330 compatibility\n"
			"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
			"layout(location = 1) in vec2 vertexPosition_texCoord;\n"
//			"layout(location = 2) in float face_visible;\n"
			"out vec2 TexCoord;\n"
			"out vec3 orgcoord;\n"
//			"out float visible;\n"
			"void main() {"
			"	gl_Position = gl_ModelViewProjectionMatrix * vec4(vertexPosition_modelspace, 1);"
			"	TexCoord = vertexPosition_texCoord;"
			"   orgcoord = vertexPosition_modelspace.xyz;"
//			"   visible = 1.0;"
			"}";

		objTex_shader = new Shader();
		objTex_shader->create(
			vs,
			"#version 330 compatibility\n"
			"out vec3 color;\n"
			"in vec2 TexCoord;\n"
			"in vec3 orgcoord;\n"
//			"in float visible;\n"
//			"uniform float visibleflag;\n"
			"uniform vec3 myC;\n"
			"uniform int zfilter;\n"
			"uniform float zmin;\n"
			"uniform float zmax;\n"
			"uniform sampler2D ourTexture;\n"
			"void main() {\n"
//			"   if(visibleflag<0.5) discard;\n"
			"if(zfilter==1&&orgcoord.z<zmin) discard;"
			"if(zfilter==1&&orgcoord.z>zmax) discard;"
			"color = texture(ourTexture, TexCoord).xyz;\n"
			"}\n"
		);
		objTex_shader_color = glGetUniformLocation(objTex_shader->program, "myC");
		objTex_zfilter = glGetUniformLocation(objTex_shader->program, "zfilter");
		objTex_zmin = glGetUniformLocation(objTex_shader->program, "zmin");
		objTex_zmax = glGetUniformLocation(objTex_shader->program, "zmax");
//		objTex_shader_visibility = glGetUniformLocation(objTex_shader->program, "visibleflag");

		objCol_shader = new Shader();
		objCol_shader->create(
			vs,
			"#version 330 compatibility\n"
			"out vec3 color;\n"
			"in vec2 TexCoord;\n"
			"in vec3 orgcoord;\n"
//			"in uint visible;\n"
//			"uniform float visibleflag;\n"
			"uniform vec3 myC;\n"
			"uniform int zfilter;\n"
			"uniform float zmin;\n"
			"uniform float zmax;\n"
			"uniform sampler2D ourTexture;\n"
			"void main() {\n"
//			"   if(visibleflag<0.5) discard;\n"
			"	if(zfilter==1&&orgcoord.z<zmin) discard;"
			"	if(zfilter==1&&orgcoord.z>zmax) discard;"
			"	color = myC;\n"
			"}\n"
		);

		objCol_shader_color = glGetUniformLocation(objCol_shader->program, "myC");
		objCol_zfilter = glGetUniformLocation(objCol_shader->program, "zfilter");
		objCol_zmin = glGetUniformLocation(objCol_shader->program, "zmin");
		objCol_zmax = glGetUniformLocation(objCol_shader->program, "zmax");
//		objCol_shader_visibility = glGetUniformLocation(objCol_shader->program, "visibleflag");
	}

	objTex_shader->use();

	textureID = new GLuint[n];
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	stbi_set_flip_vertically_on_load(true);
	for (int i = 0; i < n; ++i) {
		if (textureFile[i].empty()) {
			textureID[i] = -1;
			//cout << "empty texture: " << textureFile[i] << endl;
		}
		else {
			int w, h, c;
			unsigned char* data = stbi_load(textureFile[i].c_str(), &w, &h, &c, 0);
			/*if (!data) {
				cout << "cannot load texture: " << textureFile[i] << endl;
			}
			else {
				cout << "texture loaded: " << textureFile[i] << endl;
			}*/
			unsigned int t;
			glGenTextures(1, &t);
			glBindTexture(GL_TEXTURE_2D, t);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (c == 3) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else if (c == 4) {
				//cout << "C4-" << t << endl;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				std::cerr << "Unknown channel" << std::endl;
			}
			stbi_image_free(data);
			textureID[i] = t;
		}
	}

	glUseProgram(0);
}
void ObjModel::release(void) {
	delete[] textureID;
	delete[] v_buffer;
	delete[] t_buffer;
//	delete[] s_buffer;
}
void ObjModel::draw(DRAWING_MODE m, bool use_z_filter, float zmin, float zmax, float tx, float ty, float theta, bool show_non_texture) {
	glMatrixMode(GL_MODELVIEW_MATRIX);
	glPushMatrix();
	glTranslatef(tx, ty, 0);
	glRotatef(theta, 0, 0, 1);
	glScalef(0.001, 0.001, 0.001);
	for (int i = 0; i < tri_v.size(); ++i) {
		if (visibility[i] < 0.5) continue;
		float r, g, b;
		GLubyte v[4];

		switch(m) {
		case MODE_LINE:
			glUseProgram(objCol_shader->program);
			glUniform3f(objCol_shader_color, 0, 0, 0);
			glUniform1i(objCol_zfilter, use_z_filter ? 1 : 0);
			glUniform1f(objCol_zmin, zmin * 1000);
			glUniform1f(objCol_zmax, zmax * 1000);
			break;
		case MODE_TEXTURE:
			if (textureID[i] == -1) {
				if (!show_non_texture) continue;
				glUseProgram(objCol_shader->program);
				r = Rs[i];
				g = Gs[i];
				b = Bs[i];
				glUniform3f(objCol_shader_color, r, g, b);
				glUniform1i(objCol_zfilter, use_z_filter ? 1 : 0);
				glUniform1f(objCol_zmin, zmin * 1000);
				glUniform1f(objCol_zmax, zmax * 1000);
			}
			else {
				glUseProgram(objTex_shader->program);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureID[i]); 
				glUniform1i(objTex_zfilter, use_z_filter ? 1 : 0);
				glUniform1f(objTex_zmin, zmin * 1000);
				glUniform1f(objTex_zmax, zmax * 1000);
			}
			break;
		case MODE_GEOMETRY:
			break;
		case MODE_INDEX:
			glUseProgram(0);
			decompose_integer(i+1, v);
			glColor4ub(v[0], v[1], v[2], v[3]);
		default:;
		}
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, v_buffer[i]);
		glVertexAttribPointer(
			0,                  // 0번째 속성(attribute). 0 이 될 특별한 이유는 없지만, 쉐이더의 레이아웃(layout)와 반드시 맞추어야 합니다.
			3,                  // 크기(size)
			GL_FLOAT,           // 타입(type)
			GL_FALSE,           // 정규화(normalized)?
			0,                  // 다음 요소 까지 간격(stride)
			(void*)0            // 배열 버퍼의 오프셋(offset; 옮기는 값)
		);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, t_buffer[i]);
		glVertexAttribPointer(
			1,                  // 0번째 속성(attribute). 0 이 될 특별한 이유는 없지만, 쉐이더의 레이아웃(layout)와 반드시 맞추어야 합니다.
			2,                  // 크기(size)
			GL_FLOAT,           // 타입(type)
			GL_FALSE,           // 정규화(normalized)?
			0,                  // 다음 요소 까지 간격(stride)
			(void*)0            // 배열 버퍼의 오프셋(offset; 옮기는 값)
		);

		/*glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, s_buffer[i]);
		glVertexAttribPointer(
			2,                  // 0번째 속성(attribute). 0 이 될 특별한 이유는 없지만, 쉐이더의 레이아웃(layout)와 반드시 맞추어야 합니다.
			1,                  // 크기(size)
			GL_FLOAT,           // 타입(type)
			GL_FALSE,           // 정규화(normalized)?
			0,                  // 다음 요소 까지 간격(stride)
			(void*)0            // 배열 버퍼의 오프셋(offset; 옮기는 값)
		);*/

		switch(m){
		case MODE_LINE:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1);
			break;
		case MODE_TEXTURE:
		case MODE_GEOMETRY:
		case MODE_INDEX:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		default:;
		}
		glDrawArrays(GL_TRIANGLES, 0, tri_v[i].size() / 3);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);
	}
	glUseProgram(0);
	glPopMatrix();
}

#include <imgui.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Renderer.h"
#include "GeomObjects.h"
#include "Util.h"
#include "World.h"
#include "Controller.h"
#include "ControllerImpl.h"
#include "ControllerImpl_StateEditor.h"
#include "ControllerImpl_ConnectionEditor.h"
#include "ObjModel.h"


using namespace std;

static Camera cam;
static double last_t = 0.0;
static radian_t base_yaw = 0.0;
static radian_t base_pitch = 0.0;
static RenderBuffer render_buffer;


static bool use_z_filter = false;
static Shader z_filter;
static float z_filter_zmin = 0.0;
static float z_filter_zheight = 10.0;
void bind_z_filter_shader();
void unbind_z_filter_shader();

Point3D get_camera_pos(void) {
	return cam.center;
}

World w;
World* Controller::world = &w;

Controller* Controller::current_controller = 0;

unsigned int select_object(int w, int h, float x, float y) {
	GLubyte v[4];
	render_buffer.resize(w, h);
	render_buffer.writeon();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	cam.applyProjectionMatrix();
	cam.applyViewMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	bind_z_filter_shader();
	Controller::current_controller->draw_select_scene();
	unbind_z_filter_shader();

	render_buffer.writeend();

	render_buffer.read();
	glReadPixels(x, h-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, v);
	render_buffer.readend();

	unsigned int i = compose_integer(v);

	return i;
}

static void io(int w, int h) {
	double current_t = ImGui::GetTime();

	ImGuiIO& io = ImGui::GetIO();
	float mouse_cursor_x = io.MousePos.x;
	float mouse_cursor_y = io.MousePos.y;
	
	bool camera_updated = false;

	camera_updated = camera_updated || (cam.w != w) || (cam.h != h);
	cam.w = w;
	cam.h = h;

	if (1 || !io.WantCaptureKeyboard) {
		int cam_dx = 0, cam_dy = 0, cam_dz = 0;
		int cam_dangle = 0;
		if (io.KeysDown['W']) { cam_dx -= 1; }
		if (io.KeysDown['A']) { cam_dy -= 1; }
		if (io.KeysDown['S']) { cam_dx += 1; }
		if (io.KeysDown['D']) { cam_dy += 1; }
		if (io.KeysDown['T']) { cam_dz += 1; }
		if (io.KeysDown['G']) { cam_dz -= 1; }
		//if (ImGui::IsKeyPressed(GLFW_KEY_DELETE,false))  { cout << "DEL!" << endl; }
		Vector3D mov_vec(cam_dx, cam_dy, cam_dz);
		scalar_t speed = (current_t - last_t) * (io.KeyShift ? 15 : 3);
		cam.center = cam.center + cam.transform(mov_vec) * speed;
		camera_updated = camera_updated || cam_dx || cam_dy || cam_dz;
	}
	
	if (!io.WantCaptureMouse) {
		
		if (io.MouseClicked[1]) {
			base_yaw = cam.yaw;
			base_pitch = cam.pitch;
		}
		if (io.MouseDown[1]) {
			int dx = io.MousePos.x - io.MouseClickedPos[1].x;
			int dy = io.MousePos.y - io.MouseClickedPos[1].y;

			cam.yaw = base_yaw - dx / 100.0;
			cam.pitch = base_pitch - dy / 100.0;

			camera_updated = camera_updated || dx || dy;
		}

		unsigned int s_i = select_object(w, h, mouse_cursor_x, mouse_cursor_y);
		Point3D ray_o; Vector3D ray_d;
		cam.getRay(mouse_cursor_x, mouse_cursor_y, ray_o, ray_d);
		Line3D ray(ray_o, ray_d);

		if (io.MouseClicked[0]) {
			Controller::current_controller->on_mouse_down(mouse_cursor_x, mouse_cursor_y, ray, s_i);
		}
		else if (io.MouseReleased[0]) {
			Controller::current_controller->on_mouse_up(mouse_cursor_x, mouse_cursor_y, ray, s_i);
		}
		else if (io.MouseDown[0] ) {
			Controller::current_controller->on_mouse_drag(mouse_cursor_x, mouse_cursor_y, ray, s_i);
		}
		else if( 0 <= mouse_cursor_x && mouse_cursor_x <= w && 0 <= mouse_cursor_y && mouse_cursor_y <= h ) {
			Controller::current_controller->on_mouse_hover(mouse_cursor_x, mouse_cursor_y, ray, s_i);
		}
	}
	last_t = current_t;
}

static void draw(int w, int h) {
	glClearColor(0, 0, 0.5, 0);
	glViewport(0, 0, w, h);
	glColor3f(1, 1, 1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!use_z_filter) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	cam.applyProjectionMatrix();
	cam.applyViewMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	glEnable(GL_DEPTH_TEST);

	Controller::current_controller->draw_scene();
}

#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include "Texture.h"
#include "PointCloud.h"

vector<string> cubtexfnames;
vector<string> posefnames;
vector<Vector3D> posedelta;
vector<Point3D> cubtex_center;

Vector3D *cubtex_posedelta = 0;
TextureCubemap cubtex;
int CurrentCubTexIndex = 0;
bool cub_texturing = false;
bool cub_nearest_cam = false;

static PointCloud *pc = 0;
static bool __temp_showPC = false;
static int __temp_PCSize = 1;
static float __temp_PCAlpha = 0.1;
static float __temp_PCX = 0.0;
static float __temp_PCY = 0.0;
static float __temp_PCZ = 0.0;
static float __temp_PCSZ = 1.0;
static float __temp_PCTh = 0.0;
static string __temp_pc_filename;
static string __temp_cubtex_filename;

static Shader shader_wall_based_pointcloud_visualizaer;
static float shader_wall_based_pointcloud_visualizaer__Theta = 0.2;
static int shader_no = 0;

static ObjModel* OverlayObj;
static bool __show_overlay_obj = false;

static void init_shader(void) {
	shader_wall_based_pointcloud_visualizaer.create(0,
		"#version 120\n"
		"uniform sampler2D tex;\n"
		"uniform vec4 WindowSize;\n"
		"uniform vec2 FilterParam;\n"
		"void main() { "
		"float color = texture2D(tex,vec2(gl_FragCoord.x/WindowSize.x,gl_FragCoord.y/WindowSize.y)).r;"
		"float near = WindowSize.z;"
		"float far = WindowSize.w;"
		"float depth_plane = (2.0*near*far)/(far+near-color*(far-near));"
		"float depth_ptr = (2.0*near*far)/(far+near-gl_FragCoord.z*(far-near));"
		"if(abs(depth_ptr-depth_plane)>FilterParam.x) {"
		"discard;"
		"}"
		"float d=0.5+(depth_ptr-depth_plane)/(2*FilterParam.x);"
		"gl_FragColor=vec4(d,1-d,0,FilterParam.y);"
		"}");

	z_filter.create(
		"#version 120\n"
		"varying vec3 orgcoord;\n"
		"varying vec4 color;\n"
		"void main() { "
		"color=gl_Color;"
		"orgcoord = (gl_ModelViewMatrix*gl_Vertex).xyz;"
		"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }",

		"#version 120\n"
		"varying vec3 orgcoord;\n"
		"varying vec4 color;\n"
		"uniform float zmin;\n"
		"uniform float zmax;\n"
		"void main() {"
		"if(orgcoord.z<zmin) discard;"
		"if(orgcoord.z>zmax) discard;"
		"if(orgcoord.z<zmin+0.1) { gl_FragColor=vec4(0,0,0,0); }"
		"else if(orgcoord.z>zmax-0.1) { gl_FragColor=vec4(0,0,0,0); }"
		"else {"
		"	if(!gl_FrontFacing) discard;"
		"	else gl_FragColor = color; "
		"}}"
	);
}


static void bind_z_filter_shader(void) {
	if (use_z_filter) {
		z_filter.use();
		glDisable(GL_CULL_FACE);
		int zmin = glGetUniformLocation(z_filter.program, "zmin");
		glUniform1f(zmin, z_filter_zmin);
		int zmax = glGetUniformLocation(z_filter.program, "zmax");
		glUniform1f(zmax, z_filter_zmin+ z_filter_zheight);
	}
}
static void unbind_z_filter_shader(void) {
	glEnable(GL_CULL_FACE);
	glUseProgram(0);
}
static void bind_shader(void) {
	if (!pc) return;
	if (!__temp_showPC) return;
	if (shader_no == 0) return;

	if (shader_no == 1) {
		glDisable(GL_DEPTH_TEST);
		render_buffer.bind_depthbuffer();
		shader_wall_based_pointcloud_visualizaer.use();

		int uniform_WindowSize = glGetUniformLocation(shader_wall_based_pointcloud_visualizaer.program, "WindowSize");
		glUniform4f(uniform_WindowSize, render_buffer.width, render_buffer.height, cam.th_near, cam.th_far);
		int uniform_FilterParam = glGetUniformLocation(shader_wall_based_pointcloud_visualizaer.program, "FilterParam");
		glUniform2f(uniform_FilterParam, shader_wall_based_pointcloud_visualizaer__Theta, __temp_PCAlpha);
	}
}

static void unbind_shader(void) {
	glUseProgram(0);

	if (shader_no == 1) {
		glEnable(GL_DEPTH_TEST);
		render_buffer.unbind();	
	}
}

string getdir(const string& path) {
	size_t last_slash = path.find_last_of("/\\");
	if (last_slash == string::npos) return "./";
	return path.substr(0, last_slash + 1);
}

string get_filename(const string& path) {
	size_t last_slash = path.find_last_of("/\\");
	if (last_slash == string::npos) return path;
	return path.substr(last_slash+1);
}

void __temp_load_pcd_obj(const string& objectlistfile) {
	w.objects.clear();
	ifstream in(objectlistfile);
	string path = getdir(objectlistfile);
	string f;
	while (in >> f) {
		PCBox b;
		string fname = path + f;
		ifstream in_pcd(fname,ios::binary);
		b.read_from_pcd(in_pcd);
		b.tag = string(1,toupper(f[0]));
		w.objects.push_back(b);
	}
}

void __temp_load_pointcloud(const string& filename) {
	ifstream in(filename, ios::binary);
	pc = new PointCloud();
	if (!pc->read_from_pcd(in, 1)) {
		delete pc;
		pc = 0;
		return;
	}
}

void __temp_load_cubemap(int i) {
	cubtex.release();
	if (cubtexfnames.size() <= i) return;
	cubtex.load_jpg(cubtexfnames[i].c_str());

	GLfloat *r = cubtex.RplaneCoefficients;
	GLfloat *s = cubtex.SplaneCoefficients;
	GLfloat *t = cubtex.TplaneCoefficients;
	ifstream in(posefnames[i].c_str());
	in >> r[0] >> r[1] >> r[2] >> r[3];
	in >> s[0] >> s[1] >> s[2] >> s[3];
	in >> t[0] >> t[1] >> t[2] >> t[3];

	for (int i = 0; i < 4; ++i) { s[i] = -s[i]; }

	cubtex.postload();
	cubtex_posedelta = &posedelta[i];
}

void __temp_load_next_cubemap(void) {
	if (cubtexfnames.empty()) return;
	CurrentCubTexIndex = (CurrentCubTexIndex + 1) % cubtexfnames.size();
	__temp_load_cubemap(CurrentCubTexIndex);
}
void __temp_load_prev_cubemap(void) {
	if (cubtexfnames.empty()) return;
	CurrentCubTexIndex = (CurrentCubTexIndex + cubtexfnames.size() - 1) % cubtexfnames.size();
	__temp_load_cubemap(CurrentCubTexIndex);
}

void __temp_load_cubemap_texture(const string& filename) {
	cubtex.release();
	{
		cubtexfnames.clear();
		posefnames.clear();
		ifstream in(filename);

		string f;
		string path = getdir(filename);
		while (getline(in,f)) {
			if (f.empty()) continue;
			cubtexfnames.push_back(path + f);
			if (!getline(in, f)) continue;
			posefnames.push_back(path + f);
		}
	}

	for (int i = 0; i < posefnames.size(); ++i) {
		posedelta.push_back(Vector3D(0, 0, 0));

		ifstream in(posefnames[i]);
		GLfloat r[4];
		GLfloat s[4];
		GLfloat t[4];
		in >> r[0] >> r[1] >> r[2] >> r[3];
		in >> s[0] >> s[1] >> s[2] >> s[3];
		in >> t[0] >> t[1] >> t[2] >> t[3];
		for (int j = 0; j < 4; ++j) { s[j] = -s[j]; }

		GLfloat x = r[0] * r[3] + s[0] * s[3] + t[0] * t[3];
		GLfloat y = r[1] * r[3] + s[1] * s[3] + t[1] * t[3];
		GLfloat z = r[2] * r[3] + s[2] * s[3] + t[2] * t[3];
		//cout << x << ',' << y << ',' << z << endl;
		cubtex_center.push_back(Point3D(-x, -y, -z));
	}
}

void __temp_load_geometry(const string& filename) {

	delete Controller::current_controller;
	Controller::current_controller = new MainViewer(0);

	w.clear();

	ifstream in(filename);
	string line;
	map<int, Point3D> pts;
	vector<vector<int> > polys;
	vector<vector< vector<int> > > holes;
	while (getline(in, line)) {
		int n;
		int h;
		{
			istringstream ss(line);
			if (!(ss >> n)) break;
			if (!(ss >> h)) h = 0;
		}
		vector<int> poly;
		for (int i = 0; i < n; ++i) {
			getline(in, line);
			istringstream ss(line);
			double x, y, z;
			int idx;
			ss >> x >> y >> z >> idx;
			pts[idx] = Point3D(x, y, z);
			poly.push_back(idx);
		}
		polys.push_back(poly);
		vector<vector<int> > hole;
		for (int i = 0; i < h; ++i) {
			int m;
			vector<int> holelinestring;
			{
				getline(in, line);
				istringstream ss(line);
				ss >> m;
			}

			for (int j = 0; j < m; ++j) {
				getline(in, line);
				istringstream ss(line);
				double x, y, z;
				int idx;
				ss >> x >> y >> z >> idx;
				pts[idx] = Point3D(x, y, z);
				holelinestring.push_back(idx);
			}
			hole.push_back(holelinestring);
		}
		holes.push_back(hole);
	}

	map<int, Vertex*> vtx;
	for (auto i = pts.begin(); i != pts.end(); ++i) {
		Point3D p = i->second;
		Vertex *v = new Vertex(p.x, p.y, p.z);
		vtx[i->first] = v;
		w.vertices.push_back(v);
	}

	for (int i = 0; i < polys.size(); ++i) {
		// cerr << "POLY " << i << endl;
		vector<Vertex*> vts;
		for (int j = 0; j < polys[i].size(); ++j) {
			vts.push_back(vtx[polys[i][j]]);
		}
		Facet *f = Facet::create_facet(vts);

		for (int j = 0; j < holes[i].size(); ++j) {
			vector<Vertex*> hole_vts;
			for (int k = 0; k < holes[i][j].size(); ++k) {
				hole_vts.push_back(vtx[holes[i][j][k]]);
			}
			f->make_hole(hole_vts);
		}
		if (f) {
			f->triangulate();
			w.facets.push_back(f);
		}
		else {
			cerr << "FAIL " << i << endl;
		}
	}
}

void __temp_save_geometry(const string& filename) {
	ofstream out(filename);
	map<Vertex*, int> vmap;
	for (int i = 0; i < w.vertices.size(); ++i) {
		vmap[w.vertices[i]] = i;
	}
	for (int i = 0; i < w.facets.size(); ++i) {

		vector<Vertex*> exterior;
		{// exterior
			FacetEdge *e_start = w.facets[i]->get_exteior_edge();
			FacetEdge *e = e_start;
			do {
				exterior.push_back(e->get_u()->get_vertex());
				e = e->next();
			} while (e != e_start);
		}

		out << exterior.size() << ' ' << w.facets[i]->num_holes() << endl;

		for (int j = 0; j < exterior.size(); ++j) {
			Point3D x = exterior[j]->to_point();
			int id = vmap.at(exterior[j]);
			out << x.x << ' ' << x.y << ' ' << x.z << ' ' << id << endl;
		}

		for (int j = 0; j < w.facets[i]->num_holes(); ++j) {
			FacetEdge *e_start = w.facets[i]->get_hole_edge(j);
			vector<Vertex*> hole;
			FacetEdge *e = e_start;
			do {
				hole.push_back(e->get_u()->get_vertex());
				e = e->next();
			} while (e != e_start);

			out << hole.size() << endl;

			for (int k = 0; k < hole.size(); ++k) {
				Point3D x = hole[k]->to_point();
				int id = vmap.at(hole[k]);
				out << x.x << ' ' << x.y << ' ' << x.z << ' ' << id << endl;
			}
		}
	}
	out.close();
}

void __temp_save_connection_with_index(const string& fname) {
	ofstream out(fname);
	for (int i = 0; i < w.connections.size(); ++i) {
		int basefacet_i = -1;
		for (int j = 0; j < w.facets.size(); ++j) {
			if (w.connections[i].fbase == w.facets[j]) {
				basefacet_i = j;
				break;
			}
		}
		int oppositefacet_i = -1;
		for (int j = 0; j < w.facets.size(); ++j) {
			if (w.connections[i].fbase_opposite == w.facets[j]) {
				oppositefacet_i = j;
				break;
			}
		}

		out << basefacet_i << ' ' << oppositefacet_i << ' ' << w.connections[i].ring.size() << endl;
		for (int j = 0; j != w.connections[i].ring.size(); ++j) {
			out << w.connections[i].ring[j].x << ' ' << w.connections[i].ring[j].y << ' ' << w.connections[i].ring[j].z << ' ';
		}
		out << endl;
	}
	out.close();
}

void __temp_save_connection(const string& fname) {
	ofstream out(fname);
	for (int i = 0; i < w.connections.size(); ++i) {
		out << w.connections[i].ring.size();
		vector<Point3D> ring = w.connections[i].ring;
		if (w.connections[i].get_normal_of_ring().dot_product(w.connections[i].fbase->get_plane().h) < 0) {
			ring = vector<Point3D>(ring.rbegin(),ring.rend());
		}
		for (int j = 0; j != ring.size(); ++j) {
			out << ' ' << ring[j].x << ' ' << ring[j].y << ' ' << ring[j].z;
		}
		if (w.connections[i].fbase_opposite) {
			out << ' ' << ring.size();
			Plane Popp = w.connections[i].fbase_opposite->get_plane();
			for (int j = 0; j != ring.size(); ++j) {
				Point3D p_proj = Popp.project(ring[j]);
				out << ' ' <<  p_proj.x << ' ' << p_proj.y << ' ' << p_proj.z;
			}
		}
		else {
			out << ' ' << -1;
		}
		out << endl;
	}
	out.close();
}


template <class T>
static pair<bool,pair<T,T> > distance_bound_between_connection_and_facet(const Connection& c, Facet *f) {
	bool in = true;
	T m = 0, M = 0;
	Plane plane = f->get_plane();
	plane.h = plane.h.normalized();
	for (int i = 0; i < (int)c.ring.size(); ++i) {
		float alpha;
		interp(c.ring[i], plane, &alpha);
		T d = alpha;
		if (alpha < m || i == 0) m = alpha;
		if (M < alpha || i == 0) M = alpha;
		
		Point3D p_proj = c.ring[i] - alpha * plane.h;
		if (f->winding_number(p_proj) % 2 == 0) {
			bool close_v = false;
			for (int i = 0; i < f->num_vertices(); ++i) {
				if ((f->get_vertex(i)->to_point3().to_vector() - p_proj.to_vector()).is_zero()){
					close_v = true;
					break;
				}
			}
			for (int i = 0; i < f->num_edges(); ++i) {
				Point3D u = f->get_edge(i)->get_u()->to_point3();
				Point3D v = f->get_edge(i)->get_v()->to_point3();
				Line3D l(u, v);
				scalar_t a;
				interp(p_proj, l, &a);
				if (0 <= a && a <= 1 && (p_proj.to_vector() - (l.p + a * l.v).to_vector()).is_zero()) {
					close_v = true;
					break;
				}
			}
			if (!close_v) { in = false; }
		}
	}
	return make_pair( in, make_pair(m, M) );
}

void __temp_load_connection_with_index(const string& fname) {
	ifstream in(fname);
	int i, j;
	w.connections.clear();
	while (in >> i >> j) {
		int m;
		in >> m;
		Connection c;
		c.fbase = 0;
		c.fbase_opposite = 0;
		if (0 <= i && i < w.facets.size()) c.fbase = w.facets[i];
		if (0 <= j && j < w.facets.size()) c.fbase_opposite = w.facets[j];

		Plane plane = c.fbase->get_plane();
		//coord_t cx = 0, cy = 0, cz = 0;
		for (int k = 0; k < m; ++k) {
			coord_t x, y, z;
			if (in >> x >> y >> z) {
				//c.ring.push_back(Point3D(x, y, z));
				c.ring.push_back(plane.project(Point3D(x, y, z)));
				//cx += x; cy += y; cz += z;
			}
		}
		//cx /= m; cy /= m; cz /= m;

		w.connections.push_back(c);
	}
	in.close();
}

void __temp_load_connection(const string& fname) {
	ifstream in(fname);
	int m;
	w.connections.clear();
	while (in >> m) {
		Connection c;
		for (int k = 0; k < m; ++k) {
			coord_t x, y, z;
			if (in >> x >> y >> z) {
				c.ring.push_back(Point3D(x, y, z));
			}
		}
		{
			Facet *f_base = 0;
			scalar_t d_min;
			Vector3D normal = c.get_normal_of_ring().normalized();
			for (int i = 0; i < w.facets.size(); ++i) {
				if (abs(w.facets[i]->get_plane().h.normalized().dot_product(normal)) < 0.9) continue;
				pair<bool, pair<float, float> > ret = distance_bound_between_connection_and_facet<float>(c, w.facets[i]);
				if (ret.first == false) continue;
				float d = abs(ret.second.first) > abs(ret.second.second) ? abs(ret.second.first) : abs(ret.second.second);
				if (d > 0.05) continue;
				if (!f_base || d < d_min) {
					d_min = d;
					f_base = w.facets[i];
				}
			}
			c.fbase = f_base;

			if (!c.fbase) {
				cerr << "CANNOT FIND FACET" << endl;
			}
		}

		c.fbase_opposite = 0;
		in >> m;
		if (m > 0) {
			Connection c_opposite;
			for (int k = 0; k < m; ++k) {
				coord_t x, y, z;
				if (in >> x >> y >> z) {
					c_opposite.ring.push_back(Point3D(x, y, z));
				}
			}
			if (c.fbase) {
				Facet *f_opposite = 0;
				scalar_t d_min;
				Vector3D normal = c_opposite.get_normal_of_ring().normalized();
				for (int i = 0; i < w.facets.size(); ++i) {
					if (c.fbase->get_plane().h.dot_product(w.facets[i]->get_plane().h) > 0) continue;
					if (abs(w.facets[i]->get_plane().h.normalized().dot_product(normal)) < 0.9) continue;

					pair<bool, pair<float, float> > ret = distance_bound_between_connection_and_facet<float>(c_opposite, w.facets[i]);
					if (ret.first == false) continue;
					float d = abs(ret.second.first) > abs(ret.second.second) ? abs(ret.second.first) : abs(ret.second.second);
					if (d > 0.05) continue;
					if (!f_opposite || d < d_min) {
						d_min = d;
						f_opposite = w.facets[i];
					}
				}
				c.fbase_opposite = f_opposite;
			}


			if (!c.fbase_opposite) {
				cerr << "CANNOT FIND OPPOSITE FACET" << endl;
			}
		}

		if (c.fbase) w.connections.push_back(c);
	}
}

void init(void) {
	static std::string imgui_ini_path = get_executable_path() + string("/imgui.ini");
	ImGui::GetIO().IniFilename = imgui_ini_path.c_str();

	last_t = ImGui::GetTime();
	//__temp_load_cubemap_texture();
	//__temp_load_cubemap(0);
	//__temp_load_pointcloud();

	//__temp_load_geometry();
	//__temp_load_pcd_obj();
	init_shader();

	Controller::current_controller = new MainViewer(0);
}

#include "InFactoryQueryBuilder.h"
#include <curl/curl.h>

struct fetch_st {
	char* payload;
	size_t size;
};

size_t curl_callback(void *ptr, size_t size, size_t nmemb, void *our_ptr) {
	size_t addsize = size * nmemb;
	fetch_st *p = (fetch_st*) our_ptr;

	p->payload = (char*)realloc(p->payload, p->size + addsize + 1);

	memcpy(p->payload + p->size, ptr, addsize);
	p->size += addsize;
	p->payload[p->size] = 0;

	return addsize;
}

void __temp_export_to_indoorGML(void) {
	string fname = open_file_browser("", true, "gml");
	if (!fname.empty()) {
		curl_global_init(CURL_GLOBAL_ALL);
		CURL* curl = curl_easy_init();

		curl_slist* post_headers = NULL;
		post_headers = curl_slist_append(post_headers, "Accept: application/json");
		post_headers = curl_slist_append(post_headers, "Content-Type: application/json");

		fetch_st fetcher;
		fetcher.payload = (char*) malloc(1);
		fetcher.size = 0;

		InFactoryQueryBuilder builder;
		builder.add_world(w);
		builder.build(false);
		for (int i = 0; i < builder.query.size(); ++i) {
			builder.query[i].url;
			builder.query[i].data;

			fetcher.size = 0;
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_URL, builder.query[i].url.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, builder.query[i].data.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, post_headers);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)& fetcher);

			CURLcode ret = curl_easy_perform(curl);
			cout << "Querying: " << i << ", RET=" << ret << endl;
			if (ret != CURLE_OK) {
				message_box("통신 실패: InFactory가 정상적으로 실행 중인지 확인해주세요");
				return;
			}
		}

		cout << "Query Sent" << endl;

		long delay = 1;
		while(1) {
			cout << "Requesting IndoorGML..." << endl;
			fetcher.size = 0;
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_URL, builder.query[0].url.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, 0);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, 0);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)& fetcher);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, delay);

			//cout << "READY" << endl;
			CURLcode ret = curl_easy_perform(curl);
			if (ret != CURLE_OK) {
				if( delay < 60 ) delay *= 2;
				continue;
			}
			cout << "Complete: " << ret << ", size=" << fetcher.size << endl;
			ofstream out(fname);
			out << fetcher.payload << endl;
			out.close();
			message_box("IndoorGML 생성 완료");
			break;
		}

		free(fetcher.payload);

		curl_slist_free_all(post_headers);
		curl_easy_cleanup(curl);
		curl_global_cleanup();
	}
}

void __temp_export_to_inFactory_query(void) {
	string fname = open_file_browser("", true, "txt");
	if (!fname.empty()) {
		InFactoryQueryBuilder builder;
		builder.add_world(w);
		builder.build(false);
		ofstream out(fname);
		for (int i = 0; i < builder.query.size(); ++i) {
			out << builder.query[i].url << endl;
			out << builder.query[i].data << endl;
		}
		out.close();
	}
}

void make_ui(void) {
	ImGui::Begin("Info");
	ImGui::Text("Camera Pos: %.3f %.3f %.3f", (float)cam.center.x, (float)cam.center.y, (float)cam.center.z); 

	if (Controller::current_controller) {
		int state = Controller::current_controller->get_current_stage();
		if (state == 0) {

			if (ImGui::Button("Go to Connection Editor", ImVec2(350, 50))) {
				Controller::current_controller = new ConnectionBasicViewer(0);
			}
			ImGui::Text("");
			if (ImGui::Button("Load Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", false, "geom");
				if (!fname.empty()) {
					__temp_load_geometry(fname);
				}
			}
			if (ImGui::Button("Save Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", true, "geom");
				if (!fname.empty()) {
					__temp_save_geometry(fname);
				}
			}
			/*if (ImGui::Button("Load PCD File List for Obstacles (Loaded as MBBs)")) {
				string fname = open_file_browser("", false, "txt");
				if (!fname.empty()) {
					__temp_load_pcd_obj(fname);
				}
			}*/
		}
		else if (state == 1) {
			if (ImGui::Button("Go to State Editor", ImVec2(350, 50))) {
				w.make_cellspaces();
				Controller::current_controller = new StateMainController(0);
			}
			ImGui::Text("");
			if (ImGui::Button("Load Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", false, "geom");
				if (!fname.empty()) {
					w.clear();
					__temp_load_geometry(fname);
					Controller::current_controller = new MainViewer(0);
				}
			}
			if (ImGui::Button("Save Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", true, "geom");
				if (!fname.empty()) {
					__temp_save_geometry(fname);
				}
			}
			ImGui::Text("");
			if (ImGui::Button("Load Connections", ImVec2(250, 0))) {
				string fname = open_file_browser("", true, "conn");
				if (!fname.empty()) {
					__temp_load_connection(fname);
				}
			}
			if (ImGui::Button("Save Connections", ImVec2(250, 0))) {
				string fname = open_file_browser("", true, "conn");
				if (!fname.empty()) {
					__temp_save_connection(fname);
				}
			}
		}
		else if (state == 2) {
			if (ImGui::Button("Export to IndoorGML (InFactory)", ImVec2(350, 50))) {
				__temp_export_to_indoorGML();
			}
			if (ImGui::Button("Export InFactory Query", ImVec2(350, 50))) {
				__temp_export_to_inFactory_query();
			}
			ImGui::Text("");
			if (ImGui::Button("Load Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", false, "geom");
				if (!fname.empty()) {
					w.clear();
					__temp_load_geometry(fname);
					Controller::current_controller = new MainViewer(0);
				}
			}
			if (ImGui::Button("Save Geometry", ImVec2(250, 0))) {
				string fname = open_file_browser("", true, "geom");
				if (!fname.empty()) {
					__temp_save_geometry(fname);
				}
			}
		}
		else {
			ImGui::Text("Undefined state: %d", Controller::current_controller->get_current_stage());
		}
	}
	ImGui::End();

	ImGui::Begin("Overlay & Filter");
	if (ImGui::TreeNode("Z-Filter")) {
		ImGui::Checkbox("Use Z Filter", &use_z_filter);
		ImGui::DragFloat("Base", &z_filter_zmin, 0.1);
		ImGui::DragFloat("Height", &z_filter_zheight, 0.1, 0.1);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Point Cloud")) {
		if (ImGui::Button("PCD File")) {
			// load pcd file
			string fname = open_file_browser("", false, "pcd");
			if (!fname.empty()) {
				__temp_pc_filename = get_filename(fname);
				__temp_load_pointcloud(fname);
			}
		}
		ImGui::SameLine();
		ImGui::Text("%s", __temp_pc_filename.c_str());
		ImGui::Checkbox("Show Point Cloud", &__temp_showPC);
		//ImGui::SliderFloat(
		ImGui::SliderInt("Point Cloud Size", &__temp_PCSize, 1, 10);
		ImGui::SliderFloat("Point Cloud Alpha", &__temp_PCAlpha, 0, 1);
		ImGui::Text("Transform");
		ImGui::DragFloat("X", &__temp_PCX, 0.1);
		ImGui::DragFloat("Y", &__temp_PCY, 0.1);
		ImGui::DragFloat("Z", &__temp_PCZ, 0.1);
		ImGui::DragFloat("Th", &__temp_PCTh, 0.1);
		ImGui::DragFloat("Scale Z", &__temp_PCSZ, 0.01);
		ImGui::Text("");
		ImGui::DragInt("Shader#", &shader_no, 1, 0, 1);
		if (shader_no == 1) {
			ImGui::DragFloat("Threshold", &shader_wall_based_pointcloud_visualizaer__Theta, 0.05, 0);
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Texture")) {
		if (ImGui::Button("Load Texture")) {
			// load cubtex file
			string fname = open_file_browser("", false, "texinfo");
			if (!fname.empty()) {
				__temp_cubtex_filename = get_filename(fname);
				__temp_load_cubemap_texture(fname);
				__temp_load_cubemap(0);
				cub_texturing = true;
				cub_nearest_cam = true;
			}
		}
		ImGui::SameLine();
		ImGui::Text("%s", __temp_cubtex_filename.c_str());

		ImGui::Checkbox("Show Texture", &cub_texturing);
		ImGui::Checkbox("Auto Nearest Cubemap", &cub_nearest_cam);
		if (ImGui::Button("  < Prev  ")) {
			__temp_load_prev_cubemap();
		}
		ImGui::SameLine();
		if (ImGui::Button("  Next >  ")) {
			__temp_load_next_cubemap();
		}

		ImGui::Text("Position Adjustment");
		if (cubtex_posedelta) {
			if (sizeof(cubtex_posedelta->x) == 4) {
				ImGui::SliderFloat("X", (float*)&cubtex_posedelta->x, -5.0, 5.0);
				ImGui::SliderFloat("Y", (float*)&cubtex_posedelta->y, -10.0, 10.0);
				ImGui::SliderFloat("Z", (float*)&cubtex_posedelta->z, -5.0, 5.0);
			}
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode(".Obj Model")) {
		if (ImGui::Button("Load Obj Model")) {
			string fname = open_file_browser("", false, "obj");
			if (!fname.empty()) {
				if (OverlayObj) {
					OverlayObj->release();
					delete OverlayObj;
				}
				OverlayObj = new ObjModel();
				ifstream in(fname);
				OverlayObj->parse(in);
				OverlayObj->init();
				__show_overlay_obj = true;
			}
		}
		ImGui::SameLine();

		ImGui::Checkbox("Show Obj Model", &__show_overlay_obj);
		
		ImGui::TreePop();
	}
	ImGui::End();

	Controller::current_controller->make_ui();
}

void process_frame(int w, int h) {
	io(w, h);
	//cubtex.bind();
	if (cub_texturing) {
		if (cub_nearest_cam) {
			int min_j = CurrentCubTexIndex;
			length_t min_d = (cubtex_center[min_j] - cam.center).length();
			for (int j = 0; j < cubtex_center.size(); ++j) {
				length_t d = (cubtex_center[j] - cam.center).length();
				if (d < min_d) {
					min_d = d;
					min_j = j;
				}
			}
			if (min_j != CurrentCubTexIndex) {
				CurrentCubTexIndex = min_j;
				__temp_load_cubemap(min_j);
			}
		}
		if (cubtex_posedelta) { cubtex.bind(cubtex_posedelta->x, cubtex_posedelta->y, cubtex_posedelta->z); }
		else { cubtex.bind(); }
	}
	
	bind_z_filter_shader();
	draw(w, h);
	unbind_z_filter_shader();

	if (cub_texturing) {
		cubtex.unbind();
	}
	if ( __temp_showPC && pc) {
		glPushAttrib(GL_ENABLE_BIT);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glPushAttrib(GL_DEPTH_BITS | GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0, 0, 0, 0.5);
		glBegin(GL_QUADS);
		glVertex3f(-1, -1, 0.5);
		glVertex3f(1, -1, 0.5);
		glVertex3f(1, 1, 0.5);
		glVertex3f(-1, 1, 0.5);
		glEnd();
		glPopAttrib();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();


		glPointSize(__temp_PCSize);
		glDepthMask(GL_FALSE);
		glColor4f(1, 1, 1, __temp_PCAlpha);

		bind_shader();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glPushMatrix();
		glTranslatef(__temp_PCX, __temp_PCY, __temp_PCZ);
		glRotatef(__temp_PCTh, 0, 0, 1);
		glScalef(1, 1, __temp_PCSZ);
		pc->draw();

		unbind_shader();

		glPopMatrix();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	if (__show_overlay_obj && OverlayObj) {

		glDepthMask(GL_FALSE);

		glEnable(GL_BLEND);
		glBlendColor(0, 0, 0, 0.5);
		glBlendFunc(GL_CONSTANT_ALPHA,GL_ONE_MINUS_CONSTANT_ALPHA);
		OverlayObj->draw(false);
		glDisable(GL_BLEND);

		OverlayObj->draw(true);
		glDepthMask(GL_TRUE);
	}
	/*
	{
		GLfloat *r = cubtex.RplaneCoefficients;
		GLfloat *s = cubtex.SplaneCoefficients;
		GLfloat *t = cubtex.TplaneCoefficients;

		GLfloat x = r[0] * r[3] + s[0] * s[3] + t[0] * t[3];
		GLfloat y = r[1] * r[3] + s[1] * s[3] + t[1] * t[3];
		GLfloat z = r[2] * r[3] + s[2] * s[3] + t[2] * t[3];

		glDisable(GL_DEPTH_TEST);
		glPointSize(64);
		glBegin(GL_POINTS);
		glColor3f(1, 0, 1);
		glVertex3f(-x, -y, -z);
		glEnd();
	}*/
}

void postdraw(void) {
	Controller::current_controller->post_draw();
}
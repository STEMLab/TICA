#include <iostream>
#include <map>
#include <vector>
#include <set>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "GeoUtils.h"
#include "BasicViewer.h"
#include "Plane.h"
#include "Texture.h"
#include "TriangleMesh.h"
#include "PlanarFacet.h"
#include "Button.h"
#include "BinTexture.h"
#include "PointCloud.h"
#include "Shader.h"

#include <vector>
#include <algorithm>
#include <set>

using namespace std;

std::string open_file_browser(const std::string & path, bool save, const std::string& ext);

namespace app_editor {

	struct Annotation {
		enum ANNOTATION_TYPE {
			ANNOTATION_ANY,
		} type;
		vector<Point3D> boundary;

		void draw(void) const {
			glBegin(GL_POINTS);
			for (int i = 0; i < (int)boundary.size(); ++i) {
				glVertex3f(boundary[i].x, boundary[i].y, boundary[i].z);
			}
			glEnd();

			glBegin(GL_LINES);
			for (int i = 0; i < (int)boundary.size(); ++i) {
				int j = (i + 1) % (int)boundary.size();
				glVertex3f(boundary[i].x, boundary[i].y, boundary[i].z);
				glVertex3f(boundary[j].x, boundary[j].y, boundary[j].z);
			}
			glEnd();
		}

		int hit_test(const Ray3D& ray, length_t th = 0.1) {
			int min_i = -1;
			length_t min_d;
			for (int i = 0; i < (int)boundary.size(); ++i) {
				length_t d;
				if (!intersect(ray, boundary[i], th, 0, &d)) continue;
				if (min_i == -1 || d < min_d) {
					min_d = d;
					min_i = i;
				}
			}
			if (min_i >= 0) return min_i;
			length_t alpha;
			if (boundary.size() > 1) {
				for (int i = 0; i < (int)boundary.size(); ++i) {
					length_t b;
					length_t d;
					Point3D p = boundary[(i + boundary.size() - 1) % boundary.size()];
					Point3D q = boundary[i];
					Line3D l(p, q - p);
					if (!intersect(ray, l, th, &alpha, &b, &d)) continue;
					if (b <= EPSILON || 1 - EPSILON <= b) continue;
					if (min_i == -1 || d < min_d) {
						min_d = d;
						min_i = i;
					}
				}
			}
			if (min_i == -1) return -1;

			boundary.resize(boundary.size() + 1);
			for (int i = (int)boundary.size() - 1; i > min_i; --i) {
				boundary[i] = boundary[i - 1];
			}
			boundary[min_i] = ray.p + alpha * ray.v;

			return min_i;
		}
	};

	struct LoadedData {
		vector<PlanarFacet*> polygons;
		map<PlanarFacet*,BinTexture*> textures;
		map<PlanarFacet*, vector<Annotation> > annotations;
	};

	LoadedData load_data(void) {
		LoadedData data;

		string fname = open_file_browser(".", false, "rsd");
		if (fname.empty()) return data;
		ifstream in(fname.c_str());
		if (!in) return data;

		string token;
		in >> token;
		if (token != "RSD0" && token != "RSD1" && token != "RSD2") return data;
		int version = token[4] - '0';


		vector<pair< pair<int, int>, pair<int, int> > > opposites;
		PlanarFacet *poly = 0;
		while (in >> token) {
			if (token[0] == 'p') {

				Plane plane;
				in >> plane.p.x >> plane.p.y >> plane.p.z;
				in >> plane.x.x >> plane.x.y >> plane.x.z;
				in >> plane.y.x >> plane.y.y >> plane.y.z;

				poly = new PlanarFacet(plane);

				int m;
				in >> m;

				for (int j = 0; j < m; ++j) {
					Point2D p;
					in >> p.x >> p.y;
					poly->create_vertex(p);
				}

				in >> m;
				for (int j = 0; j < m; ++j) {
					int u, v;
					in >> u >> v;
					poly->create_edge(poly->vertices[u], poly->vertices[v]);
				}
				poly->exterior = poly->edges[0];
				data.polygons.push_back(poly);
			}
			else if (token[0] == 's' && version == 2) {
				int m;
				in >> m;
				Annotation ann;
				for (int j = 0; j < m; ++j) {
					Point2D v;
					in >> v.x >> v.y;
					if (poly) {
						ann.boundary.push_back(poly->plane.convert(v));
					}
				}
				if (poly) {
					data.annotations[poly].push_back(ann);
				}
			}
			else if (token[0] == 'i') {
				string fname;
				double scalefactor;
				double xmax, xmin, ymax, ymin;
				in >> fname >> scalefactor >> xmax >> xmin >> ymax >> ymin;
				string fpath = "resources/" + fname + ".bin";
				ifstream in_texture(fpath, ios::binary);
				if (in_texture) {

					BinTexture *t = new BinTexture();
					t->load(in_texture);
					t->set_scaler(scalefactor, xmin, ymin);
					t->xmax = xmax;
					t->ymax = ymax;
					t->ID = fname;
					in_texture.close();

					if (poly) data.textures[poly] = t;
				}
			}
			else if (token[0] == 'o') {
				int i, j, k, l;
				in >> i >> j >> k >> l;
				opposites.push_back(make_pair(make_pair(i, j), make_pair(k, l)));
			}
		}
		for (int i = 0; i < (int)opposites.size(); ++i) {
			pair<int, int> e1 = opposites[i].first;
			pair<int, int> e2 = opposites[i].second;
			data.polygons[e1.first]->edges[e1.second]->set_opposite_connection(data.polygons[e2.first]->edges[e2.second]);
		}
		return data;
	}

	struct Selection {
		set<PlanarFacet*> facets;
		set<PlanarFacetEdge*> edges;
		set<PlanarFacetVertex*> vertices;

		bool empty(void) const {
			return facets.empty() && edges.empty() && vertices.empty();
		}
		void clear(void) {
			facets.clear();
			edges.clear();
			vertices.clear();
		}

		void select_facet(PlanarFacet* f) {	clear(); facets.insert(f); }
		void select_edge(PlanarFacetEdge* e) { clear(); edges.insert(e); }
		void select_vertex(PlanarFacetVertex* v) { clear(); vertices.insert(v); }

		bool add_facet(PlanarFacet* f) {
			if (empty() || !facets.empty()) {
				if (is_selected(f)) {
					facets.erase(f);
					return false;
				}
				facets.insert(f); return true;
			}
			return false;
		}
		bool add_edge(PlanarFacetEdge* e) {
			if (empty() || !edges.empty()) {
				if (is_selected(e)) {
					edges.erase(e);
					return false;
				}
				edges.insert(e);
				return true;
			}
			return false;
		}
		bool add_vertex(PlanarFacetVertex* v) {
			if (empty() || !vertices.empty()) {
				if (is_selected(v)) {
					vertices.erase(v);
					return false;
				}
				vertices.insert(v);
				return true;
			}
			return false;
		}

		bool is_selected(PlanarFacet* f) { 	return facets.count(f) != 0; }
		bool is_selected(PlanarFacetEdge* e) { return edges.count(e) != 0; }
		bool is_selected(PlanarFacetVertex* v) { return vertices.count(v) != 0; }
	};

	void btn_callback_open_rsd(void);
	void btn_callback_save_rsd(void);
	void btn_callback_import_pc(void);
	void btn_callback_save_indoorGML(void);
	void btn_callback_action(void);
	void btn_callback_auto_connect(void);
	void btn_callback_mode(void);

	struct EditorViewer : public BasicViewer {
		vector<PlanarFacet*> facets;
		map<PlanarFacet*,BinTexture*> texture_map;

		Selection selections;
		map<PlanarFacet*,vector<Annotation> > annotations;

		int mode;
		Point3D *current_selected_point;
		Point3D *current_reference_point[2];
		PlanarFacet *current_selected_facet;

		Shader pointcloud_shader;
		PointCloud *pointcloud;

		void init_point_cloud_configuration(void) {
			pointcloud_shader.create(
				"#version 120\n"
				"varying vec4 color; "
				"varying float d; "
				"void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; color=gl_Color; d=gl_Position.z/gl_Position.w; }",

				"#version 120\n"
				"varying vec4 color;"
				"varying float d; "
				"void main() { gl_FragColor=vec4(color.xyz,1*(1-d*d)); }"
			);
		}

		vector<Button> buttons;

		EditorViewer() {
			mode = 0;
			current_selected_point = 0;
			current_reference_point[0] = 0;
			current_reference_point[1] = 0;
			current_selected_facet = 0;
			
			{
				Button btn;
				btn.x = 0;
				btn.y = 0;
				btn.w = 100;
				btn.h = 30;
				btn.set_font(GLUT_BITMAP_8_BY_13);
				btn.set_caption("Open .rsd");
				btn.callback_onclick = btn_callback_open_rsd;
				buttons.push_back(btn);

				btn.x = 100;
				btn.set_caption("Import PC");
				btn.callback_onclick = btn_callback_import_pc;
				buttons.push_back(btn);

				btn.x = 200;
				btn.set_caption("Save .rsd");
				btn.callback_onclick = btn_callback_save_rsd;
				buttons.push_back(btn);


				btn.x = 300;
				btn.set_caption("Export");
				btn.callback_onclick = btn_callback_save_indoorGML;
				buttons.push_back(btn);

				btn.x = 0;
				btn.y = camera.h - btn.h;

				btn.callback_onclick = btn_callback_action;
				btn.set_caption("Action");
				buttons.push_back(btn);

				btn.x = 100;
				btn.callback_onclick = btn_callback_auto_connect;
				btn.set_caption("Auto Connect");
				buttons.push_back(btn);

				btn.x = 200;
				btn.callback_onclick = btn_callback_mode;
				btn.set_caption("Mode: Annot");
				buttons.push_back(btn);
			}

			init_point_cloud_configuration();
			pointcloud = 0;

			load();
		}

		void load(void) {
			clear();
			LoadedData data = load_data();
			facets = data.polygons;
			texture_map = data.textures;
		}

		void clear(void) {
			for (int i = 0; i < facets.size(); ++i) {
				delete facets[i];
			}
			set<BinTexture*> textures;
			for (auto t = texture_map.begin(); t != texture_map.end(); ++t) {
				textures.insert(t->second);
			}
			for (auto t = textures.begin(); t != textures.end(); ++t) {
				delete *t;
			}
			texture_map.clear();
			textures.clear();

			selections.clear();
			annotations.clear();

			mode = 0;
			current_selected_point = 0;
			current_reference_point[0] = 0;
			current_reference_point[1] = 0;
			current_selected_facet = 0;

			if (pointcloud) {
				delete pointcloud;
				pointcloud = 0;
			}
		}

		void find_edge_connection_by_spatial_distance(void) {
			for (int i = 0; i < (int)facets.size(); ++i) {
				for (int ii = 0; ii < (int)facets[i]->edges.size(); ++ii) {
					if (facets[i]->edges[ii]->opposite) continue;

					vector<PlanarFacetEdge*> candidate;

					for (int j = i + 1; j < (int)facets.size(); ++j) {
						for (int jj = 0; jj < (int)facets[j]->edges.size(); ++jj) {
							if (facets[j]->edges[jj]->opposite) continue;

							if (facets[i]->edges[ii]->u->to_point3d() == facets[j]->edges[jj]->v->to_point3d()
								&& facets[i]->edges[ii]->v->to_point3d() == facets[j]->edges[jj]->u->to_point3d()) {
								candidate.push_back(facets[j]->edges[jj]);
							}
						}
					}

					if (candidate.size() == 1) {
						facets[i]->edges[ii]->set_opposite_connection(candidate[0]);
					}
				}
			}
		}

		void find_cell_facets(PlanarFacet *start, set<PlanarFacet*>& s) {
			s.insert(start);
			for (int i = 0; i < (int)start->edges.size(); ++i) {
				if (start->edges[i]->opposite) {
					PlanarFacet *f_opposite = start->edges[i]->opposite->base;
					if (!s.count(f_opposite)) {
						find_cell_facets(f_opposite,s);
					}
				}
			}
		}

		void save_in_indoorgml_intermediate_data(ostream& out) {
			out.setf(std::ios::fixed, std::ios::floatfield);
			out.precision(3);

			set<PlanarFacet*> visited;
			while (1) {
				//out << "=============================================" << endl;
				set<PlanarFacet*> s;
				int i = -1;
				for (i = 0; i < facets.size(); ++i) {
					if (!visited.count(facets[i])) {
						find_cell_facets(facets[i], s);
						break;
					}
				}
				if (i == facets.size()) break;

				int num_annotations = 0;
				out << s.size() << endl;
				for (auto j = s.begin(); j != s.end(); ++j) {
					PlanarFacet *f = *j;
					visited.insert(f);

					out << f->edges.size() << endl;
					{
						PlanarFacetEdge *e = f->exterior;
						do {
							Point3D p3d = e->u->to_point3d();
							out << p3d.x << ' ' << p3d.y << ' ' << p3d.z << endl;
							e = e->next();
						} while (e != f->exterior);
					}

					if (annotations.count(f)) {
						num_annotations += (int)annotations.at(f).size();
					}
				}

				out << num_annotations << endl;
				for (auto j = s.begin(); j != s.end(); ++j) {
					PlanarFacet *f = *j;
					if (!annotations.count(f)) continue;
					vector<Annotation>& ann = annotations.at(f);
					for (int k = 0; k < ann.size(); ++k) {
						out << ann[k].boundary.size() << endl;
						Vector3D normal_sum(0, 0, 0);
						int ll = (int)ann[k].boundary.size();
						for (int l = 0; l < ll; ++l) {
							Vector3D v01 = ann[k].boundary[(l + 1) % ll] - ann[k].boundary[l];
							Vector3D v12 = ann[k].boundary[(l + 2) % ll] - ann[k].boundary[(l+1)%ll];
							normal_sum = normal_sum + v01.cross_product(v12);
						}

						if (normal_sum.dot_product(f->plane.x.cross_product(f->plane.y)) > 0) {
							for (int l = 0; l < ll; ++l) {
								Point3D p = ann[k].boundary[l];
								out << p.x << ' ' << p.y << ' ' << p.z << endl;
							}
						}
						else {
							for (int l = ll-1; l >= 0; --l) {
								Point3D p = ann[k].boundary[l];
								out << p.x << ' ' << p.y << ' ' << p.z << endl;
							}
						}
					}
				}
			}
		}
		
		void save_in_rsd(ostream& out) {
			out.setf(std::ios::fixed, std::ios::floatfield);
			out.precision(8);
			out << "RSD2" << endl;
			map<PlanarFacet*, int> fmap;
			map<PlanarFacetEdge*, int> emap;

			for (int i = 0; i < (int)facets.size(); ++i) {
				fmap[facets[i]] = i;

				out << "p" << endl;
				out << facets[i]->plane.p.x << ' ' << facets[i]->plane.p.y << ' ' << facets[i]->plane.p.z << endl;
				out << facets[i]->plane.x.x << ' ' << facets[i]->plane.x.y << ' ' << facets[i]->plane.x.z << endl;
				out << facets[i]->plane.y.x << ' ' << facets[i]->plane.y.y << ' ' << facets[i]->plane.y.z << endl;

				int m;
				m = (int)facets[i]->vertices.size();
				out << m << endl;
				map<PlanarFacetVertex*, int> vmap;
				for (int j = 0; j < m; ++j) {
					Point2D v = facets[i]->plane.convert(facets[i]->vertices[j]->to_point3d());
					out << v.x << ' ' << v.y << ' ';
					vmap[facets[i]->vertices[j]] = j;
				}
				out << endl;

				m = (int)facets[i]->edges.size();
				out << m << endl;
				for (int j = 0; j < m; ++j) {
					emap[facets[i]->edges[j]] = j;
					out << vmap.at(facets[i]->edges[j]->u) << ' ' << vmap.at(facets[i]->edges[j]->v) << ' ';
				}
				out << endl;

				if (texture_map.count(facets[i])) {
					BinTexture *texture = texture_map.at(facets[i]);
					out << "i " << texture->ID << ' ' << texture->a << ' ' << texture->xmax << ' ' << texture->x0 << ' ' << texture->ymax << ' ' << texture->y0 << endl;
				}

				if (annotations.count(facets[i])) {
					const vector<Annotation>& anns = annotations.at(facets[i]);
					for (int j = 0; j < (int)anns.size(); ++j) {
						if (anns[j].boundary.size() >= 2) {
							out << "s " << anns[j].boundary.size();
							for (int k = 0; k < (int)anns[j].boundary.size(); ++k) {
								Point2D v = facets[i]->plane.convert(anns[j].boundary[k]);
								out << ' ' << v.x << ' ' << v.y;
							}
							out << endl;
						}
					}
				}
			}

			for (int i = 0; i < (int)facets.size(); ++i) {
				for (int j = 0; j < (int)facets[i]->edges.size(); ++j) {
					if (facets[i]->edges[j]->opposite) {
						int fi = fmap.at(facets[i]->edges[j]->opposite->base);
						int ei = emap.at(facets[i]->edges[j]->opposite);

						if (i < fi) {
							out << "o " << i << ' ' << j << ' ' << fi << ' ' << ei << endl;
						}
					}

				}
			}
		}

		void onDraw(void) {
			BasicViewer::onDraw();
			glClearColor(0.2, 0.2, 0.2, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			draw_world();
			draw_interface();
		}

		void draw_world(void) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_CCW);
			glDepthMask(GL_TRUE);
			glPolygonOffset(1, 1);
			glLineWidth(1);
			glPointSize(5);

			draw_world_elements();

			if (pointcloud) {
				glPointSize(1);
				pointcloud_shader.use();
				glDepthMask(GL_FALSE);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor3f(0, 1, 0);
				pointcloud->draw();
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
				glUseProgram(0);
			}

			glDisable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glLineWidth(2);
			glPointSize(5);
			glColor3f(1, 1, 1);

			draw_world_annotations();
		}

		void draw_interface(void) {
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0, camera.w, camera.h, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			glLineWidth(1);
			glPointSize(5);

			draw_interface_elements();

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		void draw_world_elements(void) {
			glPolygonMode(GL_FRONT, GL_FILL);
			for (int i = 0; i < (int)facets.size(); ++i) {
				BinTexture *texture = 0;
				if (texture_map.count(facets[i])){
					glEnable(GL_TEXTURE_2D);

					if (selections.is_selected(facets[i])) {
						glColor3f(0.8, 0.8, 1);
					}
					else {
						glColor3f(1, 1, 1);
					}
					
					texture = texture_map.at(facets[i]);
					texture->set();
				}
				else {
					glDisable(GL_TEXTURE_2D);
					if (selections.is_selected(facets[i])) {
						glColor3f(0.6, 0.6, 1);
					}
					else {
						glColor3f(0.8, 0.8, 1);
					}
				}

				bool selected = false;
				for (auto e = selections.edges.begin(); e != selections.edges.end(); ++e) {
					if ((*e)->base == facets[i]) {
						glColor3f(1, 0.8, 0.6);
						selected = true;
						break;
					}
				}
				for (auto v = selections.vertices.begin(); v != selections.vertices.end(); ++v) {
					if ((*v)->base == facets[i]) {
						glColor3f(1, 0.8, 0.6);
						selected = true;
						break;
					}
				}
				if (!selected) {
					for (auto e = selections.edges.begin(); e != selections.edges.end(); ++e) {
						if ((*e)->opposite && (*e)->opposite->base == facets[i]) {
							glColor3f(0.5, 0.5, 0.5);
							break;
						}
					}
				}

				facets[i]->draw(texture);

				glBindTexture(GL_TEXTURE_2D, 0);
			}

			// draw selected edge
			glColor3f(0, 0, 1);
			glLineWidth(2);
			glDisable(GL_DEPTH_TEST);
			glBegin(GL_LINES);
			for (auto i = selections.edges.begin(); i != selections.edges.end(); ++i) {
				Point3D u = (*i)->u->to_point3d();
				Point3D v = (*i)->v->to_point3d();
				glVertex3f(u.x, u.y, u.z);
				glVertex3f(v.x, v.y, v.z);
			}
			glEnd();
			glDisable(GL_POLYGON_OFFSET_LINE);

			// draw selected vertex
			glColor3f(0, 0, 1);
			glPointSize(5);
			glDisable(GL_DEPTH_TEST);
			glBegin(GL_POINTS);
			for (auto i = selections.vertices.begin(); i != selections.vertices.end(); ++i) {
				Point3D u = (*i)->to_point3d();
				glVertex3f(u.x, u.y, u.z);
			}
			glEnd();
			glDisable(GL_POLYGON_OFFSET_LINE);
		}
		
		void draw_world_annotations(void) {
			for (auto f = annotations.begin(); f != annotations.end(); ++f) {
				for (int i = 0; i < (int)f->second.size(); ++i) {
					f->second[i].draw();
				}
			}
		}

		void draw_interface_elements(void) {
			for (int i = 0; i < (int)buttons.size(); ++i) {
				buttons[i].draw();
			}
		}

		PlanarFacet* hit_test_facets(const Ray3D& ray, bool cull = true) {
			length_t min_d = 0.0;
			PlanarFacet* ret = 0;
			for (int i = 0; i < (int)facets.size(); ++i) {
				Point2D p;
				length_t d;
				if (!intersect(ray, facets[i]->plane, &d, &p)) continue;
				if (d < camera.th_near) continue;
				if (!facets[i]->includes_point(p)) continue;
				if (cull && ray.v.dot_product(facets[i]->plane.x.cross_product(facets[i]->plane.y)) >= 0) continue;
				if (!ret || d < min_d) {
					ret = facets[i];
					min_d = d;
				}
			}
			return ret;
		}

		PlanarFacetEdge* hit_test_edge(const Ray3D& ray, PlanarFacet* f, length_t th) {
			length_t min_d = 0.0;
			PlanarFacetEdge* ret = 0;
			for (int i = 0; i < (int)f->edges.size(); ++i) {
				Point2D p;
				length_t d;
				Point3D u = f->edges[i]->u->to_point3d();
				Point3D v = f->edges[i]->v->to_point3d();
				Line3D l(u, v - u);
				length_t line_pos;
				if (!intersect(ray, l, th, 0, &line_pos, &d)) continue;
				if (line_pos < 0 || 1 < line_pos) continue;
				if (!ret || d < min_d) {
					ret = f->edges[i];
					min_d = d;
				}
			}
			return ret;
		}

		PlanarFacetVertex* hit_test_vertex(const Ray3D& ray, PlanarFacet* f, length_t th) {
			length_t min_d = 0.0;
			PlanarFacetVertex* ret = 0;
			for (int i = 0; i < (int)f->vertices.size(); ++i) {
				Point2D p;
				length_t d;
				Point3D u = f->vertices[i]->to_point3d();
				if (!intersect(ray, u, th, 0, &d)) continue;
				if (!ret || d < min_d) {
					ret = f->vertices[i];
					min_d = d;
				}
			}
			return ret;
		}

		bool mouse_event_facet_select(int x, int y, int m) {
			Ray3D ray = camera.getRay(x, y);
			PlanarFacet* f = hit_test_facets(ray);
			if (!f) return false;
			if (m & GLUT_ACTIVE_SHIFT) {
				selections.add_facet(f);
			}
			else {
				selections.select_facet(f);
			}
			return true;
		}

		bool mouse_event_edge_select(int x, int y, int m) {
			Ray3D ray = camera.getRay(x, y);
			PlanarFacet* f = hit_test_facets(ray);
			if (!f) return false;

			PlanarFacetEdge* e = hit_test_edge(ray, f, 0.1);
			if (!e) return false;
			if (m & GLUT_ACTIVE_SHIFT) {
				selections.add_edge(e);
			}
			else {
				selections.select_edge(e);
			}
			return true;
		}

		bool mouse_event_vertex_select(int x, int y, int m) {
			Ray3D ray = camera.getRay(x, y);
			PlanarFacet* f = hit_test_facets(ray);
			if (!f) return false;

			PlanarFacetVertex* v = hit_test_vertex(ray, f, 0.1);
			if (!v) return false;
			if (m & GLUT_ACTIVE_SHIFT) {
				selections.add_vertex(v);
			}
			else {
				selections.select_vertex(v);
			}
			return true;
		}

		bool mouse_event_select_any(int x, int y, int m) {
			Ray3D ray = camera.getRay(x, y);
			PlanarFacet* f = hit_test_facets(ray);
			if (!f) return false;

			PlanarFacetVertex* v = hit_test_vertex(ray, f, 0.1);
			if (v) {
				if (m & GLUT_ACTIVE_SHIFT) {
					selections.add_vertex(v);
				}
				else {
					selections.select_vertex(v);
				}
				return true;
			}

			PlanarFacetEdge* e = hit_test_edge(ray, f, 0.1);
			if (e) {
				if (m & GLUT_ACTIVE_SHIFT) {
					selections.add_edge(e);
				}
				else {
					selections.select_edge(e);
				}
				return true;
			}

			if (m & GLUT_ACTIVE_SHIFT) {
				selections.add_facet(f);
			}
			else {
				selections.select_facet(f);
			}
			return true;

		}

		bool mouse_event_make_annotation(int x, int y, int m) {
			Ray3D ray = camera.getRay(x, y);
			PlanarFacet* f = hit_test_facets(ray);
			if (!f) return false;

			Point2D i_point2;
			intersect(ray, f->plane, 0, &i_point2);
			Point3D i_point = f->plane.convert(i_point2);

			int annotation_i = -1;
			int annotation_j = -1;

			vector<Annotation>& anns = annotations[f];
			for (int i = 0; i < anns.size(); ++i) {
				annotation_j = anns[i].hit_test(ray);
				if (annotation_j != -1) {
					annotation_i = i;
					break;
				}
			}
			if (annotation_i != -1) {
				current_selected_point = &anns[annotation_i].boundary[annotation_j];
				current_selected_facet = f;
				current_reference_point[0] = &anns[annotation_i].boundary[(annotation_j + anns[annotation_i].boundary.size() - 1) % anns[annotation_i].boundary.size()];
				current_reference_point[1] = &anns[annotation_i].boundary[(annotation_j + 1) % anns[annotation_i].boundary.size()];
				return true;
			}

			anns.push_back(Annotation());

			anns[anns.size() - 1].boundary.push_back(i_point);
			anns[anns.size() - 1].boundary.push_back(i_point);

			current_reference_point[0] = &anns[anns.size() - 1].boundary[0];
			current_selected_point = &anns[anns.size() - 1].boundary[1];
			current_selected_facet = f;

			return true;
		}
		
		void onMouseMotion(int x, int y, int m) {
			if (current_selected_point) {
				Ray3D ray = camera.getRay(x, y);
				
				PlanarFacet* f = current_selected_facet;
				if (!f) f = hit_test_facets(ray);
				if (f) {
					length_t a;
					Point2D pt;
					if (intersect(ray, f->plane, &a, &pt)) {
						if (f->includes_point(pt)) {
							Point3D p = f->plane.convert(pt);
							if ((m & GLUT_ACTIVE_SHIFT)) {
								for (int i = 0; i < 2; ++i) {
									if (!current_reference_point[i]) continue;
									Vector3D delta = (p - *current_reference_point[i]);
									delta.x = abs(delta.x); delta.y = abs(delta.y); delta.z = abs(delta.z);
									
									if( delta.x < 0.1) p.x = current_reference_point[i]->x;
									if (delta.y < 0.1) p.y = current_reference_point[i]->y;
									if (delta.z < 0.1) p.z = current_reference_point[i]->z;
								}
							}
							*current_selected_point = p;
							glutPostRedisplay();
						}
					}
				}
			}
			BasicViewer::onMouseMotion(x, y, m);
		}

		void onMouse(int button, int state, int x, int y, int m) {
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
				for (int i = 0; i < buttons.size(); ++i) {
					if (buttons[i].hitTest(x, y)) {
						buttons[i].send_clickevent();
						return;
					}
				}

				if (mode) {
					mouse_event_select_any(x, y, m);
				}
				else {
					mouse_event_make_annotation(x, y, m);
				}
			}

			if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
				if (current_selected_point) {
					current_selected_point = 0;
					if (current_selected_facet && annotations.count(current_selected_facet)) {
						vector<Annotation>& ann = annotations.at(current_selected_facet);
						for (int i = 0; i < ann.size(); ++i) {
							if (ann[i].boundary.size() == 1 || ann[i].boundary.size() == 2 && ann[i].boundary[0] == ann[i].boundary[1]) {
								ann.erase(ann.begin() + i);
								--i;
							}
						}
						current_selected_facet = 0;
					}
					current_reference_point[0] = 0;
					current_reference_point[1] = 0;
				}
				if (selections.vertices.size() == 1) {
					PlanarFacetVertex *v = *(selections.vertices.begin());
					do {
						Ray3D ray = camera.getRay(x, y);
						PlanarFacet* f = hit_test_facets(ray);
						if (!f) break;
						PlanarFacetEdge* e = hit_test_edge(ray, f, 0.1);
						if (!e) break;
						if (e->u == v || e->v == v) break;
						if (v->base == e->base) {
							Line2D l(e->u->p, e->v->p - e->u->p);
							Point2D u = v->p;
							length_t alpha = 0;
							l.get_projection_of(u, &alpha);
							if (alpha <= EPSILON || 1 - EPSILON <= alpha) break;
							e->split(alpha);
						}

					} while (0);
				}
				
			}

			if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
				main_viewer->selections.clear();
			}

			BasicViewer::onMouse(button, state, x, y, m);
		}
		
		void do_function(void) {
			if (selections.vertices.size() == 2) {
				PlanarFacetVertex *u = *(selections.vertices.begin());
				PlanarFacetVertex *v = *(selections.vertices.rbegin());
				if (u->base == v->base && annotations.count(u->base) == 0) {
					PlanarFacet* newf = u->base->split(u, v);
					if (texture_map.count(u->base)) {
						texture_map[newf] = texture_map.at(u->base);
					}
					facets.push_back(newf);
				}
			}
			if (selections.facets.size() == 2) {
				PlanarFacet *f1 = *(selections.facets.begin());
				PlanarFacet *f2 = *(selections.facets.rbegin());
				if (annotations.count(f1) == 0 && annotations.count(f2) == 0) {
					if (f1->merge(f2)) {
						texture_map.erase(f2);
						delete f2;
						for (int i = 0; i < facets.size(); ++i) {
							if (facets[i] == f2) {
								facets.erase(facets.begin() + i);
								break;
							}
						}
					}
				}
			}
			if (!selections.edges.empty()) {
				if (selections.edges.size() == 1) {
					PlanarFacetEdge *e1 = *(selections.edges.begin());
					e1->set_opposite_connection(0);
				}
				else if (selections.edges.size() == 2) {
					PlanarFacetEdge *e1 = *(selections.edges.begin());
					PlanarFacetEdge *e2 = *(selections.edges.rbegin());
					e1->set_opposite_connection(e2);
				}
				else {
					vector<PlanarFacetEdge*> es;
					for (auto e = selections.edges.begin(); e != selections.edges.end(); ++e) {
						es.push_back(*e);
					}
					bool failed = false;
					for (int i = 0; i < ((int)es.size()) - 1; ++i) {
						int j = i + 1;
						Point3D v = es[i]->v->to_point3d();
						for (; j < (int)es.size(); ++j) {
							if (v == es[j]->u->to_point3d()) {
								break;
							}
						}
						if (j == (int)es.size()) {
							failed = true;
							break;
						}
						swap(es[i + 1], es[j]);
					}
					if (!(es[es.size() - 1]->v->to_point3d() == es[0]->u->to_point3d())) {
						failed = true;
					}
					if (!failed) {
						vector<Point3D> pts;
						for (int i = 0; i < (int)es.size(); ++i) {
							pts.push_back(es[i]->u->to_point3d());
						}
						Vector3D v01 = (pts[1] - pts[0]).normalized();
						Vector3D max_normal(0, 0, 0);
						scalar_t max_normal_size = 0;
						Vector3D max_v0i(0, 0, 0);
						for (int i = 2; i < (int)pts.size(); ++i) {
							Vector3D v0i = (pts[i] - pts[0]).normalized();
							Vector3D normal = v01.cross_product(v0i);
							scalar_t normal_size = normal.dot_product(normal);
							if (max_normal_size < normal_size) {
								max_normal_size = normal_size;
								max_normal = normal;
								max_v0i = v0i;
							}
						}
						if (!max_normal.is_zero()) {
							Plane plane(pts[0], max_v0i, v01);
							PlanarFacet *f = new PlanarFacet(plane);

							PlanarFacetVertex *prev_v = 0;
							for (int i = (int)pts.size() - 1; i >= 0; --i) {
								PlanarFacetVertex *v = f->create_vertex(plane.convert(pts[i]));
								if (prev_v) {
									PlanarFacetEdge *e = f->create_edge(prev_v, v);
									e->set_opposite_connection(es[i]);
								}
								prev_v = v;
							}
							{
								PlanarFacetEdge *e = f->create_edge(prev_v, f->vertices[0]);
								e->set_opposite_connection(es[((int)pts.size()) - 1]);
								f->set_exterior(e);
							}
							facets.push_back(f);
						}
					}

				}
			}
			glutPostRedisplay();
		}

		void onKeyDown(unsigned char ch, int x, int y, int m) {
			if (ch == 127) {
				if (current_selected_point) {
					if (current_selected_facet && annotations.count(current_selected_facet)) {
						vector<Annotation>& ann = annotations.at(current_selected_facet);
						for (int i = 0; i < (int)ann.size(); ++i) {
							for (int j = 0; j < (int)ann[i].boundary.size(); ++j) {
								if (&ann[i].boundary[j] == current_selected_point) {
									ann[i].boundary.erase(ann[i].boundary.begin() + j);
									current_selected_point = 0;
									break;
								}
							}
							if (!current_selected_point) {
								if (ann[i].boundary.size() == 1) {
									ann.erase(ann.begin() + i);
									if (ann.empty()) annotations.erase(current_selected_facet);
								}
								break;
							}
						}
						current_selected_facet = 0;
					}
					current_reference_point[0] = 0;
					current_reference_point[1] = 0;
					glutPostRedisplay();
					return;
				}
			}
			if (ch == '\t') {
				do_function();
			}
			BasicViewer::onKeyDown(ch, x, y, m);
		}

		void onResize(int w, int h) {
			int n = buttons.size();
			for (int i = 0; i < n; ++i) {
				if (buttons[i].y != 0) {
					buttons[i].y = h - buttons[i].h;
					if (buttons[i].y < buttons[i].h) {
						buttons[i].y = buttons[i].h;
					}
				}
			}
			BasicViewer::onResize(w, h);
		}
	} *main_viewer = 0;

	void btn_callback_open_rsd(void) {
		main_viewer->load();
	}
	void btn_callback_save_rsd(void) {
		string fname = open_file_browser(".", true, "rsd");
		if (fname.empty()) return;
		ofstream out(fname.c_str());
		if (!out) return;
		main_viewer->save_in_rsd(out);
		out.close();
	}
	void btn_callback_import_pc(void) {
		string fname = open_file_browser(".", false, "pcd");
		if (fname.empty()) return;
		ifstream in(fname.c_str(), ios::binary);
		PointCloud *pc = new PointCloud();
		if (!pc->read_from_pcd(in, 0.1)) {
			delete pc;
			return;
		}
		main_viewer->pointcloud = pc;
		in.close();
	}
	void btn_callback_save_indoorGML(void) {
		string fname = open_file_browser(".", true, "info");
		if (fname.empty()) return;
		ofstream out(fname.c_str());
		if (!out) return;
		main_viewer->save_in_indoorgml_intermediate_data(out);
		out.close();
	}
	void btn_callback_action(void) {
		main_viewer->do_function();
	}
	void btn_callback_auto_connect(void) {
		main_viewer->find_edge_connection_by_spatial_distance();
	}
	void btn_callback_mode(void) {
		main_viewer->selections.clear();
		main_viewer->mode = !main_viewer->mode;
		if (main_viewer->mode) {
			main_viewer->buttons[6].set_caption("Mode: Poly");
		}
		else {
			main_viewer->buttons[6].set_caption("Mode: Annot");
		}
	}
}


int main_editor(int argc, char**argv) {

	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_DOUBLE);
	int w = glutCreateWindow("TICA v0.1.0");
	glewInit();

	init_viewer_handler();
	app_editor::main_viewer = new app_editor::EditorViewer();
	register_viewer(app_editor::main_viewer);

	glutMainLoop();

	return 0;
}
#include "Viewer.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static void timer_callback(int value);
static void keyboard_down_callback(unsigned char key, int x, int y);
static void keyboard_up_callback(unsigned char key, int x, int y);
static void special_down_callback(int key, int x, int y);
static void special_up_callback(int key, int x, int y);
static void mouse_callback(int button, int state, int x, int y);
static void mousemotion_callback(int x, int y);
static void mousemove_callback(int x, int y);
static void reshape_callback(int w, int h);
static void draw_callback(void);

class ViewerController {
	ViewerController(void) : viewer(0) { ; }

	Viewer *viewer;
	static ViewerController* instance;
	friend static void timer_callback(int value);
	friend static void keyboard_down_callback(unsigned char key, int x, int y);
	friend static void keyboard_up_callback(unsigned char key, int x, int y);
	friend static void special_down_callback(int key, int x, int y);
	friend static void special_up_callback(int key, int x, int y);
	friend static void mouse_callback(int button, int state, int x, int y);
	friend static void mousemotion_callback(int x, int y);
	friend static void mousemove_callback(int x, int y);
	friend static void reshape_callback(int w, int h);
	friend static void entry_callback(int state);
	friend static void draw_callback(void);
	friend void register_viewer(Viewer* _viewer);

	void onTimer(int value) {
		glutTimerFunc(20, timer_callback, 0);
		if (viewer) viewer->onTimer(value);
	}

	void onKeyDown(unsigned char key, int x, int y) {
		if (viewer) viewer->onKeyDown(key, x, y, glutGetModifiers());
	}

	void onKeyUp(unsigned char key, int x, int y) {
		if (viewer) viewer->onKeyUp(key, x, y, glutGetModifiers());
	}

	void onSpecialKeyDown(unsigned char key, int x, int y) {
		if (viewer) viewer->onSpecialKeyDown(key, x, y, glutGetModifiers());
	}

	void onSpecialKeyUp(unsigned char key, int x, int y) {
		if (viewer) viewer->onSpecialKeyUp(key, x, y, glutGetModifiers());
	}

	void onMouse(int button, int state, int x, int y) {
		if (viewer) viewer->onMouse(button, state, x, y, glutGetModifiers());
	}

	void onMouseMotion(int x, int y) {
		if (viewer) viewer->onMouseMotion(x, y, glutGetModifiers());
	}

	void onMouseMove(int x, int y) {
		if (viewer) viewer->onMouseMove(x, y, glutGetModifiers());
	}

	void onReshape(int w, int h) {
		if (viewer) viewer->onResize(w, h);
	}

	void onDraw(void) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		if (viewer) viewer->onDraw();
		glutSwapBuffers();
	}

	void onEntry(int state) {
		if (viewer) viewer->onEntry(state);
	}

	void setViewer(Viewer* _viewer) {
		viewer = _viewer;
	}

public:
	static ViewerController* getInstance() {
		if (!instance) instance = new ViewerController();
		return instance;
	}
};

ViewerController* ViewerController::instance = 0;

static void timer_callback(int value) { ViewerController::getInstance()->onTimer(value); }
static void keyboard_down_callback(unsigned char key, int x, int y) { ViewerController::getInstance()->onKeyDown(key,x,y); }
static void keyboard_up_callback(unsigned char key, int x, int y) { ViewerController::getInstance()->onKeyUp(key, x, y); }
static void special_down_callback(int key, int x, int y) { ViewerController::getInstance()->onSpecialKeyDown(key, x, y); }
static void special_up_callback(int key, int x, int y) { ViewerController::getInstance()->onSpecialKeyUp(key, x, y); }
static void mouse_callback(int button, int state, int x, int y) { ViewerController::getInstance()->onMouse(button, state, x, y); }
static void mousemotion_callback(int x, int y) { ViewerController::getInstance()->onMouseMotion(x, y); }
static void mousemove_callback(int x, int y) { ViewerController::getInstance()->onMouseMove(x, y); }
static void reshape_callback(int w, int h) { ViewerController::getInstance()->onReshape(w, h); }
static void entry_callback(int state) { ViewerController::getInstance()->onEntry(state); }
static void draw_callback(void) {
	ViewerController::getInstance()->onDraw();
}

void register_viewer(Viewer* _viewer) {
	ViewerController::getInstance()->setViewer(_viewer);
}

void init_viewer_handler(void) {
	glutReshapeFunc(reshape_callback);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(mousemotion_callback);
	glutPassiveMotionFunc(mousemove_callback);
	glutSpecialFunc(special_down_callback);
	glutSpecialUpFunc(special_up_callback);
	glutKeyboardFunc(keyboard_down_callback);
	glutKeyboardUpFunc(keyboard_up_callback);
	glutEntryFunc(entry_callback);
	glutTimerFunc(20, timer_callback, 0);
	glutDisplayFunc(draw_callback);
}
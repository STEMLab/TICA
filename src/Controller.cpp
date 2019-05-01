#include "Controller.h"

Controller::Controller(Controller *_prev)
	: prev(_prev) {
	;
}

void Controller::end(void) {
	Controller::current_controller = prev;
	delete this;
}

int Controller::get_current_stage(void)const {
	return 0;
}
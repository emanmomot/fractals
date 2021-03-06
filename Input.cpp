
#include "Input.h"

Input::Input(int vW, int vH) {
	this->vW = vW;
	this->vH = vH;
	
	lastx = vW/2;
	lasty = vH/2;
	glutWarpPointer(lastx, lasty);
}

void
Input::update(Camera* cam) {
	cam->rotate(dx/100.0f, vec3(0,1,0));
	cam->rotate(dy/100.0f, vec3(1,0,0));
	cam->translate(cam->getSpeed() * dir);
	
	dx = 0;
	dy = 0;
}

void
Input::keyboard(unsigned char key, int x, int y) {
    switch(key)
    { 
        case 'w':
			dir.z = -1;
            break;
        case 'a':
			dir.x = -1;
            break;
        case 's':
			dir.z = 1;
            break;
        case 'd':
			dir.x = 1;
            break;
        case 'q':
			exit(1);
			break;
        default:
            break;
    }
}

void 
Input::keyboardUp(unsigned char key, int x, int y) {
    switch(key)
    {
		case 'w': case 's':
			dir.z = 0;
            break;
		case 'a': case 'd':
			dir.x = 0;
            break;
        default:
            break;
    }
}

void 
Input::passivemotion( int x, int y ) {
	
	dx = x - lastx;
	dy = y - lasty;
	
	lastx = x;
	lasty = y;
}

void
Input::entry(int state) {
	if(state == GLUT_LEFT) {
		lastx = vW/2;
		lasty = vH/2;
		glutWarpPointer(lastx, lasty);
	}
}

void
Input::setViewport(int vW, int vH) {
	this->vW = vW;
	this->vH = vH;
}

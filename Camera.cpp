
#include "Camera.h"

Camera::Camera(vec3 pos, float speed) {
	this->pos = pos;
	this->rot = quat();
	this->speed = speed;
}

void
Camera::rotate(float angle, vec3 axis) {
	rot = angleAxis(angle, axis) * rot;
}

void
Camera::translate(vec3 t) {
	pos += rot * t;
}

vec3
Camera::forward() {
	return rot * vec3(0,0,1);
}

vec3
Camera::up() {
	return rot * vec3(0,-1,0);
}

void
Camera::setMatrices() {
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(50.0, 1.0, .1f, 100);

	vec3 l = pos + forward();
	vec3 u = up();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	gluLookAt(pos.x, pos.y, pos.z,	//pos
		l.x, l.y, l.z,				//lookatpos
		u.x, u.y, u.z);				//up
}

float
Camera::getSpeed() {
	return speed;
}

void
Camera::setSpeed(float speed) {
	this->speed = speed;
}



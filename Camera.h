
#ifndef CAMERA_H
#define CAMERA_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

using namespace glm;

class Camera {

public:
	Camera(vec3 pos, float speed);

	void rotate(float angle, vec3 axis);
	void translate(vec3 t);
	void setMatrices();

	float getSpeed();
	void setSpeed(float speed);

private:
	vec3 pos;
	quat rot;
	float speed;

	vec3 forward();
	vec3 up();
};
#endif	

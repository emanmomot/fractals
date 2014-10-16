#pragma once

#include "Camera.h"

class Input
{
public:
	Input(int vW, int vH);
	~Input() { };

	void keyboard(unsigned char key, int x, int y);
	void keyboardUp(unsigned char key, int x, int y);
	void passivemotion( int x, int y );
	void update(Camera* cam);
	void setViewport(int vW, int vH);
	
private:
	int vW;
	int vH;
	int dx;
	int dy;

	vec3 dir;
};


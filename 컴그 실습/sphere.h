#pragma once
#include "object.h"

class Sphere :public Object
{
public:
	glm::vec4 initcolor;
	glm::vec4* colordata;
	glm::vec3* normaldata;

	Sphere() { }
	Sphere(glm::vec4 color) {
		initcolor = color;
	}

	float speed = (float)(rand() % 9) / 1000 + 0.001;
	void Initialize();
	void Update();
	void Draw(int programID);
};
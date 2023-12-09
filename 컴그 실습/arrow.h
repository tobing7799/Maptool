#pragma once
#include "object.h"

class Arrow :public Object
{
public:
	glm::vec4 initcolor;
	glm::vec4* colordata;
	glm::vec3* normaldata;

	Arrow() { }
	Arrow(glm::vec4 color) {
		initcolor = color;
	}

	void Initialize();
	void Update();
	void Draw(int programID);
};
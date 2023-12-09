#pragma once
#include "object.h"

class Cube :public Object
{
public:
	glm::vec4 initcolor;
	glm::vec4* colordata;
	glm::vec3* normaldata;

	Cube() { }
	Cube(glm::vec4 color){
		initcolor = color;
	}
	virtual void Initialize();
	virtual void Update();
	virtual void Draw(int programID) override;
};

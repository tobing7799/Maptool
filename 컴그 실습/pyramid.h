#pragma once
#include "object.h"

class Pyramid :public Object
{
public:
	glm::vec4* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;
	void Initialize();
	void Update();
	void Draw(int programID, unsigned int* texture);
};
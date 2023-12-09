#pragma once
#include "object.h"

class Flat :public Object
{
public:
	glm::vec4* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	Rect collisionbox{};

	void Initialize();
	void Update();
	void Draw(int programID, unsigned int texture);
};
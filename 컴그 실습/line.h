#pragma once
#include "object.h"

class Line :public Object
{
public:
	glm::vec4* colordata;
	void Initialize();
	void Update();
	void Draw(int programID);
};
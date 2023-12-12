#pragma once
#include "object.h"

class Line :public Object
{
public:
	int linenumber = 1001; // 이거 바꾸면 vertexA.glsl도 수정 필요함
	float lineOffsetSize = 0.5;
	vector<glm::vec3> positiondata_x;
	vector<glm::vec3> positiondata_z;
	bool instance_x_z = true;
	glm::vec4* colordata;
	void Initialize();
	void Update();
	void Draw(int programID);
};
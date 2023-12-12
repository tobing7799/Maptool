#pragma once
#include "object.h"

class Line :public Object
{
public:
	int linenumber = 1001; // �̰� �ٲٸ� vertexA.glsl�� ���� �ʿ���
	float lineOffsetSize = 0.5;
	vector<glm::vec3> positiondata_x;
	vector<glm::vec3> positiondata_z;
	bool instance_x_z = true;
	glm::vec4* colordata;
	void Initialize();
	void Update();
	void Draw(int programID);
};
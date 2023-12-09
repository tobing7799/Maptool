#pragma once
#include "stdafx.h"

class Object {
public:
	GLuint vao, vbo[4];
	glm::vec3* vertexdata;
	Transform objectmatrix;
	Transform modelmatrix;
	Object* parent{ nullptr };
	virtual void Initialize() {};
	virtual void Update() {};
	virtual void Draw(int programID) {};
	glm::mat4 GetTransform()
	{
		if (parent) return parent->GetTransform() * objectmatrix.GetTransform();
		return objectmatrix.GetTransform();
	}

	glm::mat4 GetmodelTransform() { return modelmatrix.GetTransform(); };

	void Readobj(FILE* objFile);
};
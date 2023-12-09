#pragma once
#include "line.h"

void Line::Initialize()
{
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * 2);
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 2);
	vertexdata[0] = glm::vec3(0.0, 1.0, 0.0);
	vertexdata[1] = glm::vec3(0.0, -1.0, 0.0);

	colordata[0] = glm::vec4(0.6, 0.6, 0.6, 0.8);
	colordata[1] = glm::vec4(0.6, 0.6, 0.6, 0.8);


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void Line::Update()
{
}

void Line::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glDrawArrays(GL_LINES, 0, 2);
}

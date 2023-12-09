#pragma once
#include "cube.h"

void Cube::Initialize()
{
	//cout << "큐브 이니셜" << endl;
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 36);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 36);

	for (int i = 0; i < 6; i++)
	{
		colordata[i * 6] = initcolor;
		colordata[i * 6 + 1] = colordata[i * 6];
		colordata[i * 6 + 2] = colordata[i * 6];
		colordata[i * 6 + 3] = colordata[i * 6];
		colordata[i * 6 + 4] = colordata[i * 6];
		colordata[i * 6 + 5] = colordata[i * 6];
	}

	for (int i = 0; i < 36; i++)
	{
		vertexdata[i] -= glm::vec3(0.5, 0.5, 0.5);
	}

	for (int i = 0; i < 6; i++)
	{
		glm::vec3 normal = glm::cross(vertexdata[i * 6 + 1] - vertexdata[i * 6 + 0], vertexdata[i * 6 + 2] - vertexdata[i * 6 + 0]);
		normaldata[i * 6 + 0] = normal;
		normaldata[i * 6 + 1] = normal;
		normaldata[i * 6 + 2] = normal;
		normaldata[i * 6 + 3] = normal;
		normaldata[i * 6 + 4] = normal;
		normaldata[i * 6 + 5] = normal;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void Cube::Update()
{
	for (int i = 0; i < 6; i++)
	{
		colordata[i * 6] = initcolor;
		colordata[i * 6 + 1] = colordata[i * 6];
		colordata[i * 6 + 2] = colordata[i * 6];
		colordata[i * 6 + 3] = colordata[i * 6];
		colordata[i * 6 + 4] = colordata[i * 6];
		colordata[i * 6 + 5] = colordata[i * 6];
	}

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void Cube::Draw(int programID)
{
	//cout << "큐브 드로우" << endl;
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
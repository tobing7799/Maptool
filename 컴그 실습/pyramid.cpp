#pragma once
#include "pyramid.h"

void Pyramid::Initialize()
{
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 18);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 18);
	uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 18);

	colordata[0] = glm::vec4(1.0, 1.0, 1.0, 1.0);
	for (int i = 1; i < 18; i++)
	{
		colordata[i] = colordata[0];
	}

	for (int i = 0; i < 18; i++)
	{
		vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
	}

	for (int i = 0; i < 5; i++)
	{
		glm::vec3 normal = glm::cross(vertexdata[i * 3 + 1] - vertexdata[i * 3 + 0], vertexdata[i * 3 + 2] - vertexdata[i * 3 + 0]);
		normaldata[i * 3 + 0] = normal;
		normaldata[i * 3 + 1] = normal;
		normaldata[i * 3 + 2] = normal;
	}

	for (int i = 0; i < 4; i++)
	{
		uvdata[i * 3 + 0] = glm::vec2(0.0, 0.0);
		uvdata[i * 3 + 1] = glm::vec2(1.0, 0.0);
		uvdata[i * 3 + 2] = glm::vec2(0.5, 1.0);
	}
	uvdata[12] = glm::vec2(0.0, 0.0);
	uvdata[13] = glm::vec2(1.0, 0.0);
	uvdata[14] = glm::vec2(1.0, 1.0);
	uvdata[15] = glm::vec2(0.0, 0.0);
	uvdata[16] = glm::vec2(1.0, 1.0);
	uvdata[17] = glm::vec2(0.0, 1.0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(4, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);
}

void Pyramid::Update()
{
}

void Pyramid::Draw(int programID, unsigned int* texture)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glDrawArrays(GL_TRIANGLES, 3, 3);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glDrawArrays(GL_TRIANGLES, 6, 3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glDrawArrays(GL_TRIANGLES, 9, 3);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glDrawArrays(GL_TRIANGLES, 12, 6);
}

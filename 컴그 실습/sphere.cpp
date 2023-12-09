#pragma once
#include "sphere.h"

void Sphere::Initialize()
{
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 240);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 240);

	//colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
	colordata[0] = initcolor;

	for (int i = 1; i < 240; i++)
	{
		colordata[i] = colordata[0];
	}

	for (int i = 0; i < 80; i++)
	{
		glm::vec3 normal = glm::cross(vertexdata[i * 3 + 1] - vertexdata[i * 3 + 0], vertexdata[i * 3 + 2] - vertexdata[i * 3 + 0]);
		normaldata[i * 3 + 0] = normal;
		normaldata[i * 3 + 1] = normal;
		normaldata[i * 3 + 2] = normal;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(3, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void Sphere::Update()
{

	colordata[0] = initcolor;

	for (int i = 1; i < 240; i++)
	{
		colordata[i] = colordata[0];
	}

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

}

void Sphere::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform"); //--- 버텍스 세이더에서 모델링 변환 위치 가져오기
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 240);
}

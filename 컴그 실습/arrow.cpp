#include "arrow.h"

void Arrow::Initialize()
{
	colordata = (glm::vec4*)malloc(sizeof(glm::vec4) * 282);
	normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 282);

	for (int i = 0; i < 47; i++)
	{
		colordata[i * 6] = initcolor;
		colordata[i * 6 + 1] = colordata[i * 6];
		colordata[i * 6 + 2] = colordata[i * 6];
		colordata[i * 6 + 3] = colordata[i * 6];
		colordata[i * 6 + 4] = colordata[i * 6];
		colordata[i * 6 + 5] = colordata[i * 6];
	}

	for (int i = 0; i < 282; i++)
	{
		vertexdata[i] += glm::vec3(0.0, 0.0, 2.0);
	}

	for (int i = 0; i < 282; i++)
	{
		normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(3, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void Arrow::Update()
{
	for (int i = 0; i < 47; i++)
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
	glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void Arrow::Draw(int programID)
{
	unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 282);
}

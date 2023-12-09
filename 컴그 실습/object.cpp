#pragma once
#include "object.h"

void Object::Readobj(FILE* objFile)
{
	fseek(objFile, 0, 0);
	char count[400];
	int vertexNum = 0;
	int faceNum = 0;
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0')
			vertexNum += 1;
		else if (count[0] == 'f' && count[1] == '\0')
			faceNum += 1;
		memset(count, '\0', sizeof(count));
	}

	glm::vec3* vertex;
	glm::vec3* face;
	int vertIndex = 0;
	int faceIndex = 0;
	int normalIndex = 0;
	vertex = (glm::vec3*)malloc(sizeof(glm::vec3) * vertexNum);
	vertexdata = (glm::vec3*)malloc(sizeof(glm::vec3) * faceNum * 3);
	face = (glm::vec3*)malloc(sizeof(glm::vec3) * faceNum);

	fseek(objFile, 0, 0);
	while (!feof(objFile)) {
		fscanf(objFile, "%s", count);
		if (count[0] == 'v' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f",
				&vertex[vertIndex].x, &vertex[vertIndex].y,
				&vertex[vertIndex].z);
			vertIndex++;
		}
		else if (count[0] == 'f' && count[1] == '\0') {
			fscanf(objFile, "%f %f %f", &face[faceIndex].x, &face[faceIndex].y, &face[faceIndex].z);
			vertexdata[faceIndex * 3 + 0] = vertex[(int)face[faceIndex].x - 1];
			vertexdata[faceIndex * 3 + 1] = vertex[(int)face[faceIndex].y - 1];
			vertexdata[faceIndex * 3 + 2] = vertex[(int)face[faceIndex].z - 1];
			faceIndex++;
		}
	}
}
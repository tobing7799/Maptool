#pragma once
#include<stdlib.h>
#include<stdio.h>
#include<iostream>
#include<gl/glew.h>
#include<gl/freeglut.h>
#include<gl/freeglut_ext.h>
#include<glm/glm.hpp>
#include<glm/ext.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<time.h>
#include<math.h>
#include <vector>

using namespace std;

#define PI 3.141592
#define SIZE 60
#define SIZE_cube 5

struct Transform {
	glm::vec3 position = glm::vec3(0.0,0.0,0.0);
	glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 GetTransform()
	{
		glm::mat4 TR = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 SR = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 RX = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 RY = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0)); 
		return TR * RX * RY * RZ * SR;
	}
};

struct Rect {
	float left = 0;
	float top = 0;
	float right = 0;
	float bottom = 0;
};
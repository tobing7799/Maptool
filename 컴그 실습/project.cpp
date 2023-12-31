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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define SNOW_SIZE 100
#define GRASS_SIZE 100
#define CUBE_SIZE 20
#define PI 3.141592
#pragma comment(lib, "winmm.lib")
#include "Mmsystem.h"
#include "Digitalv.h"

char* filetobuf(const char *file);
void InitBuffer();
void InitShader();
void Timer(int value);
int make_vertexShaders();
int make_fragmentShaders();
void Convert_xy(int x, int y);
void InitTexture();
void key_check();

GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Motion(int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid mouseWheel(int button, int dir, int x, int y);
GLvoid Reshape(int w, int h);
GLvoid Keyborad(unsigned char key, int x, int y);
GLvoid Keyborad_up(unsigned char key, int x, int y);
GLuint shaderID;
GLint width, height;
GLuint s_program;
GLchar* vertexsource;
GLuint vertexShader;GLchar* fragmentsource;
GLuint fragmentShader;

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);

	glm::mat4 GetTransform()
	{
		glm::mat4 TR = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 SR = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 RX = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
		glm::mat4 RY = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), (float)glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
		return TR * RX*RY*RZ* SR;
	}
};

glm::mat4 cameratransform;
glm::mat4 cameratransform1;
glm::mat4 cameratransform2;
glm::mat4 cameratransform3;
glm::mat4 cameratransform4;
glm::mat4 cameratransform5;
glm::mat4 cameratransform6;
unsigned int texture[42];

class Object {
public:
	GLuint vao, vbo[4];
	glm::vec3 *vertexdata;
	glm::vec3 *uvdata;
	Transform objectmatrix;
	Transform modelmatrix;
	Object *parent{ nullptr };
	void Initialize()
	{
	}
	void Update()
	{
	}
	void Draw(int programID)
	{
	}
	glm::mat4 GetTransform()
	{
		if (parent)
			return parent->GetTransform()*objectmatrix.GetTransform();
		return objectmatrix.GetTransform();
	}

	glm::mat4 GetmodelTransform()
	{
		return modelmatrix.GetTransform();
	}

	void Readobj(FILE *objFile)
	{
		fseek(objFile, 0, 0);
		char count[1200];
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

		glm::vec3 *vertex;
		glm::vec3 *face;
		int vertIndex = 0;
		int faceIndex = 0;
		int normalIndex = 0;
		vertex = (glm::vec3 *)malloc(sizeof(glm::vec3) * vertexNum);
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * faceNum * 3);
		face = (glm::vec3 *)malloc(sizeof(glm::vec3) * faceNum);

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

	void Readuv(FILE* objFile)
	{
		fseek(objFile, 0, 0);
		char count[1200];
		int uvNum = 0;
		int uvNum2 = 0;
		while (!feof(objFile)) {
			fscanf(objFile, "%s", count);
			if (count[0] == 'v' && count[1] == '\0')
				uvNum += 1;
			else if (count[0] == 'f' && count[1] == '\0')
				uvNum2 += 1;
			memset(count, '\0', sizeof(count));
		}

		glm::vec3* u;
		glm::vec3* uv;
		glm::vec3* uvstore;


		int uvIndex = 0;
		int uvIndex2 = 0;
		uv = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum);
		uvdata = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum * 3);
		u = (glm::vec3*)malloc(sizeof(glm::vec3) * uvNum2);

		fseek(objFile, 0, 0);
		while (!feof(objFile)) {
			fscanf(objFile, "%s", count);
			if (count[0] == 'v' && count[1] == '\0') {
				fscanf(objFile, "%f %f %f",
					&uv[uvIndex].x, &uv[uvIndex].y,
					&uv[uvIndex].z);
				uvIndex++;
			}
			else if (count[0] == 'f' && count[1] == '\0') {
				fscanf(objFile, "%f %f %f", &u[uvIndex2].x, &u[uvIndex2].y, &u[uvIndex2].z);
				uvdata[uvIndex2 * 3 + 0] = uv[(int)u[uvIndex2].x - 1];
				uvdata[uvIndex2 * 3 + 1] = uv[(int)u[uvIndex2].y - 1];
				uvdata[uvIndex2 * 3 + 2] = uv[(int)u[uvIndex2].z - 1];
				uvIndex2++;
			}
		}
	}
};

class Line :public Object
{
public:
	glm::vec4 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	float time = 0;
	float line_x = 0;
	float line_y = 0;
	float line_z = 0;

	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 100);
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 100);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 100);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 100);

		for (int i = 0; i < 100; i++)
		{
			vertexdata[i] = glm::vec3(0.0, 0.0, 0.0);
		}

		for (int i = 1; i < 100; i++)
		{
			colordata[i] = glm::vec4(1.0, 0.0, 0.0,1.0);
		}


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

	}
	void Update(float line_v, float angle_x,float angle_y)
	{
		time = 0;

		if (angle_x < 0)		{			angle_x = 180+angle_x;		}		if (angle_x > 0)		{			angle_x = 90 + angle_x;		}

		angle_x = angle_x * PI / 180.0;

		for (int i = 0; i < 100; i++)
		{
			time += 0.01;			line_z = line_v * cos(angle_y) * time;			line_y = line_v * sin(angle_y) * time - (0.5 * 9.8 * time * time);			if (angle_x != 0)			{				line_x = float(line_z / tan(angle_x));			}			else			{				line_x = 0;			}

			vertexdata[i] = glm::vec3(line_x , line_y, line_z);
		}
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glLineWidth(5);
		glBindTexture(GL_TEXTURE_2D, texture[21]);
		glDrawArrays(GL_LINES, 0, 300);
	}
};

class CubeMap :public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 36);

		for (int i = 0; i < 6; i++)
		{
			colordata[i * 6] = glm::vec3(1.0, 1.0, 1.0);
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

		for (int i = 0; i < 6; i++)
		{
			uvdata[i * 6 + 0] = glm::vec2(0.0, 0.0);
			uvdata[i * 6 + 1] = glm::vec2(1.0, 0.0);
			uvdata[i * 6 + 2] = glm::vec2(1.0, 1.0);
			uvdata[i * 6 + 3] = glm::vec2(0.0, 0.0);
			uvdata[i * 6 + 4] = glm::vec2(1.0, 1.0);
			uvdata[i * 6 + 5] = glm::vec2(0.0, 1.0);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void update()
	{
	}
	void Draw(int programID,int stage)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		if (stage == 0)
		{
			glBindTexture(GL_TEXTURE_2D, texture[4]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindTexture(GL_TEXTURE_2D, texture[1]);
			glDrawArrays(GL_TRIANGLES, 6, 6);
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glDrawArrays(GL_TRIANGLES, 12, 6);
			glBindTexture(GL_TEXTURE_2D, texture[5]);
			glDrawArrays(GL_TRIANGLES, 18, 6);
			glBindTexture(GL_TEXTURE_2D, texture[2]);
			glDrawArrays(GL_TRIANGLES, 24, 6);
			glBindTexture(GL_TEXTURE_2D, texture[3]);
			glDrawArrays(GL_TRIANGLES, 30, 6);
		}
		else if (stage == 3)
		{
			glBindTexture(GL_TEXTURE_2D, texture[8]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindTexture(GL_TEXTURE_2D, texture[9]);
			glDrawArrays(GL_TRIANGLES, 6, 6);
			glBindTexture(GL_TEXTURE_2D, texture[12]);
			glDrawArrays(GL_TRIANGLES, 12, 6);
			glBindTexture(GL_TEXTURE_2D, texture[11]);
			glDrawArrays(GL_TRIANGLES, 18, 6);
			glBindTexture(GL_TEXTURE_2D, texture[13]);
			glDrawArrays(GL_TRIANGLES, 24, 6);
			glBindTexture(GL_TEXTURE_2D, texture[10]);
			glDrawArrays(GL_TRIANGLES, 30, 6);
		}
		else if (stage == 2)
		{
			glBindTexture(GL_TEXTURE_2D, texture[14]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindTexture(GL_TEXTURE_2D, texture[15]);
			glDrawArrays(GL_TRIANGLES, 6, 6);
			glBindTexture(GL_TEXTURE_2D, texture[16]);
			glDrawArrays(GL_TRIANGLES, 12, 6);
			glBindTexture(GL_TEXTURE_2D, texture[19]);
			glDrawArrays(GL_TRIANGLES, 18, 6);
			glBindTexture(GL_TEXTURE_2D, texture[17]);
			glDrawArrays(GL_TRIANGLES, 24, 6);
			glBindTexture(GL_TEXTURE_2D, texture[18]);
			glDrawArrays(GL_TRIANGLES, 30, 6);
		}
		else if (stage == 1)
		{
			glBindTexture(GL_TEXTURE_2D, texture[22]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindTexture(GL_TEXTURE_2D, texture[23]);
			glDrawArrays(GL_TRIANGLES, 6, 6);
			glBindTexture(GL_TEXTURE_2D, texture[24]);
			glDrawArrays(GL_TRIANGLES, 12, 6);
			glBindTexture(GL_TEXTURE_2D, texture[27]);
			glDrawArrays(GL_TRIANGLES, 18, 6);
			glBindTexture(GL_TEXTURE_2D, texture[25]);
			glDrawArrays(GL_TRIANGLES, 24, 6);
			glBindTexture(GL_TEXTURE_2D, texture[26]);
			glDrawArrays(GL_TRIANGLES, 30, 6);
		}
	}
};

class Snow :public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;
	float speed = (((float)(rand() % 1000) / 10000)+0.001);
	void Initialize()
	{
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 6);

		vertexdata[0] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[1] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[2] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[3] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[4] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[5] = glm::vec3(0.0, 1.0, 0.0);

		for (int i = 0; i < 6; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.5, 0.0);
		}
		colordata[0] = glm::vec3(1.0, 1.0, 1.0);
		colordata[1] = colordata[0];
		colordata[2] = colordata[0];
		colordata[3] = colordata[0];
		colordata[4] = colordata[0];
		colordata[5] = colordata[0];

		glm::vec3 normal = glm::cross(vertexdata[1] - vertexdata[0], vertexdata[2] - vertexdata[0]);
		normaldata[0] = normal;
		normaldata[1] = normal;
		normaldata[2] = normal;
		normaldata[3] = normal;
		normaldata[4] = normal;
		normaldata[5] = normal;

		uvdata[0] = glm::vec2(0.0, 0.0);
		uvdata[1] = glm::vec2(1.0, 0.0);
		uvdata[2] = glm::vec2(1.0, 1.0);
		uvdata[3] = glm::vec2(0.0, 0.0);
		uvdata[4] = glm::vec2(1.0, 1.0);
		uvdata[5] = glm::vec2(0.0, 1.0);


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[6]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

class Arrow :public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;

	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 282);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 282);
		for (int i = 0; i < 47; i++)
		{
			colordata[i * 6] = glm::vec3(1.0, 1.0, 1.0);
			colordata[i * 6 + 1] = colordata[i * 6];
			colordata[i * 6 + 2] = colordata[i * 6];
			colordata[i * 6 + 3] = colordata[i * 6];
			colordata[i * 6 + 4] = colordata[i * 6];
			colordata[i * 6 + 5] = colordata[i * 6];
		}

		for (int i = 0; i < 282; i++)
		{
			normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 282 * sizeof(glm::vec3), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D,texture[20]);
		glDrawArrays(GL_TRIANGLES, 0, 282);
	}
};

class Bow :public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;

	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 468);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 468);
		for (int i = 0; i < 78; i++)
		{
			colordata[i * 6] = glm::vec3(1.0, 1.0, 1.0);
			colordata[i * 6 + 1] = colordata[i * 6];
			colordata[i * 6 + 2] = colordata[i * 6];
			colordata[i * 6 + 3] = colordata[i * 6];
			colordata[i * 6 + 4] = colordata[i * 6];
			colordata[i * 6 + 5] = colordata[i * 6];
		}

		for (int i = 0; i < 282; i++)
		{
			normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 468 * sizeof(glm::vec3), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[20]);
		glDrawArrays(GL_TRIANGLES, 0, 468);
	}
};

class Grass :public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	void Initialize()
	{
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) *6);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 6);

		vertexdata[0] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[1] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[2] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[3] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[4] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[5] = glm::vec3(0.0, 1.0, 0.0);

		for (int i = 0; i < 6; i++)
		{
			colordata[i] = glm::vec3(1.0, 1.0, 1.0);
		}

		glm::vec3 normal = glm::cross(vertexdata[1] - vertexdata[0], vertexdata[2] - vertexdata[0]);
		for (int i = 0; i < 6; i++)
		{
			normaldata[i] = normal;
		}


		uvdata[0] = glm::vec2(0.0, 0.0);
		uvdata[1] = glm::vec2(1.0, 0.0);
		uvdata[2] = glm::vec2(1.0, 1.0);
		uvdata[3] = glm::vec2(0.0, 0.0);
		uvdata[4] = glm::vec2(1.0, 1.0);
		uvdata[5] = glm::vec2(0.0, 1.0);


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[7]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

class Circle :public Object
{
public:
	float angle = 0;
	float angle_count = 0;
	float r = 1.0;
	float x = 0;
	float y = 0;
	float z = 0;
	glm::vec4 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 360);
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 360);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 360);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 360);

		for (int i = 0; i < 360; i++)
		{
			angle = (float)angle_count / 360 * 2 * PI;
			angle_count += 1;
			x = (float)r*cos(angle);
			y = (float)r*sin(angle);
			z = 0.0;
			vertexdata[i] = glm::vec3(x, y, z);
		}

		for (int i = 0; i < 360; i++)
		{
			colordata[i] = glm::vec4(1.0, 1.0, 1.0, 1.0);
		}

		for (int i = 0; i < 360; i++)
		{
			normaldata[i] = glm::vec3(0.0, 0.0, 0.0);
		}

		for (int i = 0; i < 360; i++)
		{
			uvdata[i] = glm::vec2(1.0, 1.0);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void Update(float color_r, float color_g, float color_b)
	{
		for (int i = 0; i < 360; i++)
		{
			colordata[i] = glm::vec4(color_r, color_g, color_b, 1.0);
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 360 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[21]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	}
};

class Board:public Object
{
public:
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	void Initialize()
	{
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 6);

		vertexdata[0] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[1] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[2] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[3] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[4] = glm::vec3(1.0, 1.0, 0.0);
		vertexdata[5] = glm::vec3(0.0, 1.0, 0.0);

		for (int i = 0; i < 6; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.5, 0.0);
		}

		for (int i = 0; i < 6; i++)
		{
			colordata[i] = glm::vec3(1.0, 1.0, 1.0);
		}

		glm::vec3 normal = glm::cross(vertexdata[1] - vertexdata[0], vertexdata[2] - vertexdata[0]);
		for (int i = 0; i < 6; i++)
		{
			normaldata[i] = normal;
		}


		uvdata[0] = glm::vec2(0.0, 0.0);
		uvdata[1] = glm::vec2(1.0, 0.0);
		uvdata[2] = glm::vec2(1.0, 1.0);
		uvdata[3] = glm::vec2(0.0, 0.0);
		uvdata[4] = glm::vec2(1.0, 1.0);
		uvdata[5] = glm::vec2(0.0, 1.0);


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[28]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Draw1(int programID,int number_10)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		switch (number_10) {
		case 0:
			glBindTexture(GL_TEXTURE_2D, texture[29]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 1:
			glBindTexture(GL_TEXTURE_2D, texture[30]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 2:
			glBindTexture(GL_TEXTURE_2D, texture[31]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 3:
			glBindTexture(GL_TEXTURE_2D, texture[32]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 4:
			glBindTexture(GL_TEXTURE_2D, texture[33]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 5:
			glBindTexture(GL_TEXTURE_2D, texture[34]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 6:
			glBindTexture(GL_TEXTURE_2D, texture[35]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 7:
			glBindTexture(GL_TEXTURE_2D, texture[36]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 8:
			glBindTexture(GL_TEXTURE_2D, texture[37]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 9:
			glBindTexture(GL_TEXTURE_2D, texture[38]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		}
	}

	void Draw2(int programID,int number_1)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		switch (number_1) {
		case 0:
			glBindTexture(GL_TEXTURE_2D, texture[29]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 1:
			glBindTexture(GL_TEXTURE_2D, texture[30]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 2:
			glBindTexture(GL_TEXTURE_2D, texture[31]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 3:
			glBindTexture(GL_TEXTURE_2D, texture[32]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 4:
			glBindTexture(GL_TEXTURE_2D, texture[33]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 5:
			glBindTexture(GL_TEXTURE_2D, texture[34]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 6:
			glBindTexture(GL_TEXTURE_2D, texture[35]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 7:
			glBindTexture(GL_TEXTURE_2D, texture[36]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 8:
			glBindTexture(GL_TEXTURE_2D, texture[37]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 9:
			glBindTexture(GL_TEXTURE_2D, texture[38]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		}
	}

	void Draw3(int programID,int wind_dir)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[39]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Draw4(int programID,int wind_speed)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		switch (wind_speed) {
		case 0:
			glBindTexture(GL_TEXTURE_2D, texture[29]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 1:
			glBindTexture(GL_TEXTURE_2D, texture[30]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 2:
			glBindTexture(GL_TEXTURE_2D, texture[31]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 3:
			glBindTexture(GL_TEXTURE_2D, texture[32]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 4:
			glBindTexture(GL_TEXTURE_2D, texture[33]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 5:
			glBindTexture(GL_TEXTURE_2D, texture[34]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 6:
			glBindTexture(GL_TEXTURE_2D, texture[35]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 7:
			glBindTexture(GL_TEXTURE_2D, texture[36]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 8:
			glBindTexture(GL_TEXTURE_2D, texture[37]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		case 9:
			glBindTexture(GL_TEXTURE_2D, texture[38]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			break;
		}
	}

	void Draw5(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[40]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
};

class Cube :public Object
{
public:
	glm::vec3* colordata;
	glm::vec3* normaldata;
	glm::vec2* uvdata;

	void Initialize()
	{
		colordata = (glm::vec3*)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3*)malloc(sizeof(glm::vec3) * 36);
		uvdata = (glm::vec2*)malloc(sizeof(glm::vec2) * 36);

		for (int i = 0; i < 6; i++)
		{
			colordata[i * 6] = glm::vec3((float)(rand() % 11) / 10 , (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
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

		for (int i = 0; i < 6; i++)
		{
			uvdata[i * 6 + 0] = glm::vec2(0.0, 0.0);
			uvdata[i * 6 + 1] = glm::vec2(1.0, 0.0);
			uvdata[i * 6 + 2] = glm::vec2(1.0, 1.0);
			uvdata[i * 6 + 3] = glm::vec2(0.0, 0.0);
			uvdata[i * 6 + 4] = glm::vec2(1.0, 1.0);
			uvdata[i * 6 + 5] = glm::vec2(0.0, 1.0);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec2), uvdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}
	void update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform() * GetmodelTransform()));
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 6, 6);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 12, 6);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 18, 6);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 24, 6);
		glBindTexture(GL_TEXTURE_2D, texture[41]);
		glDrawArrays(GL_TRIANGLES, 30, 6);
	}
};

Snow snow[SNOW_SIZE];
Grass grass[GRASS_SIZE];
CubeMap background;
Arrow arrow;
Circle circle[10];
Line line;
Board board;
Bow bow;
Cube paticle[CUBE_SIZE];

MCI_OPEN_PARMS m_mciOpenParms;
MCI_PLAY_PARMS m_mciPlayParms;
DWORD m_dwDeviceID;
MCI_OPEN_PARMS mciOpen;
MCI_PLAY_PARMS mciPlay;
int dwID;

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.0;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;
float camera_x = 0.2;
float camera_y = 0.2;
float camera_z = 0.6;
glm::vec3 cameraPos{ camera_x, camera_y, camera_z };

bool keybuffer[256] = { 0, };
float Light_R = 1.0f;
float Light_G = 1.0f;
float Light_B = 1.0f;

float x_1 =  0.0;
float y_1 = -1.0;
float z_1 = -0.0;

float t = 0;
float arrow_angle_x = 0;
float arrow_angle_y = 0;
float v = 0;
float arrow_x = 0;
float arrow_z = 0;
float arrow_y = 0;
float pre_arrow_x = 0;
float pre_arrow_z = 0;
float pre_arrow_y = 0;
bool arrow_on = false;
float distance = 0;
float wind_x = 0;
float wind_y = 0;
float wind_z = 0;
float wind_speed = 0.0;
int wind_dir = 0;
int wind_timer = 1000;

bool bezier = false;
float bezier_x1 = 0;
float bezier_x2 = 0;
float bezier_y1 = 0;
float bezier_y2 = 0;
float bezier_z1 = 0;
float bezier_z2 = 0;

int camera_mode = 0;
int score = 0;
int total_score = 0;
int stage = 0;
bool score_on = false;
bool replay = false;
bool pass = false;

float particle_during = 1.0;
float particle_speed = 0;
bool particle_on = false;
int particle_way_x[CUBE_SIZE];
int particle_way_y[CUBE_SIZE];

int number_1 = 0;
int number_10 = 0;

float wind_angle_z = 0;

bool main_loading = true;

GLenum Mode = GL_FILL;

void main(int argc, char** argv)
{
	width = 800;
	height = 800;

	FILE *cubemap = fopen("cube.obj", "rb");
	FILE *arrowfile = fopen("arrow_face_no.obj", "rb");
	FILE *arrowuv = fopen("arrowtest.txt", "rb");
	FILE *bowvertex = fopen("bow_vertex.obj", "rb");
	FILE *bowuv = fopen("bow_uvdata.obj", "rb");
	FILE* cube = fopen("cube.obj", "rb");

	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("test");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)	{		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);	}	else	{		std::cout << "GLEW Initialized\n";	}
	glEnable(GL_DEPTH_TEST);

	background.Readobj(cubemap);
	arrow.Readobj(arrowfile);
	arrow.Readuv(arrowuv);
	bow.Readobj(bowvertex);
	bow.Readuv(bowuv);

	for (int i = 0; i < CUBE_SIZE; ++i)
	{
		paticle[i].Readobj(cube);
	}
	
	InitShader();
	InitBuffer();
	InitTexture();

	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
		snow[i].modelmatrix.position = glm::vec3((float)((float)(rand() % 10001) / 100)-50.0, 65.0, (float)((float)(rand() % 10001) / 100));
	}

	for (int i = 0; i < GRASS_SIZE; i++)
	{
		grass[i].modelmatrix.scale = glm::vec3(8.0, 1.0, 1.0);
		grass[i].modelmatrix.position = glm::vec3((float)((float)(rand() % 10001) / 100) - 50.0, -35.0, (float)((float)(rand() % 10001) / 100));
	}

	for (int i = 0; i < CUBE_SIZE; i++)
	{
		paticle[i].modelmatrix.scale = glm::vec3(0.1, 0.1, 0.1);
		paticle[i].modelmatrix.position = glm::vec3(0.0, 0.0, 90.0);
		particle_way_x[i] = rand() % 360;
		particle_way_y[i]= rand() % 360;
	}

	arrow.modelmatrix.scale = glm::vec3(0.6, 0.6, 0.6);
	arrow.objectmatrix.position = glm::vec3(0.07, 0.0, 0.5);
	background.modelmatrix.scale = glm::vec3(100.0, 100.0, 100.0);
	background.modelmatrix.position = glm::vec3(0.0, 15.0, 45.0);

	stage = 3;	mciOpen.lpstrElementName = "브금2.mp3";
	mciOpen.lpstrDeviceType = "mpegvideo";

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen);

	dwID = mciOpen.wDeviceID;

	mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms);

	for (int i = 0; i < 10; i++)
	{
		circle[i].modelmatrix.position.z += 90.01+0.001*i;
		circle[i].modelmatrix.scale = glm::vec3(i*0.1+0.1, i*0.1+0.1, 1.0);
	}

	line.modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
	board.modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
	board.modelmatrix.rotation.y = 180.0;
	board.modelmatrix.position.z = -0.5;

	Timer(0);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyborad);
	glutKeyboardUpFunc(Keyborad_up);
	glutMouseFunc(Mouse);	glutMotionFunc(Motion);
	glutMouseWheelFunc(mouseWheel);
	glutMainLoop();
}

int make_vertexShaders()
{
	vertexsource = filetobuf("vertex-project.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexsource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return false;
	}
}
int make_fragmentShaders()
{
	fragmentsource = filetobuf("fg-project.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return false;
	}
}

GLuint make_shaderProgram()
{
	GLuint ShaderProgramID;
	ShaderProgramID = glCreateProgram();
	glAttachShader(ShaderProgramID, vertexShader);
	glAttachShader(ShaderProgramID, fragmentShader);
	glLinkProgram(ShaderProgramID);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(ShaderProgramID);
	return ShaderProgramID;
}

GLvoid drawScene()
{
	glViewport(0, 0, 800, 800);
	GLfloat rColor = 0.0f, gColor = 0.0f, bColor = 0.0f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);
	glPolygonMode(GL_FRONT, Mode);

	if (main_loading == false)
	{
		cameratransform = glm::mat4(1.0f);
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(y_angle + 180.0), glm::vec3(0.0, 1.0, 0.0));
		//cameratransform = glm::rotate(cameratransform, (float)glm::radians(180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform = glm::translate(cameratransform, glm::vec3(-camera_x, -camera_y, -camera_z));
		unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(cameratransform));

		glm::mat4 perspect = glm::mat4(1.0f);
		perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

		int cameraPosLocation = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation, 1, glm::value_ptr(cameraPos));
		int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation, x_1, y_1, z_1);
		int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		if (left_button == true && arrow_on == false)
		{
			line.Draw(s_program);
		}

		for (int i = 0; i < 10; i++)
		{
			circle[i].Draw(s_program);
		}


		background.Draw(s_program, stage);

		arrow.Draw(s_program);
		bow.Draw(s_program);

		if (stage == 1)
		{
			for (int i = 0; i < GRASS_SIZE; i++)
			{
				grass[i].Draw(s_program);
			}
		}

		if (stage == 3)
		{
			for (int i = 0; i < SNOW_SIZE; i++)
			{
				snow[i].Draw(s_program);
			}
		}
		
		if (particle_on)
		{
			for (int i = 0; i < CUBE_SIZE; ++i)
			{
				paticle[i].Draw(s_program);
			}
		}


		glViewport(0, 700, 100, 100);
		cameratransform1 = glm::mat4(1.0f);
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation1 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation1, 1, GL_FALSE, glm::value_ptr(cameratransform1));

		glm::mat4 perspect1 = glm::mat4(1.0f);
		perspect1 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect1 = glm::translate(perspect1, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation1 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation1, 1, GL_FALSE, glm::value_ptr(perspect1));

		int cameraPosLocation1 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation1, 1, glm::value_ptr(cameraPos));
		int lightPosLocation1 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation1, x_1, y_1, z_1);
		int lightColorLocation1 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw(s_program);

		glViewport(100, 700, 100, 100);
		cameratransform2 = glm::mat4(1.0f);
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation2 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation2, 1, GL_FALSE, glm::value_ptr(cameratransform2));

		glm::mat4 perspect2 = glm::mat4(1.0f);
		perspect2 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect2 = glm::translate(perspect2, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation2 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation2, 1, GL_FALSE, glm::value_ptr(perspect2));

		int cameraPosLocation2 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation2, 1, glm::value_ptr(cameraPos));
		int lightPosLocation2 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation2, x_1, y_1, z_1);
		int lightColorLocation2 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw1(s_program, number_10);

		glViewport(200, 700, 100, 100);
		cameratransform3 = glm::mat4(1.0f);
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation3 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation3, 1, GL_FALSE, glm::value_ptr(cameratransform3));

		glm::mat4 perspect3 = glm::mat4(1.0f);
		perspect3 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect3 = glm::translate(perspect3, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation3 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation3, 1, GL_FALSE, glm::value_ptr(perspect3));

		int cameraPosLocation3 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation3, 1, glm::value_ptr(cameraPos));
		int lightPosLocation3 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation3, x_1, y_1, z_1);
		int lightColorLocation3 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation3, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw2(s_program, number_1);

		switch (wind_dir) {
		case 1:
			wind_angle_z = -90;
			break;
		case 2:
			wind_angle_z = -45;
			break;
		case 3:
			wind_angle_z = -0;
			break;
		case 4:
			wind_angle_z = 45;
			break;
		case 5:
			wind_angle_z = 90;
			break;
		case 6:
			wind_angle_z = 135;
			break;
		case 7:
			wind_angle_z = 180;
			break;
		case 8:
			wind_angle_z = 225;
			break;
		}
		glViewport(700, 700, 100, 100);
		cameratransform4 = glm::mat4(1.0f);
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(wind_angle_z), glm::vec3(0.0, 0.0, 1.0));
		unsigned int cameraLocation4 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation4, 1, GL_FALSE, glm::value_ptr(cameratransform4));


		glm::mat4 perspect4 = glm::mat4(1.0f);
		perspect4 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect4 = glm::translate(perspect4, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation4 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation4, 1, GL_FALSE, glm::value_ptr(perspect4));

		int cameraPosLocation4 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation4, 1, glm::value_ptr(cameraPos));
		int lightPosLocation4 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation4, x_1, y_1, z_1);
		int lightColorLocation4 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation4, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw3(s_program, wind_dir);

		glViewport(700, 600, 100, 100);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(cameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw4(s_program, wind_speed);
	}
	else if (main_loading)
	{
		glViewport(-100, -100, 1000, 1000);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(cameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw5(s_program);
	}

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

char* filetobuf(const char *file)
{
	char *buf;
	FILE *fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void InitBuffer()
{
	background.Initialize();
	arrow.Initialize();
	line.Initialize();
	board.Initialize();
	bow.Initialize();

	for (int i = 0; i < 10; i++)
	{
		circle[i].Initialize();
	}

	//과녁 위치
	circle[0].Update(1.0, 1.0, 0.7);
	circle[1].Update(1.0, 1.0, 0.0);
	circle[2].Update(1.0, 0.2, 0.2);
	circle[3].Update(1.0, 0.0, 0.0);
	circle[4].Update(0.2, 0.2, 1.0);
	circle[5].Update(0.0, 0.0, 1.0);
	circle[6].Update(0.2, 0.2, 0.2);
	circle[7].Update(0.4, 0.4, 0.4);
	circle[8].Update(0.6, 0.6, 0.6);
	circle[9].Update(0.8, 0.8, 0.8);

	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].Initialize();
	}

	for (int i = 0; i < GRASS_SIZE; i++)
	{
		grass[i].Initialize();
	}

	for (int i = 0; i < CUBE_SIZE; ++i)
	{
		paticle[i].Initialize();
	}
}

void InitTexture()
{
	int widthImage, heightImage, numberOfChannel;
	int widthImage_snow, heightImage_snow, numberOfChannel_snow;
	int widthImage_grass, heightImage_grass, numberOfChannel_grass;
	int widthImage_arctic, heightImage_arctic, numberOfChannel_arctic;
	int widthImage_ocean, heightImage_ocean, numberOfChannel_ocean;
	int widthImage_arrow, heightImage_arrow, numberOfChannel_arrow;
	int widthImage_white, heightImage_white, numberOfChannel_white;
	int widthImage_field, heightImage_field, numberOfChannel_field;
	int widthImage_score, heightImage_score, numberOfChannel_score;
	int widthImage_number, heightImage_number, numberOfChannel_number;
	int widthImage_direction, heightImage_direction, numberOfChannel_direction;
	int widthImage_loading, heightImage_loading, numberOfChannel_loading;
	int widthImage_cube, heightImage_cube, numberOfChannel_cube;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* sky1 = stbi_load("sky1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky2 = stbi_load("sky2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky3 = stbi_load("sky3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky4 = stbi_load("sky4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky5 = stbi_load("sky5.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky6 = stbi_load("sky6.png", &widthImage, &heightImage, &numberOfChannel, 0);

	unsigned char* snow = stbi_load("snow.png", &widthImage_snow, &heightImage_snow, &numberOfChannel_snow, 0);
	unsigned char* grass = stbi_load("grass.png", &widthImage_grass, &heightImage_grass, &numberOfChannel_grass, 0);

	unsigned char* arctic1 = stbi_load("arctic1.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic2 = stbi_load("arctic2.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic3 = stbi_load("arctic3.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic4 = stbi_load("arctic4.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic5 = stbi_load("arctic5.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic6 = stbi_load("arctic6.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);

	unsigned char* ocean1 = stbi_load("ocean1.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean2 = stbi_load("ocean2.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean3 = stbi_load("ocean3.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean4 = stbi_load("ocean4.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean5 = stbi_load("ocean5.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean6 = stbi_load("ocean6.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);

	unsigned char* arrow = stbi_load("arrow_basebolor.png", &widthImage_arrow, &heightImage_arrow, &numberOfChannel_arrow, 0);

	unsigned char* white = stbi_load("white.png", &widthImage_white, &heightImage_white, &numberOfChannel_white, 0);

	unsigned char* field1 = stbi_load("field1.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field2 = stbi_load("field2.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field3 = stbi_load("field3.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field4 = stbi_load("field4.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field5 = stbi_load("field5.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field6 = stbi_load("field6.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);

	unsigned char* score = stbi_load("score.png", &widthImage_score, &heightImage_score, &numberOfChannel_score, 0);

	unsigned char* number0 = stbi_load("0.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number1 = stbi_load("1.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number2 = stbi_load("2.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number3 = stbi_load("3.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number4 = stbi_load("4.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number5 = stbi_load("5.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number6 = stbi_load("6.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number7 = stbi_load("7.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number8 = stbi_load("8.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number9 = stbi_load("9.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);

	unsigned char* direction = stbi_load("direction.png", &widthImage_direction, &heightImage_direction, &numberOfChannel_direction, 0);

	unsigned char* loading = stbi_load("loading.png", &widthImage_loading, &heightImage_loading, &numberOfChannel_loading, 0);

	unsigned char* cube = stbi_load("cube.png", &widthImage_cube, &heightImage_cube, &numberOfChannel_cube, 0);

	
	glGenTextures(42, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky1);
	stbi_image_free(sky1);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky2);
	stbi_image_free(sky2);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky3);
	stbi_image_free(sky3);

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky4);
	stbi_image_free(sky4);

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky5);
	stbi_image_free(sky5);

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky6);
	stbi_image_free(sky6);



	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_snow, heightImage_snow, 0, GL_RGBA, GL_UNSIGNED_BYTE, snow);
	stbi_image_free(snow);

	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_grass, heightImage_grass, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass);
	stbi_image_free(grass);


	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic1);
	stbi_image_free(arctic1);

	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic2);
	stbi_image_free(arctic2);

	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic3);
	stbi_image_free(arctic3);

	glBindTexture(GL_TEXTURE_2D, texture[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic4);
	stbi_image_free(arctic4);

	glBindTexture(GL_TEXTURE_2D, texture[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic5);
	stbi_image_free(arctic5);

	glBindTexture(GL_TEXTURE_2D, texture[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic6);
	stbi_image_free(arctic6);



	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean1);
	stbi_image_free(ocean1);

	glBindTexture(GL_TEXTURE_2D, texture[15]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean2);
	stbi_image_free(ocean2);

	glBindTexture(GL_TEXTURE_2D, texture[16]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean3);
	stbi_image_free(ocean3);

	glBindTexture(GL_TEXTURE_2D, texture[17]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean4);
	stbi_image_free(ocean4);

	glBindTexture(GL_TEXTURE_2D, texture[18]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean5);
	stbi_image_free(ocean5);

	glBindTexture(GL_TEXTURE_2D, texture[19]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean6);
	stbi_image_free(ocean6);



	glBindTexture(GL_TEXTURE_2D, texture[20]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arrow, heightImage_arrow, 0, GL_RGBA, GL_UNSIGNED_BYTE, arrow);
	stbi_image_free(arrow);

	glBindTexture(GL_TEXTURE_2D, texture[21]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_white, heightImage_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	stbi_image_free(white);

	glBindTexture(GL_TEXTURE_2D, texture[22]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field1);
	stbi_image_free(field1);

	glBindTexture(GL_TEXTURE_2D, texture[23]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field2);
	stbi_image_free(field2);

	glBindTexture(GL_TEXTURE_2D, texture[24]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field3);
	stbi_image_free(field3);

	glBindTexture(GL_TEXTURE_2D, texture[25]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field4);
	stbi_image_free(field4);

	glBindTexture(GL_TEXTURE_2D, texture[26]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field5);
	stbi_image_free(field5);

	glBindTexture(GL_TEXTURE_2D, texture[27]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field6);
	stbi_image_free(field6);

	glBindTexture(GL_TEXTURE_2D, texture[28]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_score, heightImage_score, 0, GL_RGBA, GL_UNSIGNED_BYTE, score);
	stbi_image_free(score);

	glBindTexture(GL_TEXTURE_2D, texture[29]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number0);
	stbi_image_free(number0);

	glBindTexture(GL_TEXTURE_2D, texture[30]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number1);
	stbi_image_free(number1);

	glBindTexture(GL_TEXTURE_2D, texture[31]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number2);
	stbi_image_free(number2);

	glBindTexture(GL_TEXTURE_2D, texture[32]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number3);
	stbi_image_free(number3);

	glBindTexture(GL_TEXTURE_2D, texture[33]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number4);
	stbi_image_free(number4);

	glBindTexture(GL_TEXTURE_2D, texture[34]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number5);
	stbi_image_free(number5);

	glBindTexture(GL_TEXTURE_2D, texture[35]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number6);
	stbi_image_free(number6);

	glBindTexture(GL_TEXTURE_2D, texture[36]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number7);
	stbi_image_free(number7);

	glBindTexture(GL_TEXTURE_2D, texture[37]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number8);
	stbi_image_free(number8);

	glBindTexture(GL_TEXTURE_2D, texture[38]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number9);
	stbi_image_free(number9);

	glBindTexture(GL_TEXTURE_2D, texture[39]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_direction, heightImage_direction, 0, GL_RGBA, GL_UNSIGNED_BYTE, direction);
	stbi_image_free(direction);

	glBindTexture(GL_TEXTURE_2D, texture[40]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_loading, heightImage_loading, 0, GL_RGBA, GL_UNSIGNED_BYTE, loading);
	stbi_image_free(loading);

	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_cube, heightImage_cube, 0, GL_BGRA, GL_UNSIGNED_BYTE, cube);
	stbi_image_free(cube);

	glUseProgram(s_program);
	int tLocation = glGetUniformLocation(s_program, "outTexture");
	glUniform1i(tLocation, 0);
}

void InitShader()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(s_program);
}
void Convert_xy(int x, int y){	float w = width;	float h = height;	ox = (float)(x - (float)w / 2.0)*(float)(1.0 / (float)(w / 2.0));	oy = -(float)(y - (float)h / 2.0)*(float)(1.0 / (float)(h / 2.0));}void Timer(int value){	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].modelmatrix.position.y -= snow[i].speed;
		if (snow[i].modelmatrix.position.y <= -15.0)
		{
			snow[i].modelmatrix.position.y = 65.0;
		}
	}	if (arrow_on)	{		if (bezier == false)		{			if (replay)			{				if (arrow_z< 90.0 && arrow_y > -10.0)				{					switch (wind_dir) {					case '0':						wind_x += 0;						wind_y += 0;						wind_z += 0;						break;					case '1':						wind_x += 0;						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case '2':						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case '3':						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += 0;						wind_z += 0;						break;					case '4':						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case '5':						wind_x -= 0;						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case '6':						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case '7':						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += 0;						wind_z += 0;						break;					case '8':						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					}					t += 0.01;					arrow_z = v * cos(arrow_angle_y) * t;					arrow_y = v * sin(arrow_angle_y) * t - (0.5 * 9.8 * t * t);					if (arrow_angle_x != 0)					{						arrow_x = float(arrow_z / tan(arrow_angle_x));					}					else					{						arrow_x = 0;					}					arrow.objectmatrix.position = glm::vec3(arrow_x + wind_x, arrow_y + wind_y, arrow_z + wind_z);					arrow.modelmatrix.rotation.x = -atanf((arrow_y - pre_arrow_y) / (arrow_z - pre_arrow_z))*(180.0 / PI);					arrow.modelmatrix.rotation.y = -atanf((arrow_x - pre_arrow_x) / (arrow_z - pre_arrow_z))*(180.0 / PI);					pre_arrow_x = arrow_x;					pre_arrow_y = arrow_y;					pre_arrow_z = arrow_z;					if (camera_mode == 0)					{						camera_y = arrow_y + wind_y;						camera_z = arrow_z + wind_z;						camera_x = arrow_x + wind_x;					}					else if (camera_mode == 1)					{						camera_z = 88;						camera_y = 0;						camera_x = 0;						y_angle = 180;					}				}				else				{					score = 10;					for (int i = 0; i < 10; i++)					{						if (sqrt(pow(circle[i].modelmatrix.position.x - arrow_x + wind_x, 2.0) + pow(circle[i].modelmatrix.position.y - arrow_y + wind_y, 2.0)) <= (i*0.1 + 0.1))						{							particle_on = true;							mciOpen.lpstrElementName = "jump.wav";
							mciOpen.lpstrDeviceType = "mpegvideo";
							mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
								(DWORD)(LPVOID)&mciOpen);
							dwID = mciOpen.wDeviceID;
							mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&m_mciPlayParms);							if (score > i)							{								score = i;							}						}					}					score_on = true;					replay = false;				}			}			else			{				if (arrow_z < 90.0 && arrow_y > -10.0)				{					switch (wind_dir) {					case 0:						wind_x += 0;						wind_y += 0;						wind_z += 0;						break;					case 1:						wind_x += 0;						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case 2:						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case 3:						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += 0;						wind_z += 0;						break;					case 4:						wind_x -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case 5:						wind_x -= 0;						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case 6:						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y -= (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					case 7:						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += 0;						wind_z += 0;						break;					case 8:						wind_x += (0.0009*1.229*pow(wind_speed, 2.0));						wind_y += (0.0009*1.229*pow(wind_speed, 2.0));						wind_z += 0;						break;					}					t += 0.01;					arrow_z = v * cos(arrow_angle_y) * t;					arrow_y = v * sin(arrow_angle_y) * t - (0.5 * 9.8 * t * t);					if (arrow_angle_x != 0)					{						arrow_x = float(arrow_z / tan(arrow_angle_x));					}					else					{						arrow_x = 0;					}					arrow.objectmatrix.position = glm::vec3(arrow_x + wind_x, arrow_y + wind_y, arrow_z + wind_z);					arrow.modelmatrix.rotation.x = -atanf((arrow_y - pre_arrow_y) / (arrow_z - pre_arrow_z))*(180.0 / PI);					arrow.modelmatrix.rotation.y = atanf((arrow_x - pre_arrow_x) / (arrow_z - pre_arrow_z))*(180.0 / PI);					pre_arrow_x = arrow_x;					pre_arrow_y = arrow_y;					pre_arrow_z = arrow_z;					if (camera_mode == 0)					{						camera_y = arrow_y + wind_y;						camera_z = arrow_z + wind_z;						camera_x = arrow_x + wind_x;					}					else if (camera_mode == 1)					{						camera_z = 88;						camera_y = 0;						camera_x = 0;						y_angle = 180;					}				}				else				{					if (score_on == false)					{						score = 10;						for (int i = 0; i < 10; i++)						{							if (sqrt(pow(circle[i].modelmatrix.position.x - arrow_x+wind_x, 2.0) + pow(circle[i].modelmatrix.position.y - arrow_y+wind_y, 2.0)) <= (i*0.1 + 0.1))							{								particle_on = true;								mciOpen.lpstrElementName = "jump.wav";
								mciOpen.lpstrDeviceType = "mpegvideo";
								mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
									(DWORD)(LPVOID)&mciOpen);
								dwID = mciOpen.wDeviceID;
								mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&m_mciPlayParms);								if (score > i)								{									score = i;								}							}						}						total_score += (10 - score);						score_on = true;					}				}			}		}		else if (bezier)		{			t += 0.002;			if (t <= 1.0)			{				arrow_x = 0.0*(1 - t)*(1 - t)*(1 - t) + 3.0 * bezier_x1*t*(1 - t)*(1 - t) + 3.0*-bezier_x2*t*t*(1 - t) + 0.0 * t*t*t;				arrow_y = 0.0*(1 - t)*(1 - t)*(1 - t) + 3.0 * -bezier_y1 *t*(1 - t)*(1 - t) + 3.0*bezier_y2*t*t*(1 - t) + 0.0 * t*t*t;				arrow_z = 0.0*(1 - t)*(1 - t)*(1 - t) + 3.0 * bezier_z1 *t*(1 - t)*(1 - t) + 3.0*bezier_z2*t*t*(1 - t) + 90.0 * t*t*t;				arrow.objectmatrix.position = glm::vec3(arrow_x, arrow_y, arrow_z);				arrow.modelmatrix.rotation.x = -atanf((arrow_y - pre_arrow_y) / (arrow_z - pre_arrow_z))*(180.0 / PI);				arrow.modelmatrix.rotation.y = atanf((arrow_x - pre_arrow_x) / (arrow_z - pre_arrow_z))*(180.0 / PI);				pre_arrow_x = arrow_x;				pre_arrow_y = arrow_y;				pre_arrow_z = arrow_z;			}			else			{				if(score_on==false)				{					particle_on = true;					mciOpen.lpstrElementName = "jump.wav";
					mciOpen.lpstrDeviceType = "mpegvideo";
					mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
						(DWORD)(LPVOID)&mciOpen);
					dwID = mciOpen.wDeviceID;
					mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&m_mciPlayParms);					total_score += 10;					std::cout << total_score << std::endl;					score_on = true;				}				if (replay)				{					particle_on = true;					mciOpen.lpstrElementName = "jump.wav";
					mciOpen.lpstrDeviceType = "mpegvideo";
					mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
						(DWORD)(LPVOID)&mciOpen);
					dwID = mciOpen.wDeviceID;
					mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&m_mciPlayParms);					std::cout << total_score << std::endl;					replay = false;				}			}			if (camera_mode == 0)			{				camera_y = arrow.objectmatrix.position.y;				camera_z = arrow.objectmatrix.position.z;				camera_x = arrow.objectmatrix.position.x;			}			else if (camera_mode == 1)			{				camera_z = 88;				camera_y = 0;				camera_x = 0;				y_angle = 180;			}		}	}	else	{		if (wind_timer <= 0)		{			if (stage == 0)			{				wind_speed = 0.0;			}			else if (stage == 1)			{				wind_speed = ((float)(rand() % 21) / 10);			}			else if (stage == 2)			{				wind_speed = ((float)(rand() % 41) / 10);			}			else if (stage == 3)			{				wind_speed = ((float)(rand() % 91) / 10);			}			wind_timer = 1000;			wind_dir = rand() % 9;			std::cout << wind_dir << std::endl;		}		else		{			wind_timer -= 1;		}	}	if (particle_on)	{		if (particle_during == 1.0)		{			for (int i = 0; i < CUBE_SIZE; ++i)			{				paticle[i].modelmatrix.position.x = arrow.objectmatrix.position.x;
				paticle[i].modelmatrix.position.y = arrow.objectmatrix.position.y;			}		}		particle_speed += 0.0005;		for (int i = 0; i < CUBE_SIZE; ++i)		{			paticle[i].modelmatrix.position.x+= cos(particle_way_x[i]*PI / 180) * particle_speed;			paticle[i].modelmatrix.position.y+=sin(particle_way_y[i]*PI/180)* particle_speed;			paticle[i].modelmatrix.position.z -= 0.005;		}		particle_during -= 0.01;		if (particle_during < 0)		{			particle_during = 1.0;			particle_on = false;			particle_speed = 0;			for (int i = 0; i < CUBE_SIZE; ++i)			{				paticle[i].modelmatrix.position.x = 0.0;
				paticle[i].modelmatrix.position.y = 0.0;				paticle[i].modelmatrix.position.z = 90.0;			}		}	}	if (total_score>=30 && pass)	{		stage += 1;		total_score = 0;		t = 0;		v = 0;		camera_x = 0.2;
		camera_y = 0.2;
		camera_z = 0.6;		arrow_x = 0;		arrow_y = 0;		arrow_z = 0;		arrow_on = false;		score_on = false;		bezier = false;		pass = false;		arrow.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);		if (stage > 3)		{			stage = 0;		}		mciSendCommand(dwID, MCI_STOP, 0, (DWORD)(LPVOID)&m_mciPlayParms);		switch (stage) {		case 0:			mciOpen.lpstrElementName = "브금2.mp3";
			mciOpen.lpstrDeviceType = "mpegvideo";

			mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
				(DWORD)(LPVOID)&mciOpen);

			dwID = mciOpen.wDeviceID;

			mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms);			break;		case 1:			mciOpen.lpstrElementName = "브금4.mp3";
			mciOpen.lpstrDeviceType = "mpegvideo";

			mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
				(DWORD)(LPVOID)&mciOpen);

			dwID = mciOpen.wDeviceID;

			mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms);			break;		case 2:			mciOpen.lpstrElementName = "브금3.mp3";
			mciOpen.lpstrDeviceType = "mpegvideo";

			mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
				(DWORD)(LPVOID)&mciOpen);

			dwID = mciOpen.wDeviceID;

			mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms);			break;		case 3:			mciOpen.lpstrElementName = "브금5.mp3";
			mciOpen.lpstrDeviceType = "mpegvideo";

			mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
				(DWORD)(LPVOID)&mciOpen);

			dwID = mciOpen.wDeviceID;

			mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&m_mciPlayParms);			break;		}	}	if (left_button)	{		line.Update(v,y_angle,-x_angle * PI / 180.0);	}	if (left_button && arrow_on == false)	{		if (v < 40)		{			v += 0.1;		}		arrow.objectmatrix.position.z = -v * 0.01+0.5;		arrow.objectmatrix.position.x = 0.07;		arrow_angle_y = -x_angle * PI / 180.0;		arrow.modelmatrix.rotation.x = -atanf(arrow_angle_y)*(180.0 / PI);		bow.modelmatrix.rotation.x = -atanf(arrow_angle_y)*(180.0 / PI);	}	else if (left_button == false && arrow_on == false)	{		v = 0;	}	number_1 = total_score%10;
	number_10 = total_score/10;	key_check();	glutTimerFunc(10, Timer, value);	glutPostRedisplay();}void key_check(){	if (keybuffer['w'] == true || keybuffer['W'] == true)	{		camera_z += 0.1;	}	if (keybuffer['a'] == true || keybuffer['A'] == true)	{
		camera_x += 0.1;	}	if (keybuffer['s'] == true || keybuffer['S'] == true)	{		camera_z -= 0.1;	}	if (keybuffer['d'] == true || keybuffer['D'] == true)	{		camera_x -= 0.1;	}	if (keybuffer['z'] == true || keybuffer['Z'] == true)	{		camera_y += 0.1;	}	if (keybuffer['x'] == true || keybuffer['X'] == true)	{		camera_y -= 0.1;	}}GLvoid Keyborad(unsigned char key, int x, int y){	if (GLUT_KEY_DOWN)	{		keybuffer[key] = true;	}	switch (key) {	case 'Q':	case 'q':		if (pass == false)		{			pass = true;			total_score = 30;		}		break;	case 'o':	case 'O':		if (arrow_on == false && left_button)		{			wind_x = 0;			wind_y = 0;			wind_z = 0;			arrow.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);			arrow.modelmatrix.rotation.x = 0;			arrow.modelmatrix.rotation.y = 0;			arrow.modelmatrix.rotation.z = 0;			arrow_angle_y = -x_angle * PI / 180.0;			if (y_angle < 0)			{				y_angle = 180 + y_angle;			}			if (y_angle > 0)			{				y_angle = 90 + y_angle;			}			arrow_angle_x = y_angle * PI / 180.0;			arrow_on = true;			y_angle = 0;			x_angle = 0;		}		break;	case '8':		camera_mode = 0;		break;	case '9':		camera_mode = 1;		break;	case 'm':	case 'M':		if (arrow_on == true)		{			t = 0;			v = 0;			y_angle = 0;			x_angle = 0;			camera_x = 0.2;
			camera_y = 0.2;
			camera_z = 0.6;			arrow_x = 0.07;			arrow_y = 0;			arrow_z = 0.5;			arrow_on = false;			score_on = false;			bezier = false;			arrow.objectmatrix.position = glm::vec3(0.07, 0.0, 0.5);			arrow.modelmatrix.rotation.x = 0;			arrow.modelmatrix.rotation.y = 0;			arrow.modelmatrix.rotation.z = 0;			arrow_angle_y = -x_angle * PI / 180.0;			if (y_angle < 0)			{				y_angle = 180 + y_angle;			}			if (y_angle > 0)			{				y_angle = 90 + y_angle;			}			arrow_angle_x = y_angle * PI / 180.0;		}		break;	case 'N':	case 'n':		y_angle = 0;		x_angle = 0;		camera_x = 0.2;
		camera_y = 0.2;
		camera_z = 0.6;		break;	case 'l':	case 'L':		if (arrow_on == false)		{			bezier_x1 = (((float)(rand() % 1201) / 10) - 60.0);			bezier_x2 = (((float)(rand() % 1201) / 10) - 60.0);			bezier_y1 = (((float)(rand() % 1201) / 10) - 60.0);			bezier_y2 = (((float)(rand() % 1201) / 10) - 60.0);			bezier_z1 = ((float)(rand() % 501) / 10);			bezier_z2 = (((float)(rand() % 201) / 10) + 50.0);			arrow_on = true;			bezier = true;			score_on = false;		}		break;	case 'u':	case 'U':		replay = true;		t = 0;		camera_x = 0.2;
		camera_y = 0.2;
		camera_z = 0.6;		arrow_x = 0.07;		arrow_y = 0;		arrow_z = 0.5;		arrow.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);		break;	case 'p':	case 'P':		if (pass == false)		{			pass = true;		}		break;	case 't':	case 'T':		main_loading = false;		break;	}	glutPostRedisplay();}

GLvoid Keyborad_up(unsigned char key, int x, int y){	if (GLUT_KEY_UP)	{		keybuffer[key] = false;	}	glutPostRedisplay();}

GLvoid Mouse(int button, int state, int x, int y){	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)	{		ox = x;		oy = y;		x_angle = 0;		y_angle = 0;		left_button = true;	}	else	{		ox = 0;		oy = 0;		pre_x_angle = x_angle;		pre_y_angle = y_angle;		left_button = false;	}}GLvoid Motion(int x, int y){	if (left_button)	{		y_angle = x - ox;		x_angle = y - oy;		y_angle /= 8;		x_angle /= 8;		for (int i = 0; i < SNOW_SIZE; i++)		{			snow[i].modelmatrix.rotation.y = -y_angle;		}		for (int i = 0; i < GRASS_SIZE; i++)		{			grass[i].modelmatrix.rotation.y = -y_angle;		}	}	glutPostRedisplay();}GLvoid mouseWheel(int button, int dir, int x, int y){
	if (dir > 0)	{		wheel_scale += dir * 0.1;	}	else if (dir < 0)	{		wheel_scale += dir * 0.1;		if (wheel_scale < 0.1)		{			wheel_scale = 0.1;		}	}	glutPostRedisplay();}
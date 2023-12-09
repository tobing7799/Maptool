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
#include<memory.h>
#define SIZE_ob 4
#define SIZE_pi 4
#define SIZE_follow 4

char* filetobuf(const char *file);
void InitBuffer();
void InitShader();
void Timer(int value);
int make_vertexShaders();
int make_fragmentShaders();
void Convert_xy(int x, int y);
int check();
void key_check();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Motion(int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid mouseWheel(int button, int dir, int x, int y);
GLvoid Reshape(int w, int h);
GLvoid Reshape1(int w, int h);
GLvoid Reshape2(int w, int h);
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

class Object {
public:
	GLuint vao, vbo[3];
	glm::vec3 *vertexdata;
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
};

class Cube :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Obstacle :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Pillar :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	bool state = true;
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot :public Object
{
public:
	bool robot_state = true;
	bool robot_walk = false;
};

class Robot_head :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_body :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_left_arm :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_right_arm :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_left_leg :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_right_leg :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Robot_nose :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot :public Object
{
public:
	bool robot_state = false;
	bool robot_walk = false;
	bool robot_move = false;
};

class Follow_Robot_head :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_body :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_left_arm :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_right_arm :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_left_leg :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_right_leg :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Follow_Robot_nose :public Object
{
	glm::vec3 *colordata;
	glm::vec3 *normaldata;
public:
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
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
		glGenBuffers(3, vbo);

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
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};


glm::vec3 cameraPos{ 0.0,2.0,5.0 };
float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 4.0;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;


float leg_rotate = 1.0;
float arm_rotate = 1.0;

bool jump = false;
int jump_count = 0;
float jump_ability = 0.02;


bool keybuffer[256] = { 0, };
bool on = false;
int check_w = 0;
int check_a = 0;
int check_s = 0;
int check_d = 0;

float Light_R = 1.0;
float Light_G = 1.0;
float Light_B = 1.0;
float Light_x = 0;
float Light_y = 0;
float Light_z = 0;
bool Light_on = true;

typedef struct {
	glm::vec3 robot_move;
	int robot_dir = 0;
	bool robot_walk = false;
}Follow;

float follow_leg_rotate[4] = { 1.0,1.0,1.0,1.0 };
float follow_arm_rotate[4] = { 1.0,1.0,1.0,1.0 };
int move_count = 0;
int robot_count = 0;
int robot_way = 0;

Cube stage;
Obstacle obstacle[SIZE_ob];
Pillar pillar[SIZE_pi];

Robot robot;
Robot_head robot_head;
Robot_body robot_body;
Robot_left_arm robot_left_arm;
Robot_right_arm robot_right_arm;
Robot_left_leg robot_left_leg;
Robot_right_leg robot_right_leg;
Robot_nose robot_nose;

Follow_Robot follow_robot[SIZE_follow];
Follow_Robot_head follow_robot_head[SIZE_follow];
Follow_Robot_body follow_robot_body[SIZE_follow];
Follow_Robot_left_arm follow_robot_left_arm[SIZE_follow];
Follow_Robot_right_arm follow_robot_right_arm[SIZE_follow];
Follow_Robot_left_leg follow_robot_left_leg[SIZE_follow];
Follow_Robot_right_leg follow_robot_right_leg[SIZE_follow];
Follow_Robot_nose follow_robot_nose[SIZE_follow];
Follow follow[200];

GLenum Mode = GL_FILL;

void main(int argc, char** argv)
{
	width = 800;
	height = 800;

	FILE *cu = fopen("cube.obj", "rb");

	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("숙제 2");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)	{		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);	}	else	{		std::cout << "GLEW Initialized\n";	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	for (int i = 0; i < SIZE_ob; i++)
	{
		obstacle[i].Readobj(cu);
	}

	for (int i = 0; i < SIZE_pi; i++)
	{
		pillar[i].Readobj(cu);
	}

	stage.Readobj(cu);
	robot.Readobj(cu);
	robot_body.Readobj(cu);
	robot_left_arm.Readobj(cu);
	robot_right_arm.Readobj(cu);
	robot_left_leg.Readobj(cu);
	robot_right_leg.Readobj(cu);
	robot_nose.Readobj(cu);
	robot_head.Readobj(cu);

	for (int i = 0; i < SIZE_follow; i++)
	{
		follow_robot[i].Readobj(cu);
		follow_robot_body[i].Readobj(cu);
		follow_robot_left_arm[i].Readobj(cu);
		follow_robot_right_arm[i].Readobj(cu);
		follow_robot_left_leg[i].Readobj(cu);
		follow_robot_right_leg[i].Readobj(cu);
		follow_robot_nose[i].Readobj(cu);
		follow_robot_head[i].Readobj(cu);
	}

	InitShader();
	InitBuffer();

	stage.objectmatrix.scale = glm::vec3(5.0, 0.001, 5.0);

	obstacle[0].modelmatrix.rotation = glm::vec3(45, 45, 0.0);
	obstacle[1].modelmatrix.rotation = glm::vec3(45, 45, 0.0);
	obstacle[2].modelmatrix.rotation = glm::vec3(45, 45, 0.0);
	obstacle[3].modelmatrix.rotation = glm::vec3(45, 45, 0.0);

	obstacle[0].modelmatrix.scale = glm::vec3(0.2, 0.2, 0.2);
	obstacle[1].modelmatrix.scale = glm::vec3(0.2, 0.2, 0.2);
	obstacle[2].modelmatrix.scale = glm::vec3(0.2, 0.2, 0.2);
	obstacle[3].modelmatrix.scale = glm::vec3(0.2, 0.2, 0.2);

	obstacle[0].objectmatrix.position = glm::vec3(1.0, 0.2, 2.0);
	obstacle[1].objectmatrix.position = glm::vec3(-1.5, 0.2, 1.0);
	obstacle[2].objectmatrix.position = glm::vec3(-1.0, 0.2, -1.5);
	obstacle[3].objectmatrix.position = glm::vec3(2.0, 0.2, -1.0);

	pillar[0].modelmatrix.position = glm::vec3(-1.5, 0.0, -1.25);
	pillar[1].modelmatrix.position = glm::vec3(1.75, 0.0, -1.25);
	pillar[2].modelmatrix.position = glm::vec3(-1.25, 0.0, 1.75);
	pillar[3].modelmatrix.position = glm::vec3(1.5, 0.0, 1.75);

	pillar[0].modelmatrix.scale = glm::vec3(0.2, 0.4, 0.2);
	pillar[1].modelmatrix.scale = glm::vec3(0.2, 0.4, 0.2);
	pillar[2].modelmatrix.scale = glm::vec3(0.2, 0.4, 0.2);
	pillar[3].modelmatrix.scale = glm::vec3(0.2, 0.4, 0.2);

	robot_head.modelmatrix.scale = glm::vec3(0.12, 0.12, 0.12);
	robot_body.modelmatrix.scale = glm::vec3(0.1, 0.2, 0.05);
	robot_head.modelmatrix.position.y = 0.4;
	robot_nose.modelmatrix.position.y = 0.44;
	robot_nose.modelmatrix.position.z = 0.06;
	robot_body.modelmatrix.position.y = 0.2;
	robot_left_arm.modelmatrix.position.x = -0.07;
	robot_left_arm.modelmatrix.position.y = 0.38;
	robot_right_arm.modelmatrix.position.x = 0.07;
	robot_right_arm.modelmatrix.position.y = 0.38;

	robot_left_leg.modelmatrix.position.x = -0.03;
	robot_left_leg.modelmatrix.position.y = 0.2;
	robot_right_leg.modelmatrix.position.x = 0.03;
	robot_right_leg.modelmatrix.position.y = 0.2;

	robot_left_arm.modelmatrix.rotation.x = 180;
	robot_right_arm.modelmatrix.rotation.x = 180;

	robot_left_leg.modelmatrix.rotation.x = 180;
	robot_right_leg.modelmatrix.rotation.x = 180;

	robot_left_arm.modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
	robot_right_arm.modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
	robot_left_leg.modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
	robot_right_leg.modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
	robot_nose.modelmatrix.scale = glm::vec3(0.01, 0.01, 0.02);

	robot.objectmatrix.position = glm::vec3(0.0,0.0,0.0);


	for (int i = 0; i < SIZE_follow; i++)
	{
		follow_robot_left_arm[i].modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
		follow_robot_right_arm[i].modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
		follow_robot_left_leg[i].modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
		follow_robot_right_leg[i].modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
		follow_robot_nose[i].modelmatrix.scale = glm::vec3(0.01, 0.01, 0.04);

		follow_robot_head[i].modelmatrix.scale = glm::vec3(0.12, 0.12, 0.12);
		follow_robot_body[i].modelmatrix.scale = glm::vec3(0.1, 0.2, 0.05);
		follow_robot_head[i].modelmatrix.position.y = 0.4;
		follow_robot_nose[i].modelmatrix.position.y = 0.44;
		follow_robot_nose[i].modelmatrix.position.z = 0.06;
		follow_robot_body[i].modelmatrix.position.y = 0.2;
		follow_robot_left_arm[i].modelmatrix.position.x = -0.07;
		follow_robot_left_arm[i].modelmatrix.position.y = 0.38;
		follow_robot_right_arm[i].modelmatrix.position.x = 0.07;
		follow_robot_right_arm[i].modelmatrix.position.y = 0.38;

		follow_robot_left_leg[i].modelmatrix.position.x = -0.03;
		follow_robot_left_leg[i].modelmatrix.position.y = 0.2;
		follow_robot_right_leg[i].modelmatrix.position.x = 0.03;
		follow_robot_right_leg[i].modelmatrix.position.y = 0.2;

		follow_robot_left_arm[i].modelmatrix.rotation.x = 180;
		follow_robot_right_arm[i].modelmatrix.rotation.x = 180;

		follow_robot_left_leg[i].modelmatrix.rotation.x = 180;
		follow_robot_right_leg[i].modelmatrix.rotation.x = 180;

		follow_robot[i].objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);

		follow_robot_left_arm[i].modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
		follow_robot_right_arm[i].modelmatrix.scale = glm::vec3(0.02, 0.1, 0.02);
		follow_robot_left_leg[i].modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
		follow_robot_right_leg[i].modelmatrix.scale = glm::vec3(0.02, 0.2, 0.02);
		follow_robot_nose[i].modelmatrix.scale = glm::vec3(0.01, 0.01, 0.04);

		follow_robot[i].objectmatrix.scale = glm::vec3(0.0, 0.0, 0.0);
	}
	
	Light_x = robot.objectmatrix.position.x;
	Light_y = robot.objectmatrix.position.y+1.0;
	Light_z = robot.objectmatrix.position.z;

	robot.objectmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
	std::cout<<"로봇 이동w: 앞 a : 왼쪽 s : 뒤 d : 오른쪽j : 점프 카메라 이동 z : z축 + 방향 x : z축 - 방향 n : x축 + 방향 m : x축 - 방향 c : y축 + 방향회전 v : y축 - 방향회 조명과 그 외 i : 모든 값 초기화 q : 프로그램종료 t : 조명끄기 켜기"<<std::endl;
	Timer(0);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyborad);
	glutKeyboardUpFunc(Keyborad_up);
	glutMouseFunc(Mouse);	glutMotionFunc(Motion);
	glutMouseWheelFunc(mouseWheel);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

int make_vertexShaders()
{
	vertexsource = filetobuf("vertexL.glsl");
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
	fragmentsource = filetobuf("fgL.glsl");
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
	GLfloat rColor = 0.0f, gColor = 0.0f, bColor = 0.0f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);
	glPolygonMode(GL_FRONT, Mode);

	cameratransform = glm::mat4(1.0f);
	cameratransform = glm::rotate(cameratransform, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
	cameratransform = glm::rotate(cameratransform, (float)glm::radians(y_angle), glm::vec3(0.0, 1.0, 0.0));
	cameratransform = glm::translate(cameratransform, cameraPos);
	cameratransform = glm::inverse(cameratransform);
	unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(cameratransform));

	glm::mat4 perspect = glm::mat4(1.0f);
	perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

	int cameraPosLocation = glGetUniformLocation(s_program, "cameraPos");
	glUniform3fv(cameraPosLocation,1, glm::value_ptr(cameraPos));
	int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosLocation, Light_x, Light_y, Light_z);
	int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);

	stage.Draw(s_program);
	for (int i = 0; i < SIZE_ob; i++)
	{
		obstacle[i].Draw(s_program);
	}

	for (int i = 0; i < SIZE_pi; i++)
	{
		if (pillar[i].state == true)
		{
			pillar[i].Draw(s_program);
		}
	}

	for (int i = 0; i < SIZE_follow; i++)
	{
		if (follow_robot[i].robot_state == true)
		{
			follow_robot_body[i].Draw(s_program);
			follow_robot_right_arm[i].Draw(s_program);
			follow_robot_left_arm[i].Draw(s_program);
			follow_robot_right_leg[i].Draw(s_program);
			follow_robot_left_leg[i].Draw(s_program);
			follow_robot_head[i].Draw(s_program);
			follow_robot_nose[i].Draw(s_program);
		}
	}

	robot_body.Draw(s_program);
	robot_right_arm.Draw(s_program);
	robot_left_arm.Draw(s_program);
	robot_right_leg.Draw(s_program);
	robot_left_leg.Draw(s_program);
	robot_head.Draw(s_program);
	robot_nose.Draw(s_program);
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
	stage.Initialize();
	for (int i = 0; i < SIZE_ob; i++)
	{
		obstacle[i].Initialize();
	}

	for (int i = 0; i < SIZE_pi; i++)
	{
		pillar[i].Initialize();
	}

	robot_body.Initialize();
	robot_right_arm.Initialize();
	robot_left_arm.Initialize();
	robot_left_leg.Initialize();
	robot_right_leg.Initialize();
	robot_head.Initialize();
	robot_nose.Initialize();

	robot_body.parent = &robot;
	robot_right_arm.parent = &robot;
	robot_left_arm.parent = &robot;
	robot_left_leg.parent = &robot;
	robot_right_leg.parent = &robot;
	robot_head.parent = &robot;
	robot_nose.parent = &robot;

	for (int i = 0; i < SIZE_follow; i++)
	{
		follow_robot_body[i].Initialize();
		follow_robot_right_arm[i].Initialize();
		follow_robot_left_arm[i].Initialize();
		follow_robot_left_leg[i].Initialize();
		follow_robot_right_leg[i].Initialize();
		follow_robot_head[i].Initialize();
		follow_robot_nose[i].Initialize();

		follow_robot_body[i].parent = &follow_robot[i];
		follow_robot_right_arm[i].parent = &follow_robot[i];
		follow_robot_left_arm[i].parent = &follow_robot[i];
		follow_robot_left_leg[i].parent = &follow_robot[i];
		follow_robot_right_leg[i].parent = &follow_robot[i];
		follow_robot_head[i].parent = &follow_robot[i];
		follow_robot_nose[i].parent = &follow_robot[i];
	}
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
void Convert_xy(int x, int y){	float w = width;	float h = height;	ox = (float)(x - (float)w / 2.0)*(float)(1.0 / (float)(w / 2.0));	oy = -(float)(y - (float)h / 2.0)*(float)(1.0 / (float)(h / 2.0));}void Timer(int value){	if(robot.robot_walk == true)	{		if (robot_left_leg.modelmatrix.rotation.x >= 210)		{			leg_rotate *= -1;		}		else if (robot_left_leg.modelmatrix.rotation.x <= 150)		{			leg_rotate *= -1;		}		robot_left_leg.modelmatrix.rotation.x += leg_rotate;		robot_right_leg.modelmatrix.rotation.x += leg_rotate * -1;		if (robot_left_arm.modelmatrix.rotation.x >= 210)		{			arm_rotate *= -1;		}		else if (robot_left_arm.modelmatrix.rotation.x <= 150)		{			arm_rotate *= -1;		}		robot_left_arm.modelmatrix.rotation.x += arm_rotate * -1;		robot_right_arm.modelmatrix.rotation.x += arm_rotate;	}	else if (robot.robot_walk == false)	{		robot_left_leg.modelmatrix.rotation.x = 180;		robot_right_leg.modelmatrix.rotation.x = 180;		robot_left_arm.modelmatrix.rotation.x = 180;		robot_right_arm.modelmatrix.rotation.x = 180;	}	if (jump == true)	{		robot.objectmatrix.position.y += jump_ability;		jump_count++;		if (jump_count == 24)		{			jump_ability *= -1;		}		if (jump_count == 48)		{			jump_ability *= -1;			jump = false;			jump_count = 0;		}	}	obstacle[0].objectmatrix.rotation.y +=2;
	obstacle[1].objectmatrix.rotation.y -=0.5;
	obstacle[2].objectmatrix.rotation.y +=3;
	obstacle[3].objectmatrix.rotation.y -=1;	if (robot.robot_state == false)	{		if (Light_R <= 0 && Light_G <= 0 && Light_B <= 0)		{			robot.robot_state = true;			robot.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);			robot.objectmatrix.scale = glm::vec3(1.0, 1.0, 1.0);			Light_R = 1.0;			Light_G = 1.0;			Light_B = 1.0;			cameraPos.x = 0.0;			cameraPos.z = 5.0;			x_angle = 0.0;			y_angle = 0.0;			robot_count = 0;			Light_on = true;			robot.robot_walk = false;			for (int i = 0; i < SIZE_follow; i++)			{				follow_robot[i].robot_state = false;			}			for (int i = 0; i < SIZE_pi; i++)			{				pillar[i].state = true;			}			for (int i = 0; i < 200; i++)			{				follow[i].robot_dir = 0;				follow[i].robot_walk = false;				follow[i].robot_move = glm::vec3(0.0, 0.0, 0.0);			}		}		else		{			Light_R -= 0.02;			Light_G -= 0.02;			Light_B -= 0.02;			if(robot.objectmatrix.scale.x>0.0)			{				robot.objectmatrix.scale -= glm::vec3(0.05, 0.05, 0.05);			}		}	}	for (int i = 0; i < SIZE_follow; i++)	{		if (follow_robot[i].robot_state == true && follow_robot[i].objectmatrix.scale.x<1.0 &&follow_robot[i].objectmatrix.scale.y<1.0&&follow_robot[i].objectmatrix.scale.z<1.0)		{			follow_robot[i].objectmatrix.scale.x += 0.05;			follow_robot[i].objectmatrix.scale.y += 0.05;			follow_robot[i].objectmatrix.scale.z += 0.05;		}		else if (follow_robot[i].robot_state == true)		{			follow_robot[i].robot_move = true;		}	}	if (move_count < 200)	{		if (follow[move_count].robot_move != robot.objectmatrix.position)		{			follow[move_count].robot_move = robot.objectmatrix.position;			follow[move_count].robot_walk = robot.robot_walk;			follow[move_count].robot_dir = robot_way;		}		move_count++;	}	else if (move_count == 200)	{		if (follow[0].robot_move != robot.objectmatrix.position)		{			for (int i = 198; i >= 0; i--)			{				follow[i + 1].robot_move = follow[i].robot_move;				follow[i + 1].robot_walk = follow[i].robot_walk;				follow[i + 1].robot_dir = follow[i].robot_dir;			}			follow[0].robot_move = robot.objectmatrix.position;			if (robot.robot_walk == 1)			{				follow[0].robot_walk = true;			}			else if(robot.robot_walk != 1)			{				follow[0].robot_walk = false;			}			follow[0].robot_dir = robot_way;		}	}	for (int i = 0; i < robot_count; i++)	{		if (follow_robot[i].robot_state == true)		{			follow_robot[i].objectmatrix.position = follow[30 * (i + 1)].robot_move;			follow_robot[i].robot_walk= follow[30 * (i + 1)].robot_walk;			switch (follow[30 * (i + 1)].robot_dir) {			case 0:				follow_robot[i].objectmatrix.rotation.y = 180;				break;			case 1:				follow_robot[i].objectmatrix.rotation.y = -90;				break;			case 2:				follow_robot[i].objectmatrix.rotation.y = 0;				break;			case 3:				follow_robot[i].objectmatrix.rotation.y = 90;				break;			case 4:				follow_robot[i].objectmatrix.rotation.y = -135;				break;			case 5:				follow_robot[i].objectmatrix.rotation.y = 135;				break;			case 6:				follow_robot[i].objectmatrix.rotation.y = 45;				break;			case 7:				follow_robot[i].objectmatrix.rotation.y = -45;				break;			}		}	}	for (int i = 0; i < SIZE_follow; i++)	{		if (follow_robot[i].robot_walk == true)		{			if (follow_robot_left_leg[i].modelmatrix.rotation.x >= 210)			{				follow_leg_rotate[i] *= -1;			}			else if (follow_robot_left_leg[i].modelmatrix.rotation.x <= 150)			{				follow_leg_rotate[i] *= -1;			}			follow_robot_left_leg[i].modelmatrix.rotation.x += follow_leg_rotate[i];			follow_robot_right_leg[i].modelmatrix.rotation.x += follow_leg_rotate[i] * -1;			if (follow_robot_left_arm[i].modelmatrix.rotation.x >= 210)			{				follow_arm_rotate[i] *= -1;			}			else if (follow_robot_left_arm[i].modelmatrix.rotation.x <= 150)			{				follow_arm_rotate[i] *= -1;			}			follow_robot_left_arm[i].modelmatrix.rotation.x += follow_arm_rotate[i] * -1;			follow_robot_right_arm[i].modelmatrix.rotation.x += follow_arm_rotate[i];		}		else if (follow_robot[i].robot_walk == false)		{			follow_robot_left_leg[i].modelmatrix.rotation.x = 180;			follow_robot_right_leg[i].modelmatrix.rotation.x = 180;			follow_robot_left_arm[i].modelmatrix.rotation.x = 180;			follow_robot_right_arm[i].modelmatrix.rotation.x = 180;		}	}	check();	key_check();	Light_x = robot.objectmatrix.position.x;
	Light_y = robot.objectmatrix.position.y + 1.0;
	Light_z = robot.objectmatrix.position.z;	glutTimerFunc(20, Timer, value);	glutPostRedisplay();}GLvoid Keyborad(unsigned char key, int x, int y){	if (GLUT_KEY_DOWN)	{		keybuffer[key] = true;	}	switch (key) {	case 't':	case'T':		if (Light_on == true)		{			Light_on = false;			Light_R = 0.0;			Light_G = 0.0;			Light_B = 0.0;		}		else		{			Light_on = true;			Light_R = 1.0;			Light_G = 1.0;			Light_B = 1.0;		}		break;	}	glutPostRedisplay();}

GLvoid Keyborad_up(unsigned char key, int x, int y){	if (GLUT_KEY_UP)	{		keybuffer[key] = false;	}	glutPostRedisplay();}

int check()
{
	for (int i = 0; i < SIZE_ob; i++)
	{
		if ((robot.objectmatrix.position.x - 0.06 < pillar[i].modelmatrix.position.x + 0.1 && robot.objectmatrix.position.x + 0.06 > pillar[i].modelmatrix.position.x - 0.1) &&
			(robot.objectmatrix.position.z - 0.06 < pillar[i].modelmatrix.position.z + 0.1 && robot.objectmatrix.position.z + 0.06 > pillar[i].modelmatrix.position.z - 0.1) &&
			(pillar[i].state==true))
		{
			pillar[i].state = false;
			follow_robot[robot_count].robot_state = true;
			follow_robot[i].objectmatrix.position = pillar[i].modelmatrix.position;
			robot_count++;
			follow[30*robot_count].robot_move = pillar[i].modelmatrix.position;
			follow[30 * robot_count].robot_walk = false;
			return 1;
		}
	}
}

void key_check()
{
	if (robot.robot_state == true)
	{
		if (keybuffer['w'] == true || keybuffer['W'] == true)		{			for (int i = 0; i < SIZE_ob; i++)			{				if (!((robot.objectmatrix.position.x - 0.06 < obstacle[i].objectmatrix.position.x + 0.1 && robot.objectmatrix.position.x + 0.06 > obstacle[i].objectmatrix.position.x - 0.1) &&
					(robot.objectmatrix.position.z - 0.06 - 0.01< obstacle[i].objectmatrix.position.z + 0.1 && robot.objectmatrix.position.z + 0.06 > obstacle[i].objectmatrix.position.z - 0.1)))
				{
					check_w++;
				}
			}			if (check_w == 4)			{				if (robot.objectmatrix.position.z - 0.06 - 0.01 >= -2.5)				{					robot.objectmatrix.position.z -= 0.01;					robot.robot_walk = true;					robot_way = 0;				}			}			else			{				robot.robot_state = false;			}			robot.objectmatrix.rotation.y = 180;			check_w = 0;		}		if (keybuffer['a'] == true || keybuffer['A'] == true)		{
			for (int i = 0; i < SIZE_ob; i++)
			{
				if (!((robot.objectmatrix.position.x - 0.06 - 0.01 < obstacle[i].objectmatrix.position.x + 0.1 && robot.objectmatrix.position.x + 0.06 > obstacle[i].objectmatrix.position.x - 0.1) &&
					(robot.objectmatrix.position.z - 0.06 < obstacle[i].objectmatrix.position.z + 0.1 && robot.objectmatrix.position.z + 0.06 > obstacle[i].objectmatrix.position.z - 0.1)))
				{
					check_a++;
				}
			}
			if (check_a == 4)
			{				if (robot.objectmatrix.position.x - 0.06 - 0.01 >= -2.5)				{					robot.objectmatrix.position.x -= 0.01;					robot.robot_walk = true;					robot_way = 1;				}			}			else			{				robot.robot_state = false;			}			robot.objectmatrix.rotation.y = -90;			check_a = 0;		}		if (keybuffer['s'] == true || keybuffer['S'] == true)		{			for (int i = 0; i < SIZE_ob; i++)			{				if (!((robot.objectmatrix.position.x - 0.06 < obstacle[i].objectmatrix.position.x + 0.1 && robot.objectmatrix.position.x + 0.06 > obstacle[i].objectmatrix.position.x - 0.1) &&
					(robot.objectmatrix.position.z - 0.06 < obstacle[i].objectmatrix.position.z + 0.1 && robot.objectmatrix.position.z + 0.06 + 0.01 > obstacle[i].objectmatrix.position.z - 0.1)))
				{
					check_s++;
				}
			}			if (check_s == 4)			{				if (robot.objectmatrix.position.z + 0.06 + 0.01 <= 2.5)				{					robot.objectmatrix.position.z += 0.01;					robot.robot_walk = true;					robot_way = 2;				}			}			else			{				robot.robot_state = false;			}			robot.objectmatrix.rotation.y = 0;			check_s = 0;		}		if (keybuffer['d'] == true || keybuffer['D'] == true)		{			for (int i = 0; i < SIZE_ob; i++)			{				if (!((robot.objectmatrix.position.x - 0.06 < obstacle[i].objectmatrix.position.x + 0.1 && robot.objectmatrix.position.x + 0.06 + 0.01 > obstacle[i].objectmatrix.position.x - 0.1) &&
					(robot.objectmatrix.position.z - 0.06 < obstacle[i].objectmatrix.position.z + 0.1 && robot.objectmatrix.position.z + 0.06 > obstacle[i].objectmatrix.position.z - 0.1)))
				{
					check_d++;
				}
			}			if (check_d == 4)			{				if (robot.objectmatrix.position.x + 0.06 + 0.01 <= 2.5)				{					robot.objectmatrix.position.x += 0.01;					robot.robot_walk = true;					robot_way = 3;				}			}			else			{				robot.robot_state = false;			}			robot.objectmatrix.rotation.y = 90;			check_d = 0;		}		if ((keybuffer['w'] == true || keybuffer['W'] == true) && (keybuffer['a'] == true || keybuffer['A'] == true))		{			robot.objectmatrix.rotation.y = -135;			robot_way = 4;		}		if ((keybuffer['w'] == true || keybuffer['W'] == true) && (keybuffer['d'] == true || keybuffer['D'] == true))		{			robot.objectmatrix.rotation.y = 135;			robot_way = 5;		}		if ((keybuffer['s'] == true || keybuffer['S'] == true) && (keybuffer['d'] == true || keybuffer['D'] == true))		{			robot.objectmatrix.rotation.y = 45;			robot_way = 6;		}		if ((keybuffer['s'] == true || keybuffer['S'] == true) && (keybuffer['a'] == true || keybuffer['A'] == true))		{			robot.objectmatrix.rotation.y = -45;			robot_way = 7;		}		if (keybuffer['w'] == false && keybuffer['W'] == false && keybuffer['a'] == false && keybuffer['A'] == false && keybuffer['s'] == false && keybuffer['S'] == false && keybuffer['d'] == false && keybuffer['D'] == false)		{			robot.robot_walk = false;		}	}	if (keybuffer['c'] == true || keybuffer['C'] == true)	{		y_angle += 1.0;	}	if (keybuffer['v'] == true || keybuffer['V'] == true)	{		y_angle -= 1.0;	}	if (keybuffer['j'] == true || keybuffer['J'] == true)	{		if (jump == false)		{			jump = true;		}	}	if (keybuffer['i'] == true || keybuffer['I'] == true)	{		robot.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);		robot.objectmatrix.scale = glm::vec3(1.0, 1.0, 1.0);		robot.robot_state = true;		cameraPos.x = 0.0;		cameraPos.z = 5.0;		x_angle = 0.0;		y_angle = 0.0;		Light_R = 1.0;		Light_G = 1.0;		Light_B = 1.0;		robot_count = 0;		Light_on = true;		robot.robot_walk = false;		for (int i = 0; i < SIZE_follow; i++)		{			follow_robot[i].robot_state = false;		}		for (int i = 0; i < SIZE_pi; i++)		{			pillar[i].state = true;		}		for (int i = 0; i < 200; i++)		{			follow[i].robot_dir = 0;			follow[i].robot_walk = false;			follow[i].robot_move = glm::vec3(0.0, 0.0, 0.0);		}	}	if (keybuffer['z'] == true || keybuffer['Z'] == true)	{		cameraPos.z += 0.05;	}	if (keybuffer['x'] == true || keybuffer['X'] == true)	{		cameraPos.z -= 0.05;	}	if (keybuffer['n'] == true || keybuffer['N'] == true)	{		cameraPos.x += 0.05;	}	if (keybuffer['m'] == true || keybuffer['M'] == true)	{		cameraPos.x -= 0.05;	}
	if (keybuffer['q'] == true || keybuffer['Q'] == true)
	{
		glutLeaveMainLoop();
	}
}

GLvoid Mouse(int button, int state, int x, int y){	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)	{		ox = x;		oy = y;		left_button = true;	}	else	{		ox = 0;		oy = 0;		pre_x_angle = x_angle;		pre_y_angle = y_angle;		left_button = false;	}}GLvoid Motion(int x, int y){	if (left_button)	{		y_angle = x - ox;		x_angle = y - oy;		x_angle += pre_x_angle;		y_angle += pre_y_angle;		y_angle /= 2;		x_angle /= 2;	}	glutPostRedisplay();}GLvoid mouseWheel(int button, int dir, int x, int y){
	if (dir < 0)	{		wheel_scale += dir * 0.1;	}	else if (dir > 0)	{		wheel_scale += dir * 0.1;		if (wheel_scale < 0.1)		{			wheel_scale = 0.1;		}	}	glutPostRedisplay();}
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
#define PI 3.141592
#define SIZE 60
#define SIZE_cube 5

char* filetobuf(const char *file);
void InitBuffer();
void InitShader();
void Timer(int value);
int make_vertexShaders();
int make_fragmentShaders();
void Convert_xy(int x, int y);
void InitTexture();
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

glm::mat4 projection;
unsigned int texture[12];

class Object {
public:
	GLuint vao, vbo[4];
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

class Sphere :public Object
{
public:
	glm::vec4 *colordata;
	glm::vec3 *normaldata;
	float speed = (float)(rand() % 9) / 1000 + 0.001;
	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 240);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 240);

		//colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		colordata[0] = glm::vec4(1.0, 1.0, 1.0,1.0);

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
	void Update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform"); //--- 버텍스 세이더에서 모델링 변환 위치 가져오기
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 240);
	}
};

class Line :public Object
{
	glm::vec4 *colordata;
public:
	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 6);
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		vertexdata[0] = glm::vec3(0.0, 1.0, 0.0);
		vertexdata[1] = glm::vec3(0.0, -1.0, 0.0);
		vertexdata[2] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[3] = glm::vec3(-1.0, 0.0, 0.0);
		vertexdata[4] = glm::vec3(0.0, 0.0, 1.0);
		vertexdata[5] = glm::vec3(0.0, 0.0, -1.0);

		colordata[0] = glm::vec4(1.0, 0.0, 0.0,1.0);
		colordata[1] = glm::vec4(1.0, 0.0, 0.0,1.0);
		colordata[2] = glm::vec4(0.0, 1.0, 0.0,1.0);
		colordata[3] = glm::vec4(0.0, 1.0, 0.0,1.0);
		colordata[4] = glm::vec4(0.0, 0.0, 1.0,1.0);
		colordata[5] = glm::vec4(0.0, 0.0, 1.0,1.0);


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES, 0, 6);
	}
};

class Flat :public Object
{
public:
	glm::vec4 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;

	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 36);

		for (int i = 0; i < 6; i++)
		{
			colordata[i * 6] = glm::vec4(1.0, 1.0, 1.0,1.0);
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
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
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
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 6, 6);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 12, 6);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 18, 6);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 24, 6);
		glBindTexture(GL_TEXTURE_2D, texture[8]);
		glDrawArrays(GL_TRIANGLES, 30, 6);
	}
};

class Cube :public Object
{
public:
	glm::vec4 *colordata;
	glm::vec3 *normaldata;

	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 36);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);

		for (int i = 0; i < 6; i++)
		{
			colordata[i * 6] = glm::vec4(1.0, 1.0, 1.0,0.5);
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
	void update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Pyramid :public Object
{
public:
	glm::vec4 *colordata;
	glm::vec3 *normaldata;
	glm::vec2 *uvdata;
	void Initialize()
	{
		colordata = (glm::vec4 *)malloc(sizeof(glm::vec4) * 18);
		normaldata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 18);
		uvdata = (glm::vec2 *)malloc(sizeof(glm::vec2) * 18);

		colordata[0] = glm::vec4(1.0, 1.0, 1.0,1.0);
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
	void Update()
	{
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
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
};

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.5;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;

Flat flat;
Pyramid pyramid;
Cube location;
Sphere snow[SIZE];
Cube cube[SIZE_cube];


float Light_R = 1.0f;
float Light_G = 1.0f;
float Light_B = 1.0f;

float x_1 = 0.0;
float y_1 = 1.0;
float z_1 = 0.0;

float r = 1.5;
float angle = 0;
float angle_count = 90;

bool light_move = false;
bool snowing = false;
bool light_on = true;
int stage = 0;

float ball_r = 1.0;

GLenum Mode = GL_FILL;


void main(int argc, char** argv)
{
	width = 800;
	height = 800;

	FILE *sp = fopen("sphere.obj", "rb");
	FILE *cu = fopen("cube.obj", "rb");
	FILE *py = fopen("pyramid.obj", "rb");

	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("29");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)	{		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);	}	else	{		std::cout << "GLEW Initialized\n";	}
	glEnable(GL_DEPTH_TEST);

	flat.Readobj(cu);
	pyramid.Readobj(py);
	location.Readobj(cu);
	for (int i = 0; i < SIZE; i++)
	{
		snow[i].Readobj(sp);
	}

	for (int i = 0; i < SIZE_cube; i++)
	{
		cube[i].Readobj(cu);
	}

	InitShader();
	InitBuffer();
	InitTexture();

	flat.modelmatrix.scale = glm::vec3(4.0, 0.00001, 4.0);
	pyramid.modelmatrix.scale = glm::vec3(0.5, 0.5, 0.5);
	pyramid.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);
	location.modelmatrix.scale = glm::vec3(0.1, 0.1, 0.1);
	location.objectmatrix.position = glm::vec3(x_1, y_1, z_1);

	for (int i = 0; i < SIZE; i++)
	{
		snow[i].modelmatrix.scale = glm::vec3(0.01, 0.01, 0.01);
		snow[i].objectmatrix.position = glm::vec3(((float)(rand() % 40001) / 10000) - 2.0, 1.0, ((float)(rand() % 40001) / 10000) - 2.0);
	}

	for (int i = 0; i < SIZE_cube; i++)
	{
		cube[i].modelmatrix.scale = glm::vec3(0.1, 0.4, 0.1);
	}

	cube[0].modelmatrix.position = glm::vec3(0.5, 0.2, 0.75);
	cube[1].modelmatrix.position = glm::vec3(-0.75, 0.2, 1.25);
	cube[2].modelmatrix.position = glm::vec3(-0.5, 0.2, -0.5);
	cube[3].modelmatrix.position = glm::vec3(0.75, 0.2, -0.5);
	cube[4].modelmatrix.position = glm::vec3(1.5, 0.2, 1.5);

	Timer(0);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyborad);
	glutMouseFunc(Mouse);	glutMotionFunc(Motion);
	glutMouseWheelFunc(mouseWheel);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

int make_vertexShaders()
{
	vertexsource = filetobuf("vertexA.glsl");
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
	fragmentsource = filetobuf("fgA.glsl");
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

	projection = glm::mat4(1.0f);
	projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
	projection = glm::rotate(projection, (float)glm::radians(x_angle + 30), glm::vec3(1.0, 0.0, 0.0));
	projection = glm::rotate(projection, (float)glm::radians(y_angle), glm::vec3(0.0, 1.0, 0.0));

	unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 perspect = glm::mat4(1.0f);
	perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));


	int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosLocation, x_1, y_1, z_1);
	int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);

	flat.Draw(s_program);
	location.Draw(s_program);
	pyramid.Draw(s_program);
	if (snowing == true)
	{
		for (int i = 0; i < SIZE; i++)
		{
			snow[i].Draw(s_program);
		}
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < SIZE_cube; i++)
	{
		cube[i].Draw(s_program);
	}
	glDisable(GL_BLEND);


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
	flat.Initialize();
	pyramid.Initialize();
	location.Initialize();
	for (int i = 0; i < SIZE; i++)
	{
		snow[i].Initialize();
	}
	for (int i = 0; i < SIZE_cube; i++)
	{
		cube[i].Initialize();
	}
	
}

void InitTexture()
{
	int widthImage, heightImage, numberOfChannel;
	int widthImage1, heightImage1, numberOfChannel1;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data1 = stbi_load("1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* data2 = stbi_load("2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* data3 = stbi_load("3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* data4 = stbi_load("4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* data5 = stbi_load("5.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* data6 = stbi_load("6.png", &widthImage, &heightImage, &numberOfChannel, 0);

	unsigned char* sky1 = stbi_load("sky1.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);
	unsigned char* sky2 = stbi_load("sky2.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);
	unsigned char* sky3 = stbi_load("sky3.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);
	unsigned char* sky4 = stbi_load("sky4.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);
	unsigned char* sky5 = stbi_load("sky5.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);
	unsigned char* sky6 = stbi_load("sky6.png", &widthImage1, &heightImage1, &numberOfChannel1, 0);

	glGenTextures(12, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data3);

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data4);

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data5);

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, data6);

	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky1);

	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky2);

	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky3);

	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky4);
	;
	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky5);

	glBindTexture(GL_TEXTURE_2D, texture[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage1, heightImage1, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky6);

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
void Convert_xy(int x, int y){	float w = width;	float h = height;	ox = (float)(x - (float)w / 2.0)*(float)(1.0 / (float)(w / 2.0));	oy = -(float)(y - (float)h / 2.0)*(float)(1.0 / (float)(h / 2.0));}void Timer(int value){	if (value == 0 && light_move == true)	{		angle_count += 1.0;
		angle = (float)angle_count / 360 * 2 * PI;
		x_1 = (float)r*cos(angle);
		z_1 = (float)r*sin(angle);		location.objectmatrix.position = glm::vec3(x_1, y_1, z_1);	}	if (value == 0 && snowing == true)	{		for (int i = 0; i < SIZE; i++)		{			snow[i].objectmatrix.position.y -= snow[i].speed;			if (snow[i].objectmatrix.position.y < 0.01)			{				snow[i].objectmatrix.position.y = 1.0;			}		}	}	glutTimerFunc(10, Timer, value);	glutPostRedisplay();}GLvoid Keyborad(unsigned char key, int x, int y){	switch (key) {	case 'm':	case 'M':
		if (light_on == false)
		{
			light_on = true;
			Light_R=1.0;
			Light_G=1.0;
			Light_B=1.0;
		}
		else
		{
			light_on = false;
			Light_R = 0.0;
			Light_G = 0.0;
			Light_B = 0.0;
		}
		break;	case 'r':	case 'R':
		if (light_move == false)		{			light_move = true;		}		else		{			light_move = false;		}		break;	case 's':	case 'S':		if (snowing == false)		{			snowing = true;		}		else		{			snowing = false;		}		break;	case 'y':		y_angle += 1;		break;	case 'Y':		y_angle -= 1;		break;	}	glutPostRedisplay();}

GLvoid Mouse(int button, int state, int x, int y){	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)	{		ox = x;		oy = y;		left_button = true;	}	else	{		ox = 0;		oy = 0;		pre_x_angle = x_angle;		pre_y_angle = y_angle;		left_button = false;	}}GLvoid Motion(int x, int y){	if (left_button)	{		y_angle = x - ox;		x_angle = y - oy;		x_angle += pre_x_angle;		y_angle += pre_y_angle;		y_angle /= 2;		x_angle /= 2;	}	glutPostRedisplay();}GLvoid mouseWheel(int button, int dir, int x, int y){
	if (dir > 0)	{		wheel_scale += dir * 0.1;	}	else if (dir < 0)	{		wheel_scale += dir * 0.1;		if (wheel_scale < 0.1)		{			wheel_scale = 0.1;		}	}	glutPostRedisplay();}
#pragma once
#include "stdafx.h"
#include "object.h"
#include "cube.h"
#include "line.h"
#include "flat.h"
#include "pyramid.h"
#include "sphere.h"
#include "arrow.h"

char* filetobuf(const char* file);
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
void key_check();
void ExportObjFile();


GLuint shaderID;
GLuint s_program;
GLchar* vertexsource;
GLuint vertexShader;
GLchar* fragmentsource;
GLuint fragmentShader;

glm::mat4 projection;
glm::mat4 projection1;
glm::mat4 projection2;
unsigned int texture[14];

float ox = 0, oy = 0;
float x_angle = 30;
float y_angle = 180;
float z_angle = 0;
float pre_x_angle = 30;
float pre_y_angle = 180;
float wheel_scale = 0;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 100.0;
float persfect_z = -2.0;

glm::vec3 cameraPosition = glm::vec3(0.0f, -6.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.1;

float camera_move = 0.03;

bool keybuffer[256] = { 0, };

Arrow arrow;
Arrow x_arrow{ glm::vec4(1.0,0.0,0.0,0.7) };
Arrow y_arrow{ glm::vec4(0.0, 1.0, 0.0, 0.7) };
Arrow z_arrow{ glm::vec4(0.0,0.0,1.0,0.7) };

vector<Flat>UI;
vector<Object*> objects;
Line line;

float Light_R = 0.5f;
float Light_G = 0.5f;
float Light_B = 0.5f;

float x_1 = 0.0;
float y_1 = 10.0;
float z_1 = 0.0;

float r = 1.5;
float angle = 0;
float angle_count = 90;

bool light_move = false;
bool light_on = true;
int stage = 0;

bool Select = false;
int selectObject = -1;

bool Grid = true;

float ball_r = 1.0;

GLenum Mode = GL_FILL;

GLint width = 1000;
GLint height = 1000;

float yaw = -90.0f;
float pitch = 0.0f;


FILE* sp = fopen("sphere.obj", "rb");
FILE* cu = fopen("cube.obj", "rb");
FILE* py = fopen("pyramid.obj", "rb");
FILE* ar = fopen("arrow.obj", "rb");
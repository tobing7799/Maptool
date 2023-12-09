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

char* filetobuf(const char *file);
void InitBuffer();
void InitShader();
void Timer(int value);
int make_vertexShaders();
int make_fragmentShaders();
void Convert_xy(int x, int y);
void Inputinfo();
void Make_maze();
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

glm::mat4 projection;

class Object {
public:
	GLuint vao, vbo[2];
	glm::vec3 *vertexdata;
	Transform objectmatrix;
	Transform modelmatrix;
	Object *parent{ nullptr };
	void setquad()
	{
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 6);
		vertexdata[0] = glm::vec3(0.0, 0.0, 0.0);
		vertexdata[1] = glm::vec3(0.0, 0.0, 1.0);
		vertexdata[2] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[3] = glm::vec3(1.0, 0.0, 0.0);
		vertexdata[4] = glm::vec3(0.0, 0.0, 1.0);
		vertexdata[5] = glm::vec3(1.0, 0.0, 1.0);
	}
	void settetra()
	{
		vertexdata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 3);
		vertexdata[0] = glm::vec3(0.5, 0.0, 0.0);
		vertexdata[1] = glm::vec3(0.5, 0.0, 1.0);
		vertexdata[2] = glm::vec3(0.5, 1.0, 0.5);
	}
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
		char count[100];
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
public:
	glm::vec3 *colordata;
	int state = 0;
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
		for (int i = 1; i < 36; i++)
		{
			colordata[i] = colordata[0];
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void update()
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Floor :public Object
{
public:
	glm::vec3 *colordata;
	void Initialize()
	{
		colordata = (glm::vec3 *)malloc(sizeof(glm::vec3) * 36);
		for (int i = 0; i < 36; i++)
		{
			colordata[i] = glm::vec3(1.0, 1.0, 1.0);
		}

		for (int i = 0; i < 36; i++)
		{
			vertexdata[i] -= glm::vec3(0.5, 0.0, 0.5);
		}

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void update()
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
	}
	void Draw(int programID)
	{
		unsigned int modelLocation = glGetUniformLocation(programID, "modelTransform");
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(GetTransform()*GetmodelTransform()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
};

class Maze :public Object
{
};

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.2;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;
float left = -1.0;
float right = 1.0;
float bottom = -1.0;
float top = 1.0;
float near_2 = -3.0;
float far_2 = 3.0;float perspect_ = 2;
bool keybuffer[256] = {0,};

int row = 0;
int column = 0;

float start_x = -1.0;
float start_z = -1.0;
bool start = false;

bool choose_projection = false;

bool up_down_move = false;

int point = 3;

int way = 0;
int pre_way = 0;
int move = 0;
int maze_x = 0;
int maze_y = 0;
float size_x = 0.05;
float size_y = 0.05;
float size_z = 0.05;

bool key_w = false;
bool key_a = false;
bool key_d = false;
bool key_s = false;

float character_speed = 0.002;

int count = 0;
bool automoving = false;
int auto_count = 0;
int auto_count2 = 0;

typedef struct cube_info{
	float max_move;
	float move_speed;
	float maze=0.0;
	bool state = false;
}Cube_info;

Cube_info cube_info[20][20];

int way_data[5000] = {0,};
Cube cube[20][20];
Floor fl;
Maze maze;
Cube character;
GLenum Mode = GL_FILL;

void main(int argc, char** argv)
{
	width = 1000;
	height = 1000;

	FILE *cubeobj = fopen("cube.obj", "rb");


	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(600, 200);
	glutInitWindowSize(width, height);
	glutCreateWindow("2018182013 박동규 숙제 1");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)	{		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);	}	else	{		std::cout << "GLEW Initialized\n";	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	fl.Readobj(cubeobj);
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			cube[i][j].Readobj(cubeobj);
		}
	}
	character.Readobj(cubeobj);

	fl.modelmatrix.scale = glm::vec3(2.0, 0.0, 2.0);
	character.modelmatrix.scale = glm::vec3(size_x, size_y, size_z);
	character.objectmatrix.position = glm::vec3(-0.95, 0.0, -0.95);

	InitShader();
	InitBuffer();

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
	vertexsource = filetobuf("vertexp.glsl");
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
	fragmentsource = filetobuf("fg.glsl");
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
	glViewport(0, 0, 900, 900);
	GLfloat rColor = 0.0f, gColor = 0.0f, bColor = 0.0f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);
	glPolygonMode(GL_FRONT, Mode);
	glEnable(GL_DEPTH_TEST);


	if (point == 3)
	{
		projection = glm::mat4(1.0f);
		projection = glm::rotate(projection, (float)glm::radians(x_angle + 30), glm::vec3(1.0, 0.0, 0.0));
		projection = glm::rotate(projection, (float)glm::radians(y_angle), glm::vec3(0.0, 1.0, 0.0));
		projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
		unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

		if (choose_projection == false)
		{
			glm::mat4 perspect = glm::mat4(1.0f);
			perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
			//perspect = glm::rotate(perspect, (float)glm::radians(perspect_y_angle), glm::vec3(0.0, 1.0, 0.0));
			perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
			unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));
		}
		else if (choose_projection == true)
		{
			glm::mat4 perspect = glm::mat4(1.0f);
			perspect = glm::ortho(left, right, bottom, top, near_2, far_2);
			unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));
		}
	}
	else if (point == 1)
	{
		projection = glm::mat4(1.0f);
		//projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
		projection = glm::rotate(projection, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
		projection = glm::rotate(projection, (float)glm::radians(y_angle+180), glm::vec3(0.0, 1.0, 0.0));
		projection = glm::translate(projection, glm::vec3(-character.objectmatrix.position.x, character.objectmatrix.position.y-0.025,-character.objectmatrix.position.z));
		unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 perspect = glm::mat4(1.0f);
		perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, (float)0.0001, far_1);
		//perspect = glm::rotate(perspect, (float)glm::radians(perspect_y_angle), glm::vec3(0.0, 1.0, 0.0));
		//perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));
	}

	fl.Draw(s_program);

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			if (cube[i][j].state == 1)
			{
				cube[i][j].Draw(s_program);
			}
		}
	}
	if (character.state == 1 && point == 3)
	{
		character.Draw(s_program);
	}

	glViewport(800, 800, 200, 200);
	//projection = glm::mat4(1.0f);
	//projection = glm::rotate(projection, (float)glm::radians(-90.0), glm::vec3(1.0, 0.0, 0.0));
	//projection = glm::scale(projection, glm::vec3(wheel_scale - 0.7, wheel_scale - 0.7, wheel_scale- 0.7));
	projection = glm::mat4(1.0f);
	projection = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	projection = glm::rotate(projection, (float)glm::radians(180.0), glm::vec3(0.0, 1.0, 0.0));
	unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

	//glm::mat4 perspect = glm::mat4(1.0f);
	//perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	//perspect = glm::rotate(perspect, (float)glm::radians(perspect_y_angle), glm::vec3(0.0, 1.0, 0.0));
	//perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
	glm::mat4 perspect = glm::mat4(1.0f);
	perspect = glm::ortho(left, right, bottom, top, near_2, far_2);
	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

	fl.Draw(s_program);

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			if (cube[i][j].state == 1)
			{
				cube[i][j].Draw(s_program);
			}
		}
	}
	if (character.state == 1)
	{
		character.Draw(s_program);
	}

	glutSwapBuffers();
	if (start == false)
	{
		Inputinfo();
		start = true;
		fl.Draw(s_program);

		for (int i = 0; i < row; i++)
		{
			for (int j = 0; j < column; j++)
			{
				if (cube[i][j].state == 1)
				{
					cube[i][j].Draw(s_program);
				}
			}
		}
		glutSwapBuffers();
	}

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
	for (int i = 0; i < 20; i++)
	{
		for (int j = 0; j < 20; j++)
		{
			cube[i][j].Initialize();
			cube[i][j].parent = &maze;
		}
	}
	fl.Initialize();
	character.Initialize();
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
void Convert_xy(int x, int y){	float w = width;	float h = height;	ox = (float)(x - (float)w / 2.0)*(float)(1.0 / (float)(w / 2.0));	oy = -(float)(y - (float)h / 2.0)*(float)(1.0 / (float)(h / 2.0));}void Timer(int value){	if (up_down_move == true && value==1)	{		for (int i = 0; i < row; i++)		{			for (int j = 0; j < column; j++)			{				if (cube[i][j].modelmatrix.scale.y >= cube_info[i][j].max_move)				{					cube_info[i][j].state = true;				}				else if (cube[i][j].modelmatrix.scale.y <= 0.1)				{					cube_info[i][j].state = false;				}				if (cube_info[i][j].state == false)				{					cube[i][j].modelmatrix.scale.y += cube_info[i][j].move_speed;				}				else if (cube_info[i][j].state == true)				{					cube[i][j].modelmatrix.scale.y -= cube_info[i][j].move_speed;				}			}		}		glutTimerFunc(50, Timer, 1);	}	if (automoving == true && value==2)	{		switch (way_data[auto_count]) {		case 1:			character.objectmatrix.position.z+=(cube[0][0].modelmatrix.position.x - cube[0][1].modelmatrix.position.x)/20.0;			auto_count2++;			break;		case 2:			character.objectmatrix.position.z -= (cube[0][0].modelmatrix.position.z - cube[1][0].modelmatrix.position.z) / 20.0;			auto_count2++;			break;		case 3:			character.objectmatrix.position.x -= (cube[0][0].modelmatrix.position.x - cube[0][1].modelmatrix.position.x) / 20.0;			auto_count2++;			break;		case 4:			character.objectmatrix.position.x += (cube[0][0].modelmatrix.position.z - cube[1][0].modelmatrix.position.z) / 20.0;			auto_count2++;			break;		default:			break;		}		if (auto_count2 == 20)		{			auto_count++;			auto_count2 = 0;		}		if (auto_count == count)		{			automoving = false;		}		glutTimerFunc(20, Timer, 2);	}	if (value == 0)	{		key_check();		glutTimerFunc(5, Timer, 0);	}	glutPostRedisplay();}void Inputinfo(){	do	{		std::cout << "가로(5~20) : ";		std::cin >> column;	} while (!(column>=5 && column<=20));	do	{		std::cout << "세로(5~20) : ";		std::cin >> row;	} while (!(row >= 5 && row <= 20));	start_z=-1.0f- (2.0f / (float)row / 2.0f);	for (int i = 0; i < row; i++)
	{
		start_z += (float)(2.0f / (float)row);
		start_x = -1.0f+(2.0f/ (float)column/2.0f);
		for (int j = 0; j < column; j++)
		{
			cube[i][j].state = 1;
			cube[i][j].modelmatrix.scale = glm::vec3(2.0f / (float)column, (float)(rand() % 101) / 100 + 0.01, 2.0f / (float)row);
			cube[i][j].modelmatrix.position = glm::vec3((float)start_x, 0.0, (float)start_z);
			start_x += 2.0f / (float)column;
		}
	}	for (int i = 0; i < row; i++)	{		for (int j = 0; j < column; j++)		{			cube_info[i][j].max_move = cube[i][j].modelmatrix.scale.y+1.0;			cube_info[i][j].move_speed= (float)((float)(rand() % 101) / 1000)+0.01;		}	}	std::cout << "o/O : 직각투영과 원근 투영을 번갈아 가면서 바꾼다."<<std::endl;	std::cout << "m/M : 육면체들이 위 아래로 움직인다/멈춘다. "<<std::endl;	std::cout << "x : x축 기준으로 양의 방향 회전"<<std::endl;	std::cout << "X : x축 기준으로 음의 방향 회전"<<std::endl;	std::cout << "y : y축 기준으로 양의 방향 회전"<<std::endl;	std::cout << "Y : Y축 기준으로 음의 방향 회전"<<std::endl;	std::cout << "r/R : 미로를 제작한다."<<std::endl;	std::cout << "t/T : 미로를 만들때 생긴 순서대로 육면체가 자동으로 이동한다."<<std::endl;	std::cout << "v/V : 육면체들 움직임이 멈추고 낮은 높이로 변한다."<<std::endl;	std::cout << "u/U : 모든 육면체의 색상과 높이를 변경한다."<<std::endl;	std::cout << "w/W : 육면체가 앞으로 이동한다."<<std::endl;	std::cout << "a/A : 육면체가 왼쪽으로 이동한다."<<std::endl;	std::cout << "s/S : 육면체가 뒤로 이동한다."<<std::endl;	std::cout << "d/D : 육면체가 오른쪽으로 이동한다."<<std::endl;	std::cout << "+ : 육면체가 이동하는 속도 증가"<<std::endl;	std::cout << "- : 육면체가 이동하는 속도 감소"<<std::endl;	std::cout << "1 : 카메라 1인침 시점"<<std::endl;	std::cout << "3 : 카메라 3인칭 시점"<<std::endl;	std::cout << "c/C : 모든 값 초기화"<<std::endl;	std::cout << "마우스 클릭 후 회전 : (3인칭)육면체를 회전시켜서 볼 수 있다. (1인칭)주위를 둘러볼 수 있다."<<std::endl;	std::cout << "마우스 휠 : 줌 인/줌 아웃을 할 수 있다."<<std::endl;	std::cout << "q/Q : 프로그램 종료"<<std::endl;}void Make_maze(){	maze_x = 0;	maze_y = 0;	for (int i = 0; i < row; i++)	{		for (int j = 0; j < column; j++)		{			cube_info[i][j].maze = 0;			cube[i][j].state = 1;		}	}		count = 0;	cube_info[0][0].maze = 1.0;	while (1)	{		way = rand() % 8;		move = rand() % 3 + 1;		if (way == 0)		{			if (pre_way == 1 || pre_way == 2 || pre_way == 3)			{				continue;			}		}		else if (way == 1 || way == 2 || way == 3)		{			if (pre_way == 0)			{				continue;			}		}		else if (way == 4 || way == 5 || way == 6)		{			if (pre_way == 7)			{				continue;			}		}		else if (way == 7)		{			if (pre_way == 4 || pre_way == 5 || pre_way == 6)			{				continue;			}		}		switch (way){		case 0:			if (maze_y-1 < 0)			{				continue;			}			else			{				maze_y--;				cube_info[maze_y][maze_x].maze = 1.0;				way_data[count] = 1;				count++;			}			break;		case 1:		case 2:		case 3:			if (maze_y+1 > row-1)			{				continue;			}			else			{				maze_y++;				cube_info[maze_y][maze_x].maze = 1.0;				way_data[count] = 2;				count++;			}			break;		case 4:		case 5:		case 6:			if (maze_x +1 > column - 1)			{				continue;			}			else			{				maze_x++;				cube_info[maze_y][maze_x].maze = 1.0;				way_data[count] = 3;				count++;			}			break;		case 7:			if (maze_x - 1 < 0)			{				continue;			}			else			{				maze_x--;				cube_info[maze_y][maze_x].maze = 1.0;				way_data[count] = 4;				count++;			}			break;		}		pre_way = way;		if (maze_x == column - 1 && maze_y == row - 1)		{			break;		}	}	for (int i = 0; i < row; i++)	{		for (int j = 0; j < column; j++)		{			if (cube_info[i][j].maze == 1.0)			{				cube[i][j].state = 0;			}		}	}	character.objectmatrix.position.x = cube[0][0].modelmatrix.position.x;	character.objectmatrix.position.z = cube[0][0].modelmatrix.position.z;}GLvoid Keyborad(unsigned char key, int x, int y){	if (GLUT_KEY_DOWN)	{		keybuffer[key] = true;	}	switch (key) {	case 'o':	case 'O':		if (choose_projection == false)		{			choose_projection = true;		}		else if (choose_projection == true)		{			choose_projection = false;		}		break;	case 'z':		persfect_z -= 0.1;		break;	case 'Z':		persfect_z += 0.1;		break;	case 'm':	case 'M':		if (up_down_move == false)		{			up_down_move = true;			Timer(1);		}		else if (up_down_move == true)		{			up_down_move = false;		}		break;	case 'x':		x_angle += 2.0;		break;	case 'X':		x_angle -= 2.0;		break;	case 'y':		y_angle += 2.0;		break;	case 'Y':		y_angle -= 2.0;		break;	case 'r':	case 'R':		character.state = 1;		Make_maze();		character.objectmatrix.position.x = cube[0][0].modelmatrix.position.x;		character.objectmatrix.position.z = cube[0][0].modelmatrix.position.z;		automoving = false;
		auto_count = 0;
		auto_count2 = 0;		break;	case 'v':	case 'V':		if (up_down_move == false)		{			up_down_move = true;		}		else if (up_down_move == true)		{			up_down_move = false;		}		for (int i = 0; i < row; i++)		{			for (int j = 0; j < column; j++)			{				cube[i][j].modelmatrix.scale.y = 0.1;			}		}		break;	case '+':		if (character_speed < 0.1)		{			character_speed += 0.001;		}		break;	case '-':		if (character_speed >= 0.001)		{			character_speed -= 0.001;		}		break;	case '1':		if (character.state == 1)		{			point = 1;			x_angle = 0;			y_angle = 0;		}		break;	case '3':		point = 3;		x_angle = 0;		y_angle = 0;		break;		glutLeaveMainLoop();	case 't':	case 'T':		if (automoving == false)		{			automoving = true;			auto_count = 0;			auto_count2 = 0;			character.objectmatrix.position.x = cube[0][0].modelmatrix.position.x;			character.objectmatrix.position.z = cube[0][0].modelmatrix.position.z;			Timer(2);		}		else if (automoving == true)		{			automoving = false;			auto_count = 0;			auto_count2 = 0;			character.objectmatrix.position.x = cube[0][0].modelmatrix.position.x;			character.objectmatrix.position.z = cube[0][0].modelmatrix.position.z;		}			break;	case 'c':	case 'C':		row = 0;
		column = 0;
		start_x = -1.0;
		start_z = -1.0;
		start = false;
		choose_projection = false;
		up_down_move = false;
		point = 3;
		way = 0;
		pre_way = 0;
		move = 0;
		maze_x = 0;
		maze_y = 0;
		size_x = 0.05;
		size_y = 0.05;
		size_z = 0.05;
		key_w = false;
		key_a = false;
		key_d = false;
		key_s = false;		x_angle = 0;
		y_angle = 0;
		z_angle = 0;
		pre_x_angle = 0;
		pre_y_angle = 0;
		wheel_scale = 0.2;
		left_button = 0;
		fovy = 45;
		near_1 = 0.1;
		far_1 = 200.0;
		persfect_z = -2.0;
		left = -1.0;
		right = 1.0;
		bottom = -1.0;
		top = 1.0;
		near_2 = -2.0;
		far_2 = 2.0;		perspect_ = 2;		character.state = false;		character.objectmatrix.position.x = cube[0][0].modelmatrix.position.x;		character.objectmatrix.position.z = cube[0][0].modelmatrix.position.z;		automoving = false;		count = 0;
		auto_count = 0;
		auto_count2 = 0;		break;	case 'u':	case 'U':		for (int i = 0; i < row; i++)		{			for (int j = 0; j < column; j++)			{				cube[i][j].colordata[0] = glm::vec3((float)(rand() % 11) / 10, (float)(rand() % 11) / 10, (float)(rand() % 11) / 10);
				for (int k = 1; k < 36; k++)
				{
					cube[i][j].colordata[k] = cube[i][j].colordata[0];
				}				cube[i][j].modelmatrix.scale.y = (float)(rand() % 101) / 100 + 0.01;				cube[i][j].update();			}		}		break;	case 'q':	case 'Q':		glutLeaveMainLoop();	}	glutPostRedisplay();}

GLvoid Keyborad_up(unsigned char key, int x, int y){	if (GLUT_KEY_UP)	{		keybuffer[key] = false;	}	glutPostRedisplay();}void key_check(){	key_w = true;	key_a = true;	key_s = true;	key_d = true;	if (automoving == false)	{		if (point == 1)		{			if (keybuffer['s'] == true || keybuffer['S'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_w == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 - character_speed <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_w = false;							}						}					}				}				if (character.objectmatrix.position.z - size_z / 2 - character_speed < -1.0)				{					key_w = false;				}				if (key_w == true)				{					character.objectmatrix.position.z -= character_speed;				}			}			if (keybuffer['d'] == true || keybuffer['D'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_a == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 - character_speed <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_a = false;							}						}					}				}				if (character.objectmatrix.position.x - size_x / 2 - character_speed < -1.0)				{					key_a = false;				}				if (key_a == true)				{					character.objectmatrix.position.x -= character_speed;				}			}			if (keybuffer['w'] == true || keybuffer['W'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_s == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 + character_speed >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_s = false;							}						}					}				}				if (character.objectmatrix.position.z + size_z / 2 + character_speed > 1.0)				{					key_s = false;				}				if (key_s == true)				{					character.objectmatrix.position.z += character_speed;				}			}			if (keybuffer['a'] == true || keybuffer['A'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_d == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 + character_speed >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_d = false;							}						}					}				}				if (character.objectmatrix.position.x + size_x / 2 + character_speed > 1.0)				{					key_d = false;				}				if (key_d == true)				{					character.objectmatrix.position.x += character_speed;				}			}		}		else if (point == 3)		{			if (keybuffer['w'] == true || keybuffer['W'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_w == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 - character_speed <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_w = false;							}						}					}				}				if (character.objectmatrix.position.z - size_z / 2 - character_speed < -1.0)				{					key_w = false;				}				if (key_w == true)				{					character.objectmatrix.position.z -= character_speed;				}			}			if (keybuffer['a'] == true || keybuffer['A'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_a == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 - character_speed <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_a = false;							}						}					}				}				if (character.objectmatrix.position.x - size_x / 2 - character_speed < -1.0)				{					key_a = false;				}				if (key_a == true)				{					character.objectmatrix.position.x -= character_speed;				}			}			if (keybuffer['s'] == true || keybuffer['S'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_s == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 + character_speed >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_s = false;							}						}					}				}				if (character.objectmatrix.position.z + size_z / 2 + character_speed > 1.0)				{					key_s = false;				}				if (key_s == true)				{					character.objectmatrix.position.z += character_speed;				}			}			if (keybuffer['d'] == true || keybuffer['D'] == true)			{				for (int i = 0; i < row; i++)				{					for (int j = 0; j < column; j++)					{						if (cube[i][j].state == 1 && key_d == true)						{							if (!((character.objectmatrix.position.x - size_x / 2 <= cube[i][j].modelmatrix.position.x + (float)cube[i][j].modelmatrix.scale.x / 2.0 && character.objectmatrix.position.x + size_x / 2 + character_speed >= cube[i][j].modelmatrix.position.x - (float)cube[i][j].modelmatrix.scale.x / 2.0) &&								(character.objectmatrix.position.z - size_z / 2 <= cube[i][j].modelmatrix.position.z + (float)cube[i][j].modelmatrix.scale.z / 2.0 && character.objectmatrix.position.z + size_z / 2 >= cube[i][j].modelmatrix.position.z - (float)cube[i][j].modelmatrix.scale.z / 2.0)))							{							}							else							{								key_d = false;							}						}					}				}				if (character.objectmatrix.position.x + size_x / 2 + character_speed > 1.0)				{					key_d = false;				}				if (key_d == true)				{					character.objectmatrix.position.x += character_speed;				}			}		}	}}
GLvoid Mouse(int button, int state, int x, int y){	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)	{		ox = x;		oy = y;		left_button = true;	}	else	{		ox = 0;		oy = 0;		pre_x_angle = x_angle;		pre_y_angle = y_angle;		left_button = false;	}}GLvoid Motion(int x, int y){	if (left_button)	{		y_angle = x - ox;		x_angle = y - oy;		x_angle += pre_x_angle;		y_angle += pre_y_angle;		y_angle /= 2;		x_angle /= 2;	}	glutPostRedisplay();}GLvoid mouseWheel(int button, int dir, int x, int y){
	if (dir > 0)	{		wheel_scale += dir * 0.1;	}	else if (dir < 0)	{		wheel_scale += dir * 0.1;		if (wheel_scale < 0.1)		{			wheel_scale = 0.1;		}	}	glutPostRedisplay();}
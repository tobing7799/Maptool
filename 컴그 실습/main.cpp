#pragma once
#include "main.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void main(int argc, char** argv)
{
	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("29");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n";
	}
	glEnable(GL_DEPTH_TEST);

	x_arrow.Readobj(ar);
	y_arrow.Readobj(ar);
	z_arrow.Readobj(ar);

	UI.emplace_back(Flat());
	UI.emplace_back(Flat());

	for (Flat& flat : UI) {
		flat.Readobj(cu);
	}

	InitShader();
	InitBuffer();
	InitTexture();


	for (Flat& flat : UI) {
		flat.modelmatrix.scale = glm::vec3(1.0, 0.001, 1.0);
	}

	Timer(0);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyborad);
	glutKeyboardUpFunc(Keyborad_up);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
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
	glViewport(0, 0, width, height);
	GLfloat rColor = 1.0f, gColor = 1.0f, bColor = 1.0f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);
	glPolygonMode(GL_FRONT, Mode);

	projection = glm::mat4(1.0f);
	//projection = glm::scale(projection, glm::vec3(wheel_scale, wheel_scale, wheel_scale));
	projection = glm::rotate(projection, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
	projection = glm::rotate(projection, (float)glm::radians(y_angle), glm::vec3(0.0, 1.0, 0.0));
	projection = glm::translate(projection, cameraPosition);
	unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 perspect = glm::mat4(1.0f);
	perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	//perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));


	int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosLocation, x_1, y_1, z_1);
	int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);

	int UIstate = glGetUniformLocation(s_program, "UIstate");
	glUniform1i(UIstate, 0);

	int instancingstate = glGetUniformLocation(s_program, "instancestate");
	glUniform1i(instancingstate, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < objects.size(); ++i) {
		objects[i]->Draw(s_program);
		if (i == selectObject) {
			glFrontFace(GL_CW);
			glEnable(GL_CULL_FACE);
			Object* temp;
			if (dynamic_cast<Sphere*>(objects.back())) {
				temp = new Sphere;
				*temp = *objects[i];
				temp->Readobj(sp);
				temp->Initialize();
			}
			else if (dynamic_cast<Cube*>(objects.back())) {
				temp = new Cube;
				*temp = *objects[i];
				temp->Readobj(cu);
				temp->Initialize();
			}
			temp->modelmatrix.scale = (glm::vec3(1.05, 1.05, 1.05));
			if (dynamic_cast<Sphere*>(temp)) dynamic_cast<Sphere*>(temp)->initcolor = glm::vec4(1.0, 1.0, 0.0, 1.0);
			if (dynamic_cast<Cube*>(temp)) dynamic_cast<Cube*>(temp)->initcolor = glm::vec4(1.0, 1.0, 0.0, 1.0);
			temp->Update();
			temp->Draw(s_program);
			glFrontFace(GL_CCW);
			glDisable(GL_CULL_FACE);
			delete temp;
		}
	}

	if (Select) {
		x_arrow.Draw(s_program);
		y_arrow.Draw(s_program);
		z_arrow.Draw(s_program);
	}

	if (Grid) {
		instancingstate = glGetUniformLocation(s_program, "instancestate");
		glUniform1i(instancingstate, 1);

		int instancingoffset = glGetUniformLocation(s_program, "offset");
		glUniform3fv(instancingoffset, line.linenumber, glm::value_ptr(line.positiondata_x[0]));
		line.objectmatrix.rotation.x = 90;
		line.Draw(s_program);


		line.objectmatrix.rotation.x = 0;
		line.objectmatrix.rotation.y = 90;
		instancingoffset = glGetUniformLocation(s_program, "offset");
		glUniform3fv(instancingoffset, line.linenumber, glm::value_ptr(line.positiondata_z[0]));
		line.Draw(s_program);
		line.objectmatrix.rotation.y = 0;
	}

	instancingstate = glGetUniformLocation(s_program, "instancestate");
	glUniform1i(instancingstate, 0);
	glDisable(GL_BLEND);

	glViewport(0, height-100, 200, 100);
	UI[0].collisionbox = { 0,0,200,100 };
	projection1 = glm::mat4(1.0f);
	projection1 = glm::rotate(projection1, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	projection1 = glm::rotate(projection1, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
	unsigned int cameraLocation1 = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation1, 1, GL_FALSE, glm::value_ptr(projection1));

	glm::mat4 perspect1 = glm::mat4(1.0f);
	perspect1 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	perspect1 = glm::translate(perspect1, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation1 = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation1, 1, GL_FALSE, glm::value_ptr(perspect1));

	int lightPosLocation1 = glGetUniformLocation(s_program, "lightPos");
	glUniform3f(lightPosLocation1, x_1, y_1, z_1);
	int lightColorLocation1 = glGetUniformLocation(s_program, "lightColor");
	glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);

	UIstate = glGetUniformLocation(s_program, "UIstate");
	glUniform1i(UIstate, 1);
	UI[0].Draw(s_program, texture[12]);

	glViewport(0, height-200, 200, 100);
	UI[1].collisionbox = { 0,100,200,200 };
	projection2 = glm::mat4(1.0f);
	projection2 = glm::rotate(projection2, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
	projection2 = glm::rotate(projection2, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
	unsigned int cameraLocation2 = glGetUniformLocation(s_program, "cameraTransform");
	glUniformMatrix4fv(cameraLocation2, 1, GL_FALSE, glm::value_ptr(projection2));

	glm::mat4 perspect2 = glm::mat4(1.0f);
	perspect2 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
	perspect2 = glm::translate(perspect2, glm::vec3(0.0, 0.0, persfect_z));
	unsigned int projectionLocation2 = glGetUniformLocation(s_program, "projectionTransform");
	glUniformMatrix4fv(projectionLocation2, 1, GL_FALSE, glm::value_ptr(perspect2));

	UIstate = glGetUniformLocation(s_program, "UIstate");
	glUniform1i(UIstate, 1);
	UI[1].Draw(s_program, texture[13]);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

char* filetobuf(const char* file)
{
	char* buf;
	FILE* fptr = fopen(file, "rb");
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
	for (Flat& flat : UI) {
		flat.Initialize();
		flat.modelmatrix.rotation.x = 270.0;
		flat.modelmatrix.rotation.z = 180.0;
		flat.modelmatrix.position.z -= 0.8;
	}
	x_arrow.Initialize();
	x_arrow.modelmatrix.rotation.y = 90;
	y_arrow.Initialize();
	y_arrow.modelmatrix.rotation.x = -90;
	z_arrow.Initialize();
	z_arrow.modelmatrix.rotation.y = 180;

	line.Initialize();
	line.objectmatrix.rotation.z = 90;
	for (int i = 0; i < line.linenumber; ++i) {
		line.positiondata_x.push_back(glm::vec3((float(line.linenumber / 2) * -line.lineOffsetSize) + (i * line.lineOffsetSize), 0.0, 0.0));
		line.positiondata_z.push_back(glm::vec3(0.0, 0.0, (float(line.linenumber / 2) * -line.lineOffsetSize) + (i * line.lineOffsetSize)));
	}
	line.modelmatrix.scale = glm::vec3(1.0, (float(line.linenumber / 2) * line.lineOffsetSize), 1.0);

	x_arrow.parent = &arrow;
	y_arrow.parent = &arrow;
	z_arrow.parent = &arrow;
}

void InitTexture()
{
	int widthImage, heightImage, numberOfChannel;
	int widthImage1, heightImage1, numberOfChannel1;
	int widthImage2, heightImage2, numberOfChannel2;
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

	glGenTextures(14, texture);
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

	unsigned char* CubeUI = stbi_load("Cubeimage.png", &widthImage2, &heightImage2, &numberOfChannel2, 0);
	glBindTexture(GL_TEXTURE_2D, texture[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage2, heightImage2, 0, GL_RGBA, GL_UNSIGNED_BYTE, CubeUI);

	unsigned char* SphereUI = stbi_load("sphere.png", &widthImage2, &heightImage2, &numberOfChannel2, 0);
	glBindTexture(GL_TEXTURE_2D, texture[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage2, heightImage2, 0, GL_RGBA, GL_UNSIGNED_BYTE, SphereUI);

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

void Convert_xy(int x, int y)
{
	float w = width;
	float h = height;
	ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Timer(int value)
{
	key_check();
	glutTimerFunc(10, Timer, value);
	glutPostRedisplay();
}

GLvoid Keyborad(unsigned char key, int x, int y)
{
	keybuffer[key] = true;
	switch (key) {
	case '8':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.z += 0.5;
			arrow.objectmatrix.position.z += 0.5;
		}
		break;
	case '5':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.z -= 0.5;
			arrow.objectmatrix.position.z -= 0.5;
		}
		break;
	case '6':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.x -= 0.5;
			arrow.objectmatrix.position.x -= 0.5;
		}
		break;
	case '4':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.x += 0.5;
			arrow.objectmatrix.position.x += 0.5;
		}
		break;
	case '7':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.y += 0.5;
			arrow.objectmatrix.position.y += 0.5;
		}
		break;
	case '9':
		if (!objects.empty() && selectObject > -1) {
			objects[selectObject]->objectmatrix.position.y -= 0.5;
			arrow.objectmatrix.position.y -= 0.5;
		}
		break;
	case 'f':
	case 'F':
		if (objects.size()) {
			if (dynamic_cast<Sphere*>(objects.back())) dynamic_cast<Sphere*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
			if (dynamic_cast<Cube*>(objects.back())) dynamic_cast<Cube*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
			objects.back()->Update();
		}
		Select = false;
		selectObject = -1;
		break;
	case'p':
	case 'P':
		ExportObjFile();
		cout << "Export Complete" <<'\n';
		break;
	case 'g':
	case 'G':
		Grid = !Grid;
		break;
	case '1':
		camera_move += 0.01;
		break;
	case '2':
		if (camera_move > 0) camera_move -= 0.01;
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		ox = x;
		oy = y;
		left_button = true;
		cout<<"x좌표 : " << x << " "<< "y좌표 : " << y << endl;
		for (int i = 0; i < UI.size(); ++i) {
			if (UI[i].collisionbox.left < x && UI[i].collisionbox.top < y && UI[i].collisionbox.right > x && UI[i].collisionbox.bottom > y) {
				cout << i << endl;
				switch (i) {
				case 0:
					if (objects.size()) {
						if (dynamic_cast<Sphere*>(objects.back())) dynamic_cast<Sphere*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
						if (dynamic_cast<Cube*>(objects.back())) dynamic_cast<Cube*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
						objects.back()->Update();
					}
					objects.push_back(new Cube(glm::vec4(1.0, 1.0, 1.0, 0.3)));
					objects.back()->Readobj(cu);
					objects.back()->Initialize();
					Select = true;
					selectObject = objects.size() - 1;
					arrow.objectmatrix.position = glm::vec3(0,0,0);
					break;
				case 1:
					if (objects.size()) {
						if (dynamic_cast<Sphere*>(objects.back())) dynamic_cast<Sphere*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
						if (dynamic_cast<Cube*>(objects.back())) dynamic_cast<Cube*>(objects.back())->initcolor = glm::vec4(1.0, 1.0, 1.0, 1.0);
						objects.back()->Update();
					}
					objects.push_back(new Sphere(glm::vec4(1.0, 1.0, 1.0, 0.3)));
					objects.back()->Readobj(sp);
					objects.back()->Initialize();
					Select = true;
					selectObject = objects.size() - 1;
					arrow.objectmatrix.position = glm::vec3(0, 0, 0);
					break;
				}
				break;
			}
		}
	}
	else
	{
		ox = 0;
		oy = 0;
		pre_x_angle = x_angle;
		pre_y_angle = y_angle;
		left_button = false;
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y)
{
	if (left_button)
	{
		y_angle = (x - ox)/4;
		x_angle = (y - oy)/4;
		x_angle += pre_x_angle;
		y_angle += pre_y_angle;

	}
	glutPostRedisplay();
}

GLvoid mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		wheel_scale += dir * 0.1;
	}
	else if (dir < 0)
	{
		wheel_scale += dir * 0.1;
		if (wheel_scale < 0.1)
		{
			wheel_scale = 0.1;
		}
	}
	glutPostRedisplay();
}

void key_check()
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), (float)glm::radians(x_angle), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationMatrix = glm::rotate(rotationMatrix, (float)glm::radians(y_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 right = glm::vec3(rotationMatrix[0][0], rotationMatrix[1][0], rotationMatrix[2][0]);
	glm::vec3 look = glm::vec3(rotationMatrix[0][2], rotationMatrix[1][2], rotationMatrix[2][2]);
	glm::vec3 up = glm::vec3(rotationMatrix[0][1], rotationMatrix[1][1], rotationMatrix[2][1]);

	if (keybuffer['w'] == true || keybuffer['W'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition += (look * (camera_move * 3));
		else cameraPosition += (look * camera_move);
	}

	if (keybuffer['a'] == true || keybuffer['A'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition += (right * (camera_move * 3));
		else cameraPosition += (right * camera_move);
	}

	if (keybuffer['s'] == true || keybuffer['S'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition -= (look * (camera_move * 3));
		else cameraPosition -= (look * camera_move);
	}

	if (keybuffer['d'] == true || keybuffer['D'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition -= (right * (camera_move * 3));
		else cameraPosition -=  (right * camera_move);
	}

	if (keybuffer['q'] == true || keybuffer['Q'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition -= (up * (camera_move * 3));
		else cameraPosition -= (up * camera_move);
	}

	if (keybuffer['e'] == true || keybuffer['E'] == true)
	{
		if (keybuffer['v'] || keybuffer['V']) cameraPosition += (up * (camera_move * 3));
		else cameraPosition += (up * camera_move);
	}
}

GLvoid Keyborad_up(unsigned char key, int x, int y)
{
	keybuffer[key] = false;
	glutPostRedisplay();
}

void ExportObjFile()
{
	ofstream out{ "map.txt" };

	int spherecnt = 0;
	int cubecnt = 0;

	for (Object*& obj : objects) {
		if (dynamic_cast<Sphere*>(obj)) spherecnt++;
		else if (dynamic_cast<Cube*>(obj)) cubecnt++;
	}

	out << "Sphere " << spherecnt <<'\n';
	for (Object*& obj : objects) {
		if (dynamic_cast<Sphere*>(obj)) {
			out << obj->objectmatrix.position.x << ' ' << obj->objectmatrix.position.y << ' ' << obj->objectmatrix.position.z << '\n';
		}
	}

	out << "Cube " << cubecnt << '\n';
	for (Object*& obj : objects) {
		if (dynamic_cast<Cube*>(obj)) {
			out << obj->objectmatrix.position.x << ' ' << obj->objectmatrix.position.y << ' ' << obj->objectmatrix.position.z << '\n';
		}
	}
	out.close();
}

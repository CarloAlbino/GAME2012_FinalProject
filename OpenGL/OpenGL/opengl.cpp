#include <stdio.h>
#include <fstream>
#include <string>
#define GLEW_STATIC
#pragma comment(lib, "glew32s.lib")
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "graphicsmath.h"
#include "transformations.h"

using namespace std;

// For Right Paddle
GLuint VBO;
GLuint IBO;
// For Left Paddle
GLuint VBO2;
GLuint IBO2;
// For Ball
GLuint VBO3;
GLuint IBO3;

//Memory position of World Matrix on the GPU
GLuint gWorldLocation_RightPaddle;
GLuint gWorldLocation_LeftPaddle;
GLuint gWorldLocation_Ball;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

static bool ReadFile(const char* pFileName, string& outFile)
{
	ifstream f(pFileName);
	bool ret = false;

	if (f.is_open())
	{
		string line;
		while (getline(f, line))
		{
			outFile.append(line);
			outFile.append("\n");
		}
		f.close();
		ret = true;
	}
	else
	{
		fprintf(stderr, "unable to open file '%s'\n", pFileName);
	}

	return ret;
}

static void RenderSceneCB()
{
	glClear(GL_COLOR_BUFFER_BIT);

	//Static scale variable
	static float Scale = 0.0f;

	static float Rotation = 0.0f;

	Transform transform;
	transform.Position(0.0f, 0.0f, 5.0f);
	transform.Rotation(0.0f, 0.0f, 0.0f);
	transform.SetPerspective(50.f, 1024, 768, 1.0f, 1000.0f);

	//Right Paddle
	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocation_RightPaddle, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//Draw the indices for the right paddle
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

	// Left Paddle
	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocation_LeftPaddle, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);
	//Draw the indices for the left paddle
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

	// Ball
	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocation_Ball, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO3);
	//Draw the indices for the ball
	glDrawElements(GL_TRIANGLE_FAN, 18, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);

	glutSwapBuffers();
}

static void AddShader(GLuint ShaderProgram, const char* ShaderText,
	GLenum ShaderType)
{
	//Create the shader for storage
	//GLuint used for communication with GPU
	//Position on GPU
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar* ShaderSource[1];
	ShaderSource[0] = ShaderText;
	GLint lengths[1];
	lengths[0] = strlen(ShaderText);
	//Provide GL with shader source
	glShaderSource(ShaderObj, 1, ShaderSource, lengths);
	//Compile shader using shader source (stored in ShaderObj)
	glCompileShader(ShaderObj);

	//Confirm shader compiled successfully
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, infoLog);
		fprintf(stderr, "Error compiling shader type: %d: '%s'",
			ShaderType, infoLog);
	}

	//Attach shader to shader program (collection of shaders)
	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
	//Create shader program (collection of shaders)
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0)
	{
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs;
	string fs;
	//Read vertex shader from file system
	if (!ReadFile(pVSFileName, vs))
	{
		exit(1);
	}
	//Read fragment shader from file system
	if (!ReadFile(pFSFileName, fs))
	{
		exit(1);
	}
	//Compile vertex and fragment shaders and add to the shader program
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint success = 0;
	GLchar errorLog[1024] = { 0 };
	//Tell OpenGL about the shader program and confirm its status
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
	if (success == 0)
	{
		glGetProgramInfoLog(ShaderProgram, sizeof(errorLog),
			NULL, errorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", 
			errorLog);
		exit(1);
	}
	//Ensure shader program is valid, and confirm status
	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == 0)
	{
		glGetProgramInfoLog(ShaderProgram, sizeof(errorLog),
			NULL, errorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n",
			errorLog);
		exit(1);
	}
	//Tell OpenGL to use this shader
	glUseProgram(ShaderProgram);

	//Allocate and get memory location of World Matrix on GPU
	gWorldLocation_RightPaddle = glGetUniformLocation(ShaderProgram, "gWorld");
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
	// Right Paddle
	Vector3f RightPaddle_Verts[4];
	RightPaddle_Verts[0] = Vector3f(1.9f, 1.9f, 0.0f);	// Top right
	RightPaddle_Verts[1] = Vector3f(1.9f, 1.25f, 0.0f);	// Bottom right
	RightPaddle_Verts[2] = Vector3f(1.8f, 1.9f, 0.0f);	// Top left
	RightPaddle_Verts[3] = Vector3f(1.8f, 1.25f, 0.0f);	// Bottom left

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RightPaddle_Verts), RightPaddle_Verts, GL_STATIC_DRAW);

	// Left Paddle
	Vector3f LeftPaddle_Verts[4];
	LeftPaddle_Verts[0] = Vector3f(-1.9f, -1.9f, 0.0f);	// Bottom left
	LeftPaddle_Verts[1] = Vector3f(-1.9f, -1.25f, 0.0f);// Top left
	LeftPaddle_Verts[2] = Vector3f(-1.8f, -1.9f, 0.0f);	// Bottom right
	LeftPaddle_Verts[3] = Vector3f(-1.8f, -1.25f, 0.0f);// Top right

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(LeftPaddle_Verts), LeftPaddle_Verts, GL_STATIC_DRAW);

	// Ball
	Vector3f Ball_Verts[17];
	Ball_Verts[0] = Vector3f(0.2f, 0.2f, 0.0f);		// Center
	Ball_Verts[1] = Vector3f(0.2f, 0.29f, 0.0f);	// Top
	Ball_Verts[2] = Vector3f(0.23f, 0.28f, 0.0f);	 
	Ball_Verts[3] = Vector3f(0.28f, 0.23f, 0.0f);
	Ball_Verts[4] = Vector3f(0.29f, 0.2f, 0.0f);	// Right
	Ball_Verts[5] = Vector3f(0.27f, 0.15f, 0.0f);
	Ball_Verts[6] = Vector3f(0.24f, 0.11f, 0.0f);
	Ball_Verts[7] = Vector3f(0.2f, 0.11f, 0.0f);	// Bottom
	Ball_Verts[8] = Vector3f(0.18f, 0.11f, 0.0f);
	Ball_Verts[9] = Vector3f(0.11f, 0.18f, 0.0f);
	Ball_Verts[10] = Vector3f(0.11f, 0.2f, 0.0f);	// Left
	Ball_Verts[11] = Vector3f(0.11f, 0.21f, 0.0f);
	Ball_Verts[12] = Vector3f(0.18f, 0.29f, 0.0f);
	Ball_Verts[13] = Vector3f(0.26f, 0.26f, 0.0f);
	Ball_Verts[14] = Vector3f(0.26f, 0.13f, 0.0f);	 
	Ball_Verts[15] = Vector3f(0.13f, 0.13f, 0.0f);
	Ball_Verts[16] = Vector3f(0.13f, 0.26f, 0.0f);

	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Ball_Verts), Ball_Verts, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	// Right Paddle
	//List of Indices to draw
	unsigned int Indices[] = {
		0, 1, 2,
		2, 1, 3,
	};
	//Generate Index Buffer
	glGenBuffers(1, &IBO);
	//Bind buffer to GL Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//Send Indices to GL Element Array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Left Paddle
	//List of Indices to draw
	unsigned int Indices2[] = {
		0, 1, 2,
		2, 1, 3,
	};
	//Generate Index Buffer
	glGenBuffers(1, &IBO2);
	//Bind buffer to GL Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);
	//Send Indices to GL Element Array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices2), Indices2, GL_STATIC_DRAW);

	// Ball
	//List of Indices to draw
	unsigned int Indices3[] = {
		0, 1, 2, 13,
		3, 4, 5, 14,
		6, 7, 8, 15,
		9, 10, 11, 16,
		12, 1
	};
	//Generate Index Buffer
	glGenBuffers(1, &IBO3);
	//Bind buffer to GL Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO3);
	//Send Indices to GL Element Array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices3), Indices3, GL_STATIC_DRAW);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 768);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tutorial 03");

	InitializeGlutCallbacks();

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	CreateVertexBuffer();

	CreateIndexBuffer();

	CompileShaders();

	glutMainLoop();

	return 0;
}
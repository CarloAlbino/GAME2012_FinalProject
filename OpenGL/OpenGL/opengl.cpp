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

GLuint VBO;
GLuint IBO;

GLuint VBO2;
GLuint IBO2;

GLuint VBO3;
GLuint IBO3;

//Memory position of World Matrix on the GPU
GLuint gWorldLocation;
GLuint gWorldLocation2;
GLuint gWorldLocationBall;

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

	//Scale += 0.1f;

	static float Rotation = 0.0f;

	//Rotation += 0.008f;

	Transform transform;
	transform.Position(0.0f, 0.0f, 5.0f);
	transform.Rotation(0.0f, 0.0f, 0.0f);
	transform.SetPerspective(50.f, 1024, 768, 1.0f, 1000.0f);

	// sinf(Scale * 300000.f)

	//Right Paddle
	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//Draw the indices
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

	//////////

	// Left Paddle
	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocation2, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);
	//Draw the indices
	glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);

	//////////
	// Ball

	//Update world matrix value on GPU
	glUniformMatrix4fv(gWorldLocationBall, 1, GL_TRUE, (const GLfloat*)transform.GetWorldPerspectiveTransformation());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//Bind the Index buffer to the pipeline
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO3);
	//Draw the indices
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
	gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
	glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
	/*Vector3f Vertices[4];
	Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
	Vertices[1] = Vector3f(-1.0f, 1.0f, 0.0f);
	Vertices[2] = Vector3f(1.0f, 1.0f, 0.0f);
	Vertices[3] = Vector3f(-1.0f, 1.0f, 0.0f);*/

	Vector3f Vertices[4];
	Vertices[0] = Vector3f(1.9f, 1.9f, 0.0f);
	Vertices[1] = Vector3f(1.9f, 1.25f, 0.0f);
	Vertices[2] = Vector3f(1.8f, 1.9f, 0.0f);
	Vertices[3] = Vector3f(1.8f, 1.25f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	//////

	Vector3f Vertices2[4];
	Vertices2[0] = Vector3f(-1.9f, -1.9f, 0.0f);
	Vertices2[1] = Vector3f(-1.9f, -1.25f, 0.0f);
	Vertices2[2] = Vector3f(-1.8f, -1.9f, 0.0f);
	Vertices2[3] = Vector3f(-1.8f, -1.25f, 0.0f);

	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);

	////////

	Vector3f Vertices3[17];
	Vertices3[0] = Vector3f(0.2f, 0.2f, 0.0f);	 // Center
	Vertices3[1] = Vector3f(0.2f, 0.29f, 0.0f);	 // Top
	Vertices3[2] = Vector3f(0.23f, 0.28f, 0.0f);	 
	Vertices3[3] = Vector3f(0.28f, 0.23f, 0.0f);
	Vertices3[4] = Vector3f(0.29f, 0.2f, 0.0f);	 // Right
	Vertices3[5] = Vector3f(0.27f, 0.15f, 0.0f);
	Vertices3[6] = Vector3f(0.24f, 0.11f, 0.0f);
	Vertices3[7] = Vector3f(0.2f, 0.11f, 0.0f);	 // Down
	Vertices3[8] = Vector3f(0.18f, 0.11f, 0.0f);
	Vertices3[9] = Vector3f(0.11f, 0.18f, 0.0f);
	Vertices3[10] = Vector3f(0.11f, 0.2f, 0.0f);	 // Left
	Vertices3[11] = Vector3f(0.11f, 0.21f, 0.0f);
	Vertices3[12] = Vector3f(0.18f, 0.29f, 0.0f);
	// Extra verts
	Vertices3[13] = Vector3f(0.26f, 0.26f, 0.0f);
	Vertices3[14] = Vector3f(0.26f, 0.13f, 0.0f);	 
	Vertices3[15] = Vector3f(0.13f, 0.13f, 0.0f);
	Vertices3[16] = Vector3f(0.13f, 0.26f, 0.0f);

	glGenBuffers(1, &VBO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices3), Vertices3, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	/*//List of Indices to draw
	unsigned int Indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};*/
	//List of Indices to draw
	unsigned int Indices[] = {
		0, 1, 2,
		2, 1, 3,
		2, 2, 2,
		2, 2, 2
	};
	//Generate Index Buffer
	glGenBuffers(1, &IBO);
	//Bind buffer to GL Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	//Send Indices to GL Element Array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	//List of Indices to draw
	unsigned int Indices2[] = {
		0, 1, 2,
		2, 1, 3,
		2, 2, 2,
		2, 2, 2
	};
	//Generate Index Buffer
	glGenBuffers(1, &IBO2);
	//Bind buffer to GL Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO2);
	//Send Indices to GL Element Array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices2), Indices2, GL_STATIC_DRAW);

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
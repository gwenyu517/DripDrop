#include <stdio.h>
#include <stdlib.h>
#include <vector>

//GLEW must be included before GLUT or GLFW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "shader.h"
#include "System.h"

const char* title = "Waaaaa...wo si la";
GLFWwindow* window;
int width = 1024;
int height = 1024;
const char* vertexShaderFile = "SimpleVertexShader.vertexshader";
const char* fragmentShaderFile = "SimpleFragmentShader.fragmentshader";

double prevTime;
double currTime;

GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint elementbuffer;
GLuint textureID;
GLuint textureName;
GLuint MatrixID;

static const GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,		1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,		0.0f, 1.0f
};
//std::vector<unsigned short> indices = {
static const unsigned short indices[] = {
		0,1,2,
		0,2,3
};

static GLubyte pixels_test[] = {
		255, 0, 0, 		0, 255, 0,
		0, 0, 255, 		255, 255, 0
};

/*static GLfloat pixels_test[] = {
		0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
};
*/

// NOT ENTIRELY SURE WHAT TO DO WITH THESE VARIABLES BUT LIKE YEAH
float system_width = 0.3f;
float system_height = 0.3f;
float system_gridlength = 0.01f;
//float* system_heightMap;

glm::mat4 generateMVPmatrix() {
	// Projection matrix : 45 deg field of view, width:height ratio, display range 0.1 to 100 units
	glm::mat4 Projection = glm::perspective(
			glm::radians(45.0f),			// vertical field of view, usually between 90 and 30 deg
			(float)width / (float)height,	// aspect ratio
			0.1f,							// near clipping plane (keep as big as possible)
			100.0f							// far clipping plane (keep as little as possible)
	);

	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
			glm::vec3(0,0,3),	// Camera at (4,3,3) in world space
			glm::vec3(0,0,0),	// looks at origin
			glm::vec3(0,1,0)	// head is up
	);

	// Model matrix : an identity matrix (model will be at origin)
	glm::mat4 Model = glm::mat4(1.0f);

	// Model-View-Projection matrix
	return Projection * View * Model;
}

void createVertexBuffer() {
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);		// associate any VBO/IBO that you bind with this VAO

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
			vertices, GL_STATIC_DRAW);
}

void setUpVertexAttributes() {
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
			0,			// attribute
			3, 			// size
			GL_FLOAT, 	// type
			GL_FALSE, 	// normalized?
			5 * sizeof(GLfloat), 			// stride
			(void*)0	// array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
			1,			// attribute
			2, 			// size
			GL_FLOAT, 	// type
			GL_FALSE, 	// normalized?
			5 * sizeof(GLfloat), 			// stride
			(void*)(3*sizeof(GLfloat))	// array buffer offset
	);
}
void createElementBuffer() {
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, elementbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short),
			//&indices[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices),
				indices, GL_STATIC_DRAW);
}

void createTexture() {
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
			GL_TEXTURE_2D,		// target
			0, 					// level
			GL_R32F,					// internal format
			(int)(system_width/system_gridlength),					// width
			(int)(system_height/system_gridlength),					// height
			0,					// border
			GL_RED,					// format
			GL_FLOAT,			// type
			0					// null data --> reserves memory
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void cleanUp() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteBuffers(1, &elementbuffer);
}

void render(float* data) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(
			GL_TEXTURE_2D,		// target
			0,					// level
			0,					// xoffset
			0,					// yoffset
			(int)(system_width/system_gridlength),	// width
			(int)(system_height/system_gridlength),	// height
			GL_RED,				// format
			GL_FLOAT,			// type
			data
	);
	//glUniform1i(textureName, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

/*	glDrawElements(
		GL_TRIANGLES,      // mode
		//indices.size(),    // count
		sizeof(indices),
		GL_UNSIGNED_SHORT,   // type
		(void*)0           // element array buffer offset
	);
*/
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

int main() {

// initialize GLFW
	if(!glfwInit()) {
			fprintf(stderr, "Failed to initialize GLFW\n");
			return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);	// 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// for MacOS, should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// don't want old OpenGL

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if(window == NULL ) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed later
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// width, height = 0.3
	// grid length = 0.01
	System* test = new System(system_width, system_height, system_gridlength);

	createVertexBuffer();
	setUpVertexAttributes();
	createTexture();
	createElementBuffer();
	GLuint programID = LoadShaders(vertexShaderFile, fragmentShaderFile);

	//----------- Perspective stuff ----------------
	//glm::mat4 mvp = generateMVPmatrix();
	// get handle for "MVP" uniform -- only during initialization
	//MatrixID = glGetUniformLocation(programID, "MVP");

	textureName = glGetUniformLocation(programID, "myTexture");
	glUniform1i(textureName, 0);

	prevTime = glfwGetTime();

	do {
		currTime = glfwGetTime();

		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		// send mvp transformation to currently bound shader, in the "MVP" uniform
		// done in main loop since each model will have different MVP (at least for M part)
		//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

		test->update(currTime - prevTime);
		render(test->getHeightMap());

		//std::cout << "time " << currTime - prevTime << std::endl;
		prevTime = currTime;
		// Swap buffers
		glfwSwapBuffers(window);

		glfwPollEvents();
	}	// check if ESC key was pressed or window was closed
	while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
			glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	cleanUp();

	glDeleteProgram(programID);

	delete test;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}


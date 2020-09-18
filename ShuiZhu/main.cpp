// OpenGL set-up code from http://ogldev.atspace.co.uk/
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>

//GLEW must be included before GLUT or GLFW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

//#include "CImg.h"
#include "stb_image.h"
#include "shader.h"
#include "System_NP.h"

const char* title = "DripDrop";
GLFWwindow* window;
int width = 1024;
int height = 1024;
const char* vertexShaderFile = "SimpleVertexShader.vertexshader";
//const char* fragmentShaderFile = "SimpleFragmentShader.fragmentshader";
const char* fragmentShaderFile = "Attempt1.fragmentshader";

std::string environment = "Creek";
std::string cubeFaces[6] = {
		"posx.jpg",
		"negx.jpg",
		"posy.jpg",
		"negy.jpg",
		"posz.jpg",
		"negz.jpg"
};

double prevTime;
double currTime;

GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint elementbuffer;
GLuint heightMap_textureID;
GLuint programID;

GLuint cubeMap_textureID;

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

//static GLubyte pixels_test[] = {
//		255, 0, 0, 		0, 255, 0,
//		0, 0, 255, 		255, 255, 0
//};
//
//static GLfloat pixels_test[] = {
//		0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
//		1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
//};

// NOT ENTIRELY SURE WHAT TO DO WITH THESE VARIABLES BUT LIKE YEAH
float system_width = 2.0f;	// cm
float system_height = 2.0f;	// cm
float system_gridlength = 0.001f;
//float* system_heightMap;

void createVertexBuffer() {
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);		// associate any VBO/EBO_IBO that you bind with this VAO

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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indices.size()*sizeof(unsigned short),
			//&indices[0], GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
				indices, GL_STATIC_DRAW);
}

void createTextures() {
	// height map
	glGenTextures(1, &heightMap_textureID);
	glBindTexture(GL_TEXTURE_2D, heightMap_textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
			GL_TEXTURE_2D,		// target
			0, 					// level
			GL_R32F,					// internal format
			(int)std::round(system_width/system_gridlength),					// width
			(int)std::round(system_height/system_gridlength),					// height
			0,					// border
			GL_RED,					// format
			GL_FLOAT,			// type
			0					// null data --> reserves memory
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// cube map
	glGenTextures(1, &cubeMap_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap_textureID);

	int width, height, nrChannels;
	unsigned char* image;
	for (int i = 0; i < 6; i++) {
		std::string imageSrc = "cubeMaps/" + environment + "/" + cubeFaces[i];
//		cimg_library::CImg<unsigned char> image(imageSrc.c_str());
//		glTexImage2D(
//				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,		// target
//				0, 					// level
//				GL_RGB,					// internal format
//				image.width(),					// width
//				image.height(),					// height
//				0,					// border
//				GL_RGB,					// format
//				GL_UNSIGNED_BYTE,			// type
//				image					// null data --> reserves memory
//		);

		image = stbi_load(imageSrc.c_str(), &width, &height, &nrChannels, 0);
		if (image)
			glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,		// target
					0, 					// level
					GL_RGB,					// internal format
					width,					// width
					height,					// height
					0,					// border
					GL_RGB,					// format
					GL_UNSIGNED_BYTE,			// type
					image					// null data --> reserves memory
			);
		else
			std::cout << "Failed to load cube map texture" << std::endl;
		stbi_image_free(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void cleanUp() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteBuffers(1, &elementbuffer);
}

void render(float* data) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightMap_textureID);
	glTexSubImage2D(
			GL_TEXTURE_2D,		// target
			0,					// level
			0,					// xoffset
			0,					// yoffset
			(int)std::round(system_width/system_gridlength),	// width
			(int)std::round(system_height/system_gridlength),	// height
			GL_RED,				// format
			GL_FLOAT,			// type
			data
	);
//	glUniform1i(glGetUniformLocation(programID, "heightMap"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap_textureID);
//	glUniform1i(glGetUniformLocation(programID, "cubeMap"), 1);

	// There is no need to repeatedly pass the same data to the GPU using glBind
	// a VAO basically a set of vertex attributes and VBOs/EBOs; two switch between sets,
	// just use glBindVertexArray()
	glBindVertexArray(VertexArrayID);

//	glDrawElements(
//		GL_TRIANGLES,      // mode
//		//indices.size(),    // count
//		sizeof(indices),
//		GL_UNSIGNED_SHORT,   // type
//		(void*)0           // element array buffer offset
//	);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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

//	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);

	// width, height = 0.3
	// grid length = 0.01
	System* test = new System(system_width, system_height, system_gridlength);

	createVertexBuffer();
	setUpVertexAttributes();
	createTextures();
//	createElementBuffer();
	programID = LoadShaders(vertexShaderFile, fragmentShaderFile);

//	GLuint textureName = glGetUniformLocation(programID, "heightMap");
	// placing the glUniform here has no effect, since you must first bind the program
	// to the context using glUseProgram prior to being able to change a uniform value
//	glUniform1i(glGetUniformLocation(programID, "heightMap"), 0);

	glUseProgram(programID);
	glUniform1f(glGetUniformLocation(programID, "gridLength"), system_gridlength);
	glm::vec2 step = glm::vec2(system_gridlength/system_width, system_gridlength/system_width);
	glUniform2fv(glGetUniformLocation(programID, "step"), 1, glm::value_ptr(step));
	glUniform1i(glGetUniformLocation(programID, "heightMap"), 0);
	glUniform1i(glGetUniformLocation(programID, "cubeMap"), 1);

	prevTime = glfwGetTime();
	float meh = system_width/system_gridlength;
	std::cout << meh << std::endl;
	std::cout << (int)meh << std::endl;
	std::cout << (int)(system_width/system_gridlength) << std::endl;

	do {
		currTime = glfwGetTime();

		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(programID);

		//test->update(currTime - prevTime);
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


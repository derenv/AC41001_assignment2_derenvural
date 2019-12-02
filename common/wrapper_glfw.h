/**
wrapper_glfw.h
 Modified from the OpenGL GLFW example to provide a wrapper GLFW class
  and to include shader loader functions to include shaders as text files.
  This version has been modified to build geometry shaders as well as vertex and fragment.
  Iain Martin November 2018
*/
#pragma once

#include <string>

/* Inlcude GL_Load and GLFW */
#include <glload/gl_4_0.h>
#include <glload/gl_load.h>
#include <GLFW/glfw3.h>

class GLWrapper {
private:

	int width;
	int height;
	const char *title;
	double fps;
	void(*error_callback)(int error, const char* description);
	void(*renderer)();
	void(*reshape)(GLFWwindow* window, int w, int h);
	void(*keyCallBack)(GLFWwindow* window, int key, int scancode, int action, int mods);
	bool running;
	GLFWwindow* window;

public:
	GLWrapper(int width, int height, const char *title);
	~GLWrapper();

	void setFPS(double fps) {
		this->fps = fps;
	}

	void DisplayVersion();

	/* Callback registering functions */
	void setRenderer(void(*f)());
	void setReshapeCallback(void(*f)(GLFWwindow* window, int w, int h));
	void setKeyCallback(void(*f)(GLFWwindow* window, int key, int scancode, int action, int mods));
	void setErrorCallback(void(*f)(int error, const char* description));

	/* Shader load and build support functions */
	GLuint LoadShader(const char *vertex_path, const char *fragment_path,
		const char *geometry_path = NULL);
	GLuint BuildShader(GLenum eShaderType, const std::string &shaderText);
	GLuint BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr);
	std::string readFile(const char *filePath);

	int eventLoop();
	GLFWwindow* getWindow();
};




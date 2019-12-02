/*
* triangle.h
*/

#pragma warning( disable : 4099)

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Triangle
{
public:
	Triangle();
	~Triangle();

	void makeTriangle();
	void drawTriangle(int drawmode);

	// Define vertex buffer object names (e.g as globals)
	GLuint positionBufferObject;
	GLuint colourObject;
	GLuint normalsBufferObject;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numvertices;
	int drawmode;
};

/* object_ldr.h
Example class to show the start of an .obj mesh object file loader
Calculates smooth or flat shaded vertex normals for the wavefront object.
A colour buffer has been added but just populated with a single colour.
Iain Martin November 2018

This is a wrapper class based around the code taken from :
http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Load_OBJ
I made a few changes to fit in with my vertex shaders and added the code to
bind the vertex buffers.
Uses std::vector class as the containor for the array of vec3 types
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class object_ldr
{
public:
	object_ldr();
	~object_ldr();

	void load_obj(const char* filename, bool smoothShade = false);
	void drawObject(int drawmode=0);
	void createObject();
	
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colours;
	std::vector<GLuint> elements;

	GLuint vbo_mesh_vertices;
	GLuint vbo_mesh_colours;
	GLuint vbo_mesh_normals;
	GLuint ibo_mesh_elements;
	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colour;

	void smoothNormals();
	void flatNormals();
	void setColours(glm::vec3 colour = glm::vec3(1.f, 0, 0));
};


/* object_ldr.cpp
Example class to show the start of an .obj mesh object file loader
Calculates smooth or flat shaded vertex normals for the wavefront object.
A colour buffer has been added but just popuoated with a single colour.  
Iain Martin November 2017

This is a wrapper class based around the code taken from :
http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Load_OBJ
I made a few changes to fit in with my vertex shaders and added the code to
bind the vertex buffers.
Uses std::vector class as the containor for the array of vec3 types
*/

#include "object_ldr.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

/* I don't like using namespaces in header files but have less issues with them in
   seperate cpp files */
using namespace std;
using namespace glm;

/* Define the vertex attributes for vertex positions and normals. 
   Make these match your application and vertex shader
   You might also want to add colours and texture coordinates */
object_ldr::object_ldr()
{
	attribute_v_coord = 0;
	attribute_v_colour = 1;
	attribute_v_normal = 2;
}


object_ldr::~object_ldr()
{
}

/* Load the object, parsing the file. 
For every line begining with 'v', it adds an extra vec3 to the std::vector containor 
For every line beginning with 'f', it adds the "face" indices to the array of indices
Then it calculates flat shaded normals, i.e calculates the face normal and applies that to 
all vertices in the face.

This function could be improved by extending the parsing to cope with face definitions with
normals defined.
*/
void object_ldr::load_obj(const char* filename, bool smoothShade) {
	ifstream in(filename, ios::in);
	if (!in) { cerr << "Cannot open " << filename << endl; exit(1); }

	string line;
	while (getline(in, line)) 
	{
		if (line.substr(0, 2) == "v ") 
		{
			istringstream s(line.substr(2));
			vec3 v; s >> v.x; s >> v.y; s >> v.z; 
			vertices.push_back(v);
		}
		else if (line.substr(0, 2) == "f ") 
		{
			istringstream s(line.substr(2));
			GLuint a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}
		else if (line[0] == '#') { /* ignoring this line */ }
		else { /* ignoring this line */ }
	}

	/* Manually fill the colour buffer with a chosen colour for this object.*/
	/* You could replace this with proper vertex colours */
	setColours(vec3(0, 0.5, 0.5f));

	if (smoothShade)
	{
		smoothNormals();
	}
	else
	{
		flatNormals();
	}

}


/* Copy the vertices, normals and element indices into vertex buffers */
void object_ldr::createObject()
{
	/* Generate the vertex buffer object */
	glGenBuffers(1, &vbo_mesh_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_vertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &(vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &vbo_mesh_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_normals);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &(normals[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	/* Note that these colours are not currently loaded from the .obj file */
	glGenBuffers(1, &vbo_mesh_colours);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_colours);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(vec3), &(colours[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate a buffer for the indices
	glGenBuffers(1, &ibo_mesh_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size()* sizeof(GLuint), &(elements[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

/* Enable vertex attributes and draw object
Could improve efficiency by moving the vertex attribute pointer functions to the
create object but this method is more general 
This code is almost untouched fomr the tutorial code except that I changed the
number of elements per vertex from 4 to 3*/
void object_ldr::drawObject(int drawmode)
{
	int size;	// Used to get the byte size of the element (vertex index) array

	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_vertices);
	glEnableVertexAttribArray(attribute_v_coord);
	glVertexAttribPointer(
		attribute_v_coord,  // attribute index
		3,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_normals);
	glEnableVertexAttribArray(attribute_v_normal);
	glVertexAttribPointer(
		attribute_v_normal, // attribute
		3,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_colours);
	glEnableVertexAttribArray(attribute_v_colour);
	glVertexAttribPointer(
		attribute_v_colour, // attribute
		3,                  // number of elements per vertex, here (r,g,b)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);


	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, vertices.size());
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh_elements);
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		glDrawElements(GL_TRIANGLES, size / sizeof(GLuint), GL_UNSIGNED_INT, 0);
	}
}


/** Flat shaded normals */
void object_ldr::flatNormals()
{
	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));
	for (GLuint i = 0; i < elements.size(); i += 3) {
		GLuint ia = elements[i];
		GLuint ib = elements[i + 1];
		GLuint ic = elements[i + 2];
		vec3 normal = normalize(cross(
			vec3(vertices[ib]) - vec3(vertices[ia]),
			vec3(vertices[ic]) - vec3(vertices[ia])));
		normals[ia] = normals[ib] = normals[ic] = normal;
	}
}

/* This is the smooth normals function given in the tutorial code */
void object_ldr::smoothNormals()
{
	std::vector<GLuint> nb_seen;
	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));
	nb_seen.resize(vertices.size(), 0);
	for (uint i = 0; i < elements.size(); i += 3) {
		GLuint ia = elements[i];
		GLuint ib = elements[i + 1];
		GLuint ic = elements[i + 2];
		vec3 normal = normalize(cross(
			vec3(vertices[ib]) - vec3(vertices[ia]),
			vec3(vertices[ic]) - vec3(vertices[ia])));

		int v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
		for (int j = 0; j < 3; j++) {
			GLuint cur_v = v[j];
			nb_seen[cur_v]++;
			if (nb_seen[cur_v] == 1) {
				normals[cur_v] = normal;
			}
			else {
				// average
				normals[cur_v].x = normals[cur_v].x * (1.f - 1.f / nb_seen[cur_v]) + normal.x * 1.f / nb_seen[cur_v];
				normals[cur_v].y = normals[cur_v].y * (1.f - 1.f / nb_seen[cur_v]) + normal.y * 1.f / nb_seen[cur_v];
				normals[cur_v].z = normals[cur_v].z * (1.f - 1.f / nb_seen[cur_v]) + normal.z * 1.f / nb_seen[cur_v];
				normals[cur_v] = normalize(normals[cur_v]);
			}
		}
	}

}

/**
* Define colours for our object manually
*/
void object_ldr::setColours(vec3 c)
{
	for (uint i = 0; i < vertices.size(); i++)
	{
		colours.push_back(c);
	}
}

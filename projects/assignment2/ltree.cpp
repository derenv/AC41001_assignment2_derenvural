/*
 LSystem example
 Basic example to show how to create a 2D LSystem with texture
 Note that this is still basically in 2D so you may want to modify this
 to make it fully three-dimensional and move the L System into its own class
 Iain Martin Novemeber 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "soil.lib")

#include <stack>

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Include the hacked version of SOIL */
#include "soil.h"

using namespace glm;
using namespace std;

class ltree{
public:
	//variables
	GLuint quad_vbo, quad_normals, quad_colours, quad_tex_coords;
	GLuint texID;
	mat4 model;
	GLuint program;
	GLuint colourmode;
	GLfloat angle_x, angle_inc_x, model_scale;
	GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
	GLuint drawmode;
	GLuint modelID, viewID, projectionID;
	GLuint colourmodeID;

	char lstring[20] = "F[+F][-F]\0";
	//void drawBranch(int leaf);
	//void tree(int level, int leaves);
	stack<mat4> lsystem_transform;
	int treelevel;
	bool leaves;		// if true, draw leaves


	/*
	This function is called before entering the main rendering loop.
	Use it for all your initialisation stuff
	*/
	void init(GLWrapper* glw, GLuint new_texID, GLuint new_colourmode, GLuint new_program)
	{
		/* Set the object transformation controls to their initial values */
		//x = 0;
		//y = -0.5f;
		//z = 0;
		angle_y = angle_z = 0;
		angle_x = -20.f;
		angle_inc_x = angle_inc_y = angle_inc_z = 0;
		model_scale = 0.5f;
		colourmode = new_colourmode;
		leaves = 0;
		program = new_program;

		glUseProgram(program);
		/* Define uniforms to send to vertex shader */
		modelID = glGetUniformLocation(program, "model");
		colourmodeID = glGetUniformLocation(program, "colourmode");
		viewID = glGetUniformLocation(program, "view");
		projectionID = glGetUniformLocation(program, "projection");

		/* Create our quad and texture */
		glGenBuffers(1, &quad_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

		// Create dat for our quad with vertices, normals and texturee coordinates 
		static const GLfloat quad_data[] =
		{
			// Vertex positions
			0, 0, 0,
			0.2, 0, 0,
			0.2, 1.f, 0,
			0, 1.f, 0,

			// Normals
			0, 0, 1.f,
			0, 0, 1.f,
			0, 0, 1.f,
			0, 0, 1.f,

			// Texture coordinates. Note we only need two per vertex but have a
			// redundant third to fit the texture coords in the same buffer for this simple object
			0.0f, 0.0f, 0,
			1.0f, 0.0f, 0,
			1.0f, 1.0f, 0,
			0.0f, 1.0f, 0,
		};

		// Copy the data into the buffer. See how this example combines the vertices, normals and texture
		// coordinates in the same buffer and uses the last parameter of  glVertexAttribPointer() to
		// specify the byte offset into the buffer for each vertex attribute set.
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		new_texID = texID;

		/* Define the texture behaviour parameters */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// This is the location of the texture object (TEXTURE0), i.e. tex1 will be the name
		// of the sampler in the fragment shader
		int loc = glGetUniformLocation(program, "tex1");
		if (loc >= 0) glUniform1i(loc, 0);

		treelevel = 1;
	}

	/* Called to update the display. Note that this function is called in the event loop in the wrapper
	   class because we registered display as a callback function */
	void draw_ltree(mat4 projection, mat4 view, GLfloat x, GLfloat y, GLfloat z)
	{
		/* Make the compiled shader program current */
		glUseProgram(program);

		lsystem_transform.push(mat4(1.0f));

		// Define the model transformations 
		model = mat4(1.0f);
		lsystem_transform.top() = translate(lsystem_transform.top(), vec3(x, y, z));
		lsystem_transform.top() = scale(lsystem_transform.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
		lsystem_transform.top() = rotate(lsystem_transform.top(), -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
		lsystem_transform.top() = rotate(lsystem_transform.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
		lsystem_transform.top() = rotate(lsystem_transform.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

		// Send our uniforms variables to the currently bound shader,
		glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);

		// Draw tree
		tree(treelevel, leaves, x, y);

		/* Modify our animation variables */
		angle_x += angle_inc_x;
		angle_y += angle_inc_y;
		angle_z += angle_inc_z;
	}

	/* Draw a single branch */
	/* Could draw a different shape for a leaf! */
	void drawBranch(int leaf)
	{
		mat4 transform = lsystem_transform.top();
		transform = transform * model;
		colourmode = leaf;
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(transform[0][0]));
		glUniform1ui(colourmodeID, colourmode);

		/* Draw our textured quad*/
		glBindTexture(GL_TEXTURE_2D, texID);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	/* Recursive function to draw an LSystem Tree*/
	void tree(int level, int leaves, GLfloat x, GLfloat y) {
		int current;
		current = 0;
		while (lstring[current])
		{
			switch (lstring[current])
			{
			case 'F':
				if (level == 0)
				{
					if (leaves && ((lstring[current + 1] == ']') || (lstring[current + 1] == 0)))
						drawBranch(0);
					else
						drawBranch(1);

					lsystem_transform.top() = translate(lsystem_transform.top(), vec3(0, 1.0f, 0));
				}
				else
				{
					if ((lstring[current + 1] == ']') || (lstring[current + 1] == 0))
						tree(level - 1, leaves,x,y);
					else
						tree(level - 1, 0, x, y);
				}
				break;
			case '[':
				lsystem_transform.push(scale(lsystem_transform.top(), vec3(0.7, 0.7, 0.7)));
				break;
			case ']':
				lsystem_transform.pop();
				break;
			case '+':
				// rotate 45 degrees
				lsystem_transform.top() = rotate(lsystem_transform.top(), radians(-45.f + x), vec3(0, 0, 1.f));
				break;
			case '-':
				lsystem_transform.top() = rotate(lsystem_transform.top(), radians(30.f + y), vec3(0, 0, 1.f));
				break;
			default:
				break;
			}
			current++;
		}
	}
};
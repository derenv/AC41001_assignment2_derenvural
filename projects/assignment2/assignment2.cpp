/*
AC41001 Assignment 2
Deren Vural November 2019
*/

// Link to static libraries
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

// GLFW wrapper class
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

// GLM core and matrix extensions
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Programatic objects
#include "sphere_tex.h"
#include "tetrahedron.h"

// Wavefront .obj objects
#include "tiny_loader_texture.h"

// Terrains
#include "terrain_object.h"

// image loader
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Array size
const int program_amount = 4;
const int texture_amount = 9;
const int object_files_amount = 8;

// Shader variables
GLuint programs[program_amount];
GLuint current_vertex_shader;
GLuint current_fragment_shader;
GLuint current_geometry_shader;
GLuint current_program;
GLuint vao;

// Position and view globals
GLfloat angle_x, angle_inc_x, x;
GLfloat angle_y, angle_inc_y, y;
GLfloat angle_z, angle_inc_z, z;
GLfloat view_move_x, view_move_y, view_move_z;
GLfloat vx, vy, vz;

// Lighting globals
GLfloat ambient_constant;
GLfloat light_x, light_y, light_z;

// Application globals
GLfloat model_scale, aspect_ratio;
GLuint drawmode, colourmode, emitmode, attenuationmode;

// Object globals
GLuint numlats, numlongs;

// Textures
GLuint textureID[texture_amount];
const char* image_files[texture_amount] = {
	////WINDMILL
	"..\\..\\images\\concrete.jpg",
	"..\\..\\images\\planks2.jpg",
	"..\\..\\images\\rusty_iron.jpg",//also torch
	//WINDMILL BLADES
	//CASTLE
	"..\\..\\images\\planks1.ppg",
	"..\\..\\images\\diffuse.ppg",
	"..\\..\\images\\normal.ppg",
	//DOOR
	"..\\..\\images\\doorold.jpg",
	//TORCH
	"..\\..\\images\\bark1.png",
	//TOWER
	"..\\..\\images\\Medieval tower_mid_Col.png"
};

// Uniforms IDs
GLuint modelID[program_amount], viewID[program_amount], projectionID[program_amount], normalmatrixID[program_amount];
GLuint colourmodeID[program_amount], drawmodeID[program_amount], emitmodeID[program_amount], attenuationmodeID[program_amount];
GLuint lightposID[program_amount], ambient_constantID[program_amount];

// Programatic objects
Sphere aSphere;

// Wavefront .obj objects
TinyObjLoader castle;
TinyObjLoader windmill;
TinyObjLoader torch;
TinyObjLoader door;
TinyObjLoader someObject;
const char* objects[object_files_amount] = {
	"..\\..\\objects\\monkey.obj",
	"..\\..\\objects\\monkey_normals.obj",
	"..\\..\\objects\\sofa.obj",
	"..\\..\\objects\\windmill.obj",
	"..\\..\\objects\\castle_tex.obj",
	"..\\..\\objects\\Medieval_tower_High_.obj",
	"..\\..\\objects\\torch.obj",
	"..\\..\\objects\\door.obj"
};

// Terrain objects
terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;
GLfloat sealevel = 0;

using namespace std;
using namespace glm;

void show_controls() {
	cout << "\ncontrols:" << endl;
	cout << "esc : end program" << endl;
	cout << "- : change draw mode" << endl;
	cout << "LIGHT:" << endl;
	cout << "1 : x -= 0.05f" << endl;
	cout << "2 : x += 0.05f" << endl;
	cout << "3 : y -= 0.05f" << endl;
	cout << "4 : y += 0.05f" << endl;
	cout << "5 : z -= 0.05f" << endl;
	cout << "6 : z += 0.05f" << endl;
	cout << "SCENE ROTATION:" << endl;
	cout << "Q : x -= 0.05f" << endl;
	cout << "W : x += 0.05f" << endl;
	cout << "E : y -= 0.05f" << endl;
	cout << "R : y += 0.05f" << endl;
	cout << "T : z -= 0.05f" << endl;
	cout << "Y : z += 0.05f" << endl;
	cout << "VIEW:" << endl;
	cout << "Z : x -= 0.05f" << endl;
	cout << "X : x += 0.05f" << endl;
	cout << "C : y -= 0.05f" << endl;
	cout << "V : y += 0.05f" << endl;
	cout << "B : z -= 0.05f" << endl;
	cout << "N : z += 0.05f" << endl;
	cout << ", : reset position" << endl;
	cout << "VIEW AIM:" << endl;
	cout << "NUMPAD4 : x -= 0.05f" << endl;
	cout << "NUMPAD6 : x += 0.05f" << endl;
	cout << "NUMPAD8 : y -= 0.05f" << endl;
	cout << "NUMPAD2 : y += 0.05f" << endl;
	cout << "NUMPAD0 : z -= 0.05f" << endl;
	cout << "NUMPAD5 : z += 0.05f" << endl;
	cout << ". : reset aim" << endl;
	cout << "UNIFORMS:" << endl;
	cout << "L : print active uniforms (for debug)" << endl;
}
/*
load texture from passed file
optionally generate mipmaps
*/
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps);
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps){
	glGenTextures(1, &texID);

	// Image parameters
	int width, height, nrChannels;

	// load an image file using stb_image
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data){
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3){
			pixel_format = GL_RGB;
		}else{
			pixel_format = GL_RGBA;
		}

		// Bind the texture ID
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps){
			glGenerateMipmap(GL_TEXTURE_2D);
		}else{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}else{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}


float get_terrain_position(terrain_object* heightfield, float xval, float zval);
float get_terrain_position(terrain_object* heightfield, float xval, float zval) {
	//	vec2 pos = heightfield->getGridPos(x, z);
	return heightfield->heightAtPosition(xval, zval);
}

/*
initialisation stuff
*/
void init(GLWrapper* glw)
{
	// Initial variable values
	// Position and view globals
	angle_y = angle_z = 0;
	angle_x = 295.f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	vx = 0; vy = 0; vz = 0;

	//view position
	view_move_x = -10.3; view_move_y = 1.8; view_move_z = 3;
	x = -10.3; y = 1.8; z = 4;

	// Application globals
	model_scale = .1f;
	aspect_ratio = 1.3333f;

	// Lighting globals
	ambient_constant = .2f;
	colourmode = 1;
	drawmode = 0;
	emitmode = 0;
	attenuationmode = 1;
	light_x = light_y = light_z = 1;

	// Object globals
	numlats = numlongs = 60;

	// Generate index, Create and make the current VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Load and build all file dependancies
	try{
		//shaders
		programs[0] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_phong.frag");
		programs[1] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_oren_nayar.frag");
		programs[2] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_cook_torrance.frag");

		programs[3] = glw->LoadShader("..\\..\\shaders\\terrain.vert", "..\\..\\shaders\\terrain.frag");

		//wavefront .obj objects
		//0-monkey(V--), 1-monkey_normals(V-N), 2-sofa(V-N), 3-windmill1(VTN)
		//4-castle(VTN), 5-towerhouse(VTN), 6-torch(VTN), 7-door(VTN)
		windmill.load_obj(objects[3], false, true);
		castle.load_obj(objects[4], false, true);
		torch.load_obj(objects[6], false, true);
		door.load_obj(objects[7], false, true);
		someObject.load_obj(objects[5], false, true);
	}catch (exception & e){
		cout << "Caught exception: " << e.what() << endl;
		//cin.ignore();
		exit(1);
	}
	current_program = 0;


	

	// This will flip the image so that the texture coordinates defined in
	// the sphere, match the image orientation as loaded by stb_image
	stbi_set_flip_vertically_on_load(true);

	// Load textures
	for (int i = 0; i < texture_amount; i++) {
		if (!load_texture(image_files[i], textureID[i], true))
		{
			cout << "Fatal error loading texture: " << image_files[i] << endl;
			exit(0);
		}
		else {
			cout << "successfully loaded texture: " << image_files[i] << endl;
		}
	}



	/* Define the uniforms to send to shaders */
	for (int i = 0; i < program_amount; i++)
	{
		glUseProgram(programs[i]);

		// Common uniforms
		modelID[i] = glGetUniformLocation(programs[i], "model");
		viewID[i] = glGetUniformLocation(programs[i], "view");
		projectionID[i] = glGetUniformLocation(programs[i], "projection");
		colourmodeID[i] = glGetUniformLocation(programs[i], "colourmode");

		// Not needed for terrain shader
		if (i < 3) {
			normalmatrixID[i] = glGetUniformLocation(programs[i], "normalmatrix");
			lightposID[i] = glGetUniformLocation(programs[i], "light_positions");
			ambient_constantID[i] = glGetUniformLocation(programs[i], "ambient_constant");
			emitmodeID[i] = glGetUniformLocation(programs[i], "emitmode");
			attenuationmodeID[i] = glGetUniformLocation(programs[i], "attenuationmode");
		}

		// Enable face culling
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

		// This is the location of the texture object (name of the sampler in the fragment shader)
		int loc = glGetUniformLocation(programs[i], "texture");
		if (loc >= 0) { glUniform1i(loc, 0); }
	}

	//terrain object
	octaves = 4;
	perlin_scale = 3.f;
	perlin_frequency = 1.f;
	land_size = 20.f;
	land_resolution = 1000;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size, sealevel);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();
	glBindVertexArray(vao);
	current_program = 0;

	// Create programatic objects
	aSphere.makeSphere(numlats, numlongs);

	// Define initial object positions
	//y = get_terrain_position(heightfield,x,z);

	//error check
	for (int i = 0; i < program_amount; i++)
	{
		GLint x1;
		GLint* px = &x1;
		glGetProgramiv(programs[i], GL_ATTACHED_SHADERS, px);//can change GL_ATTACHED_SHADERS to some program property from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgram.xhtml
		cout << "program " << i << ", attached shaders: " << x1 << endl;
	}
}

void check_for_gl_error() {
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		if (error == GL_INVALID_VALUE) {
			cerr << "error occurred: " << error << " GL_INVALID_VALUE" << endl;
		}
		else if (error == GL_INVALID_OPERATION) {
			cerr << "error occurred: " << error << " GL_INVALID_OPERATION" << endl;
		}
		else {
			cerr << "unknown error occurred: " << error << endl;
		}
	}
}

/*
Called to update the display in the event loop in the wrapper class (callback function)
*/
void display() {
	// Define the background colour
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Clear the colour and frame buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Make the compiled shader program current
	glUseProgram(programs[current_program]);

	// Define the model transformations
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	vec3 player_position = vec3(view_move_x, view_move_y, view_move_z);
	vec3 forward = vec3(x,y,z);
	mat4 view = lookAt(
		forward,
		player_position,
		vec3(0, 1, 0)
	);

	// View rotations/transforms
	//view = translate(view, player_position);
	//view = rotate(view, -radians(vx), vec3(.1f, 0, 0)); //normalize(vec3(player_position.x, 0, 0))); //rotating in clockwise direction around x-axis
	//view = rotate(view, -radians(vy), vec3(0, 1, 0)); //normalize(vec3(0, player_position.y, 0)));// //rotating in clockwise direction around y-axis		USE FOR PLAYER ROTATION
	//view = rotate(view, -radians(vz), vec3(0, 0, .1f)); //normalize(vec3(0, 0, player_position.z)));// //rotating in clockwise direction around z-axis

	// Light sources
	vec4 lightpos1 = view * vec4(light_x, light_y, light_z, 1.0);
	vec4 lightpos2 = view * vec4(light_x, light_y, light_z, 1.0);
	vec4 the_sun = view * vec4(0.0, 5.0, 0.0, 0.0);
	vec4 light_positions[3] = { lightpos1, lightpos2, the_sun };

	// Send our uniforms variables to the currently bound shader
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[current_program], 2, value_ptr(light_positions[0]));
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniform1ui(colourmodeID[current_program], attenuationmode);
	glUniform1f(ambient_constantID[current_program], ambient_constant);
	glFrontFace(GL_CW);

	check_for_gl_error();

	// Terrain
	model.push(model.top());
	{
		// switch to terrain program
		glUseProgram(programs[3]);

		// Uniforms
		glUniformMatrix4fv(modelID[3], 1, GL_FALSE, &(model.top()[0][0]));
		glUniformMatrix4fv(viewID[3], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionID[3], 1, GL_FALSE, &projection[0][0]);
		glUniform1ui(colourmodeID[3], colourmode);

		// Draw terrain
		heightfield->drawObject(drawmode);

		// switch to objects program
		glUseProgram(programs[current_program]);
	}
	model.pop();
	check_for_gl_error();

	//immovable objects
	//castle
	model.push(model.top());
	{
		GLuint castle_program = 1;
		glUseProgram(programs[castle_program]);

		// Transformations
		float castle_x = -10;
		float castle_z = 0;
		model.top() = translate(model.top(), vec3(castle_x, get_terrain_position(heightfield,0,0)+1.f, castle_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));

		// Uniforms
		glUniformMatrix4fv(modelID[castle_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[castle_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		glBindTexture(GL_TEXTURE_2D, textureID[0]);

		// Draw object
		castle.drawObject(drawmode);
		glUseProgram(programs[current_program]);
	}
	model.pop();
	check_for_gl_error();
	//windmill
	model.push(model.top());
	{
		// Transformations
		float windmill_x = 7;
		float windmill_z = -7;
		model.top() = translate(model.top(), vec3(windmill_x, get_terrain_position(heightfield, windmill_x, windmill_z)-.1f, windmill_z));
		model.top() = scale(model.top(), vec3(model_scale * 4, model_scale * 4, model_scale * 4));
		model.top() = rotate(model.top(), -radians(120.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//

		// Draw object
		windmill.drawObject(drawmode);
	}
	model.pop();
	check_for_gl_error();

	//Torch
	model.push(model.top());
	{
		// Transformations
		float torch_x = 7;
		float torch_z = -7;
		model.top() = translate(model.top(), vec3(torch_x, get_terrain_position(heightfield, torch_x, torch_z) - .1f, torch_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));
		model.top() = rotate(model.top(), -radians(120.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//

		// Draw object
		windmill.drawObject(drawmode);
	}
	model.pop();
	check_for_gl_error();
	// Lightsource 1
	model.push(model.top());
	{
		// Transformations
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//

		// Draw object
		// Activate emittance for light source
		emitmode = 1;
		glUniform1ui(emitmodeID[current_program], emitmode);

		aSphere.drawSphere(drawmode);

		// Deactivate emittance for next object
		emitmode = 0;
		glUniform1ui(emitmodeID[current_program], emitmode);
	}
	model.pop();
	check_for_gl_error();

	/*
	//L-Trees
	model.push(model.top());
	{
		//vars

		//fore each level
		//for(int i=0;i<;i++){
			//get random coordinates
			//

			//create level
		//}
	}
	model.pop();
	check_for_gl_error();
	*/



	// Global transformations
	//model.top() = translate(model.top(), vec3(x, y, z));
	//model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	//model.top() = rotate(model.top(), -radians(angle_x - 50.f), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	//model.top() = rotate(model.top(), -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	//model.top() = rotate(model.top(), -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Object 1
	//model.push(model.top());
	//{
	//	// Transformations
	//	//model.top() = translate(model.top(), vec3(x, y+0.2, z));
	//	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis

	//	// Uniforms
	//	glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
	//	normalmatrix = transpose(inverse(mat3(view * model.top())));
	//	glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

	//	// Textures
	//	glBindTexture(GL_TEXTURE_2D, textureID[0]);

	//	// Draw object
	//	//someObject.overrideColour(vec4(0.f, 0.f, 1.f, 1.f));
	//	someObject.drawObject(drawmode);
	//}
	//model.pop();
	//check_for_gl_error();

	// Reset for next loop
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	// Modify our animation variables
	//angle_x += angle_inc_x;
	//angle_y += angle_inc_y;
	//angle_z += angle_inc_z;

	//check for uncaught error
	check_for_gl_error();
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}


static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods){
	//bool recreate_terrain = false;
	//bool placeObject = false;

	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// activate
	if (key == 'F') {
		//
	}

	//move
	//look left/right
	if (key == 'Q' && action == GLFW_PRESS) {
		//turn left
		if (z > view_move_z && x == view_move_x) {
			//facing away turning left
			view_move_x -= 1.f;
			view_move_z = z;
		} else if (z < view_move_z && x == view_move_x) {
			//facing toward turning left
			view_move_x += 1.f;
			view_move_z = z;
		} else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left turning left
				view_move_z += 1.f;
				view_move_x = x;
			} else if (x < view_move_x) {
				//facing right turning left
				view_move_z -= 1.f;
				view_move_x = x;
			}
		}
	}
	if (key == 'E' && action == GLFW_PRESS) {
		//turn right
		if (z > view_move_z && x == view_move_x) {
			//facing away turning right
			view_move_x += 1.f;
			view_move_z = z;
		}
		else if (z < view_move_z && x == view_move_x) {
			//facing toward turning right
			view_move_x -= 1.f;
			view_move_z = z;
		}
		else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left turning right
				view_move_z -= 1.f;
				view_move_x = x;
			}
			else if (x < view_move_x) {
				//facing right turning right
				view_move_z += 1.f;
				view_move_x = x;
			}
		}
	}
	//strafe left/right
	if (key == 'A') {
		//turn left
		if (z > view_move_z&& x == view_move_x) {
			//facing away strafing left
			view_move_x -= 0.05f;
			x -= 0.05f;
		} else if (z < view_move_z && x == view_move_x) {
			//facing toward strafing left
			view_move_x += 0.05f;
			x += 0.05f;
		} else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left strafing left
				view_move_z += 0.05f;
				z += 0.05f;
			} else if (x < view_move_x) {
				//facing right strafing left
				view_move_z -= 0.05f;
				z -= 0.05f;
			}
		}
	}
	if (key == 'D') {
		//turn right
		if (z > view_move_z&& x == view_move_x) {
			//facing away strafing right
			view_move_x += 0.05f;
			x += 0.05f;
		} else if (z < view_move_z && x == view_move_x) {
			//facing toward strafing right
			view_move_x -= 0.05f;
			x -= 0.05f;
		} else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left strafing right
				view_move_z -= 0.05f;
				z -= 0.05f;
			} else if (x < view_move_x) {
				//facing right strafing right
				view_move_z += 0.05f;
				z += 0.05f;
			}
		}
	}

	//look up/down
	if (key == 'Z' && view_move_y > -1.8f) { view_move_y += 0.05f; }//up
	if (key == 'X' && view_move_y < 3.6f) { view_move_y -= 0.05f; }//down
	//reset to normal
	if (key == ',' && action == GLFW_PRESS) {
		view_move_y = y = 0.f;
	}

	//move
	if (key == 'S') {
		//facing away
		if (z > view_move_z&& x == view_move_x) {
			//facing away
			view_move_z += 0.05f;
			z += 0.05f;
		}
		else if (z < view_move_z && x == view_move_x) {
			//facing toward
			view_move_z -= 0.05f;
			z -= 0.05f;
		}
		else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left
				view_move_x += 0.05f;
				x += 0.05f;
			}
			else if (x < view_move_x) {
				//facing right
				view_move_x -= 0.05f;
				x -= 0.05f;
			}
		}
	}
	if (key == 'W') {
		//forward
		if (z > view_move_z&& x == view_move_x) {
			//facing away
			view_move_z -= 0.05f;
			z -= 0.05f;
		} else if (z < view_move_z && x == view_move_x) {
			//facing toward
			view_move_z += 0.05f;
			z += 0.05f;
		} else {
			//facing sideways
			//find side
			if (x > view_move_x) {
				//facing left
				view_move_x -= 0.05f;
				x -= 0.05f;
			} else if (x < view_move_x) {
				//facing right
				view_move_x += 0.05f;
				x += 0.05f;
			}
		}
	}



	//light controls
	/*if (key == GLFW_KEY_1) light_x -= 0.05f;
	if (key == GLFW_KEY_2) light_x += 0.05f;
	if (key == GLFW_KEY_3) light_y -= 0.05f;
	if (key == GLFW_KEY_4) light_y += 0.05f;
	if (key == GLFW_KEY_5) light_z -= 0.05f;
	if (key == GLFW_KEY_6) light_z += 0.05f;*/

	//current position
	if (key == '.' && action != GLFW_PRESS) {
		cout << "view_move_x: " << view_move_x << endl;
		cout << "view_move_y: " << view_move_y << endl;
		cout << "view_move_z: " << view_move_z << endl;
		cout << "x: " << x << endl;
		cout << "y: " << y << endl;
		cout << "z: " << z << endl;
	}

	// Object colour/texture or passed colour
	if (key == '=' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	// Cycle between drawing vertices, mesh and filled polygons
	if (key == '-' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
		cout << "drawmode=" << drawmode << endl;
	}

	// Cycle between shader programs
	if (key == '9' && action != GLFW_PRESS)
	{
		current_program++;
		if (current_program > 2) current_program = 0;
		cout << "current_program=" << current_program << endl;
	}

	// Keep object on the terrain
	/*if (placeObject)
	{
		y = get_terrain_position(heightfield, x, z);
	}*/

	//reset heightfield
	/*if (recreate_terrain)
	{
		delete heightfield;
		heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
		heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size, sealevel);
		heightfield->setColourBasedOnHeight();
		heightfield->createObject();
	}*/


	//DEBUG CONTROLS
	if (key == 'L' && action != GLFW_PRESS) {
		GLint num_uniforms;
		glGetProgramiv(programs[current_program], GL_ACTIVE_UNIFORMS, &num_uniforms);
		GLchar uniform_name[256];
		GLsizei length;
		GLint size;
		GLenum type;
		cout << "===active uniforms : " << num_uniforms << "===" << endl;
		for (int i = 0; i < num_uniforms; i++)
		{
			glGetActiveUniform(programs[current_program], i, sizeof(uniform_name), &length, &size, &type, uniform_name);
			cout << "=" << uniform_name << "=" << length << "=" << size << "=" << type << "=" << endl;
		}
	}
}

/*
Entry point of program
*/
int main(int argc, char* argv[]){
	GLWrapper* glw = new GLWrapper(1024, 768, "Assignment 2: Deren Vural");;

	if (!ogl_LoadFunctions()){
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	// Output the OpenGL vendor and version
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}
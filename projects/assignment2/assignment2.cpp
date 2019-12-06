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
#include "ltree.cpp"

// Array size
const int program_amount = 5;
const int texture_amount = 8;
const int object_files_amount = 12;

// Shader variables
GLuint programs[program_amount];
GLuint current_vertex_shader;
GLuint current_fragment_shader;
GLuint current_geometry_shader;
GLuint current_program;
GLuint vao;

// Position and view globals
GLfloat x, y, z;
GLfloat view_move_x, view_move_y, view_move_z;

// Lighting globals
GLfloat ambient_constant;
GLfloat light_x, light_y, light_z;
GLfloat light2_x, light2_y, light2_z;

// Application globals
GLfloat model_scale, aspect_ratio;
GLuint drawmode, colourmode, emitmode, attenuationmode;

// Object globals
GLuint numlats, numlongs;

// Textures
GLuint textureID[texture_amount];
const char* image_files[texture_amount] = {
	////WINDMILL
	"..\\..\\images\\concrete.png",
	"..\\..\\images\\rusty_iron.png",//also torch
	//"..\\..\\images\\planks2.png",
	//WINDMILL BLADES
	//CASTLE
	"..\\..\\images\\planks1.png",
	"..\\..\\images\\diffuse.png",
	"..\\..\\images\\normal.png",
	//"..\\..\\images\\rooftiles.png",
	//DOOR
	"..\\..\\images\\doorold.png",
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
//TinyObjLoader castle;
TinyObjLoader windmill;
TinyObjLoader windmill_arms;
TinyObjLoader torch;
TinyObjLoader door;
TinyObjLoader teapot;

TinyObjLoader statue;
TinyObjLoader tower;

TinyObjLoader castle_outside;
TinyObjLoader castle_courtyard;
TinyObjLoader castle_roof;
TinyObjLoader castle_walls;
const char* objects[object_files_amount] = {
	"..\\..\\objects\\monkey.obj",
	"..\\..\\objects\\monkey_normals.obj",
	"..\\..\\objects\\windmillarms.obj",
	"..\\..\\objects\\windmill.obj",

	"..\\..\\objects\\castleouterwalls.obj",

	"..\\..\\objects\\Medieval_tower_High_.obj",
	"..\\..\\objects\\torch.obj",
	"..\\..\\objects\\door.obj",

	"..\\..\\objects\\castlecourtyard.obj",
	"..\\..\\objects\\castleroof.obj",
	"..\\..\\objects\\castlewallsandfloors.obj",
	"..\\..\\objects\\teapot.obj"
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

// Animations
bool locked_view;
bool locked_teapot;
vec3 teapots[4] = {
		vec3(-14,5,-7.95),
		vec3(-14,5,4.75),
		vec3(-25.7,5,-7.95),
		vec3(-25.7,5,4.75)
};
int teapot_target;
GLfloat arm_speed, arm_speed_inc;

//L-trees
ltree the_tree;

void show_controls() {
	cout << "\ncontrols:" << endl;
	cout << " _ _ _               _ _ _" << endl;
	cout << "|     |   forward   |     |" << endl;
	cout << "|  Q  |    _ _ _    |  E  |" << endl;
	cout << "|     |   |     |   |     |" << endl;
	cout << "rotate    |  W  |    rotate" << endl;
	cout << "90 left   |     |   90 right" << endl;
	cout << "   _ _ _   _ _ _   _ _ _          _ _ _" << endl;
	cout << "  |     | |     | |     |        |     |" << endl;
	cout << "  |  A  | |  S  | |  D  |        |  F  |" << endl;
	cout << "  |     | |     | |     |        |     |" << endl;
	cout << "  strafe    back   strafe        activate" << endl;
	cout << "  left             right" << endl;
	cout << " _ _ _   _ _ _               _ _ _" << endl;
	cout << "|     | |     |             |     |" << endl;
	cout << "|  Z  | |  X  |             |  V  |" << endl;
	cout << "|     | |     |             |     |" << endl;
	cout << " look    look                reset" << endl;
	cout << "  up     down                 aim" << endl;
	cout << "esc : end program" << endl;
	cout << "SHADER OPTIONS:" << endl;
	cout << "- : change draw mode (0=filled, 1=lines, 2=points)" << endl;
	cout << "= : change colour mode (0=colours, 1=textures)" << endl;
	cout << "0 : change main shader program (0=phong, 1=oren-nayar, 2=cook-torrance)" << endl;
	cout << "DEBUG TOOLS:" << endl;
	cout << "B : print active uniforms" << endl;
	cout << "N : print shader program logs" << endl;
	cout << "M : print view and camera position" << endl;
}

// Debug functions
void check_program_log();
void check_program_log() {
	for (int i = 0; i < program_amount; i++)
	{
		GLchar logdata;
		GLchar* plogdata = &logdata;
		glGetProgramInfoLog(programs[i], 1000, NULL, plogdata);
		cout << "program " << i << ", log: " << plogdata << endl;
	}
}
void check_for_gl_error();
void check_for_gl_error() {
	GLenum error;
	while ((error = glGetError()) != GL_NO_ERROR) {
		if (error == GL_INVALID_ENUM) {
			cerr << "error occurred: " << error << " GL_INVALID_ENUM" << endl;
		}
		else if (error == GL_INVALID_VALUE) {
			cerr << "error occurred: " << error << " GL_INVALID_VALUE" << endl;
		}
		else if (error == GL_INVALID_OPERATION) {
			cerr << "error occurred: " << error << " GL_INVALID_OPERATION" << endl;
		}
		else if (error == GL_STACK_OVERFLOW) {
			cerr << "error occurred: " << error << " GL_STACK_OVERFLOW" << endl;
		}
		else if (error == GL_STACK_UNDERFLOW) {
			cerr << "error occurred: " << error << " GL_STACK_UNDERFLOW" << endl;
		}
		else if (error == GL_OUT_OF_MEMORY) {
			cerr << "error occurred: " << error << " GL_OUT_OF_MEMORY" << endl;
		}
		else {
			cerr << "unknown error occurred: " << error << endl;
		}
		check_program_log();
	}
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
		if (nrChannels == 1) {
			pixel_format = GL_RED;
		} else if (nrChannels == 2) {
			pixel_format = GL_RG;
		} else if (nrChannels == 3){
			pixel_format = GL_RGB;
			//pixel_format = GL_SRGB;
			//pixel_format = GL_BGR;
			//pixel_format = GL_RGB_INTEGER;
			//pixel_format = GL_BGR_EXT;
		}else{
			pixel_format = GL_RGBA;
		}
		check_for_gl_error();
		// Bind the texture ID
		glBindTexture(GL_TEXTURE_2D, texID);
		check_for_gl_error();

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
		check_for_gl_error();
		
		// Generate Mip Maps
		if (bGenMipmaps){
			glGenerateMipmap(GL_TEXTURE_2D);
		}else{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		check_for_gl_error();
	}else{
		cerr << "stb_image  loading error: filename" << filename << endl;
		return false;
	}
	stbi_image_free(data);
	return true;
}

// heightmap functions
float get_terrain_position(terrain_object* heightfield, float xval, float zval);
float get_terrain_position(terrain_object* heightfield, float xval, float zval) {
	//vec2 pos = heightfield->getGridPos(x, z);
	//cout << "position: " << pos.x << "," << pos.y << endl;
	return heightfield->heightAtPosition(xval, zval);
}

/*
initialisation stuff
*/
void init(GLWrapper* glw)
{
	// Initial variable values
	// View & camera position
	view_move_x = -10.1; view_move_y = 1.8; view_move_z = 2.5;
	x = -10.1; y = 1.8; z = 3.5;

	// Animations
	locked_view = false;
	locked_teapot = false;
	arm_speed = 0;
	arm_speed_inc = 0.05f;

	// Application globals
	model_scale = .1f;
	aspect_ratio = 1.3333f;

	// Lighting globals
	ambient_constant = .2f;
	colourmode = 1;
	drawmode = 0;
	emitmode = 0;
	attenuationmode = 1;
	light_x = -10.15;
	light_y = 1.8;
	light_z = -1.95;
	light2_x = -14;
	light2_y = 1.8;
	light2_z = -1.95;

	// Object globals
	numlats = numlongs = 60;

	// Generate index, Create and make the current VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Load and build all file dependancies
	try {
		cout << "===SHADERS===" << endl;
		//shaders
		programs[0] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_phong.frag");
		programs[1] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_oren_nayar.frag");
		programs[2] = glw->LoadShader("..\\..\\shaders\\assignment2.vert", "..\\..\\shaders\\assignment2_cook_torrance.frag");

		programs[3] = glw->LoadShader("..\\..\\shaders\\terrain.vert", "..\\..\\shaders\\terrain.frag");
		programs[4] = glw->LoadShader("..\\..\\shaders\\LSystem.vert", "..\\..\\shaders\\LSystem.frag");

		cout << "===OBJECTS===" << endl;
		//wavefront .obj objects
		//0-monkey, 1-monkey_normals
		//2-windmill-arms, 3-windmill
		//6-torch, 7-door
		//4-castle_outside, 8-castle_courtyard, 9-castle_roof, 10-castle_walls
		//11-teapot
		windmill.load_obj(objects[3], false, true);
		windmill_arms.load_obj(objects[2], false, true);

		torch.load_obj(objects[6], false, true);
		door.load_obj(objects[7], false, true);
		teapot.load_obj(objects[11], false, true);

		tower.load_obj(objects[7], false, true);
		statue.load_obj(objects[8], false, true);

		castle_outside.load_obj(objects[4], false, true);
		castle_courtyard.load_obj(objects[8], false, true);
		castle_roof.load_obj(objects[9], false, true);
		castle_walls.load_obj(objects[10], false, true);
	} catch (exception & e) {
		cout << "Caught exception: " << e.what() << endl;
		exit(1);
	}
	current_program = 0;

	// Shader error check
	check_for_gl_error();


	// This will flip the image so that the texture coordinates defined in
	// the sphere, match the image orientation as loaded by stb_image
	//stbi_set_flip_vertically_on_load(true);

	// Load textures
	cout << "===TEXTURES===" << endl;
	for (int i = 0; i < texture_amount; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		if (!load_texture(image_files[i], textureID[i], true))
		{
			cout << "Fatal error loading texture: " << image_files[i] << endl;
			exit(1);
		}
		else {
			cout << "successfully loaded texture: " << image_files[i] << endl;
		}
	}
	// Textures error check
	check_for_gl_error();


	cout << "===UNIFORMS===" << endl;
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
		int loc = glGetUniformLocation(programs[i], "tex1");
		if (loc >= 0) { glUniform1i(loc, 0); }
	}
	// Uniforms error check
	check_for_gl_error();

	the_tree.init(glw,textureID[6],colourmode, programs[4]);

	cout << "===TERRAIN===" << endl;
	{
		// Terrain object
		octaves = 4;
		perlin_scale = 3.f;
		perlin_frequency = 1.f;
		land_size = 20.f;
		land_resolution = 1000;
		heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
		heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size, sealevel);
		heightfield->setColourBasedOnHeight();
		heightfield->createObject();
	}
	// Terrain error check
	check_for_gl_error();

	cout << "===PROGRAMMATIC OBJECTS===" << endl;
	// Create programatic objects
	aSphere.makeSphere(numlats, numlongs);

	// Print shader data
	for (int i = 0; i < program_amount; i++)
	{
		GLint x1;
		GLint* px = &x1;
		glGetProgramiv(programs[i], GL_ATTACHED_SHADERS, px);//can change GL_ATTACHED_SHADERS to some program property from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetProgram.xhtml
		cout << "program " << i << ", attached shaders: " << x1 << endl;
	}

	// Programatic objects error check
	check_for_gl_error();

	// Show user controls
	show_controls();
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
	mat4 projection = perspective(radians(45.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	vec3 player_position;
	if(locked_view){
		player_position = vec3(7, get_terrain_position(heightfield, 7, -7) - .1f, -7);
	}else if (locked_teapot) {
		vec3 aim = teapots[teapot_target];
		player_position = vec3(aim.x, aim.y, aim.z);
	} else {
		player_position = vec3(view_move_x, view_move_y, view_move_z);
	}
	vec3 forward = vec3(x,y,z);
	mat4 view = lookAt(
		forward,//at
		player_position,//looking at
		vec3(0, 1, 0)//y axis facing
	);

	// Light sources
	vec4 lightpos1 = view * vec4(light_x, light_y, light_z, 1.0);
	vec4 lightpos2 = view * vec4(light2_x, light2_y, light2_z, 1.0);
	vec4 the_sun = view * vec4(0.0, 5.0, 0.0, 0.0);
	vec4 light_positions[3] = { lightpos1, lightpos2, the_sun };

	// Send our uniforms variables to the currently bound shader
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[current_program], 2, value_ptr(light_positions[0]));
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniform1ui(attenuationmodeID[current_program], attenuationmode);
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
	GLuint castle_program = 0;
	glUseProgram(programs[castle_program]);
	model.push(model.top());
	{

		// Transformations
		float castle_x = -10;
		float castle_z = 0;
		model.top() = translate(model.top(), vec3(castle_x, get_terrain_position(heightfield, 0, 0) + 1.f, castle_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));

		// Uniforms
		glUniformMatrix4fv(modelID[castle_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[castle_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		glBindTexture(GL_TEXTURE_2D, textureID[3]);//BRICK

		// Draw object
		castle_outside.drawObject(drawmode);
	}
	model.pop();
	check_for_gl_error();
	model.push(model.top());
	{
		GLuint castle_program = 0;
		glUseProgram(programs[castle_program]);

		// Transformations
		float castle_x = -10;
		float castle_z = 0;
		model.top() = translate(model.top(), vec3(castle_x, get_terrain_position(heightfield, 0, 0) + 1.f, castle_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));

		// Uniforms
		glUniformMatrix4fv(modelID[castle_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[castle_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		glBindTexture(GL_TEXTURE_2D, textureID[3]);//STONE

		// Draw object
		castle_courtyard.drawObject(drawmode);
	}
	model.pop();
	check_for_gl_error();
	model.push(model.top());
	{
		GLuint castle_program = 0;
		glUseProgram(programs[castle_program]);

		// Transformations
		float castle_x = -10;
		float castle_z = 0;
		model.top() = translate(model.top(), vec3(castle_x, get_terrain_position(heightfield, 0, 0) + 1.f, castle_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));

		// Uniforms
		glUniformMatrix4fv(modelID[castle_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[castle_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//glBindTexture(GL_TEXTURE_2D, textureID[3]);//ROOFTILE

		// Draw object
		castle_roof.drawObject(drawmode);
	}
	model.pop();
	check_for_gl_error();
	model.push(model.top());
	{
		GLuint castle_program = 0;
		glUseProgram(programs[castle_program]);

		// Transformations
		float castle_x = -10;
		float castle_z = 0;
		model.top() = translate(model.top(), vec3(castle_x, get_terrain_position(heightfield, 0, 0) + 1.f, castle_z));
		model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));

		// Uniforms
		glUniformMatrix4fv(modelID[castle_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[castle_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//glBindTexture(GL_TEXTURE_2D, textureID[3]);//BRICK

		// Draw object
		castle_walls.drawObject(drawmode);
	}
	model.pop();
	glUseProgram(programs[current_program]);
	check_for_gl_error();
	model.push(model.top());
	{
		//GLuint door_program = 1;
		//glUseProgram(programs[door_program]);
		// Transformations
		model.top() = translate(model.top(), vec3(-14, 1.5, -1.75));
		model.top() = scale(model.top(), vec3(model_scale / 2, model_scale / 2, model_scale / 2));
		model.top() = rotate(model.top(), -radians(90.f), vec3(0, 1, 0));

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//glBindTexture(GL_TEXTURE_2D, textureID[6]);//DOOR
		teapot.overrideColour(vec4(0.f, 0.f, 1.f, 1.f));

		// Draw object
		door.drawObject(drawmode);
		//glUseProgram(programs[current_program]);
	}
	model.pop();
	check_for_gl_error();
	model.push(model.top());
	{
		//GLuint door_program = 1;
		//glUseProgram(programs[door_program]);
		// Transformations
		model.top() = translate(model.top(), vec3(0, 0, 0));
		//model.top() = scale(model.top(), vec3(model_scale / 3, model_scale / 3, model_scale / 3));
		//model.top() = rotate(model.top(), -radians(90.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//glBindTexture(GL_TEXTURE_2D, textureID[6]);//DOOR

		// Draw object
		tower.drawObject(drawmode);
		//glUseProgram(programs[current_program]);
	}
	model.pop();
	check_for_gl_error();
	model.push(model.top());
	{
		//GLuint door_program = 1;
		//glUseProgram(programs[door_program]);
		// Transformations
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		//model.top() = scale(model.top(), vec3(model_scale / 3, model_scale / 3, model_scale / 3));
		//model.top() = rotate(model.top(), -radians(90.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

		// Uniforms
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Textures
		//glBindTexture(GL_TEXTURE_2D, textureID[6]);//DOOR

		// Draw object
		//door.drawObject(drawmode);
		//glUseProgram(programs[current_program]);
	}
	model.pop();
	check_for_gl_error();



	//windmill
	float windmill_x = 5.4;
	float windmill_z = -7;
	model.push(model.top());
	{
		model.push(model.top());
		{
			// Transformations
			model.top() = translate(model.top(), vec3(windmill_x, get_terrain_position(heightfield, windmill_x, windmill_z) - .1f, windmill_z));
			model.top() = scale(model.top(), vec3(model_scale * 4, model_scale * 4, model_scale * 4));
			model.top() = rotate(model.top(), -radians(120.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

			// Uniforms
			glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

			// Textures
			teapot.overrideColour(vec4(0.f, 0.f, 1.f, 1.f));

			// Draw object
			windmill.drawObject(drawmode);
		}
		model.pop();
		check_for_gl_error();
		model.push(model.top());
		{
			// Transformations
			model.top() = translate(model.top(), vec3(windmill_x, get_terrain_position(heightfield, windmill_x, windmill_z) + 1.9f, windmill_z));
			model.top() = rotate(model.top(), -radians(120.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis
			model.top() = rotate(model.top(), -radians(2*arm_speed), vec3(1, 0, 0)); //rotating in clockwise direction around z-axis
			model.top() = scale(model.top(), vec3(model_scale * 4, model_scale * 4, model_scale * 4));
			
		
			// Uniforms
			glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

			// Textures
			teapot.overrideColour(vec4(.8f, .8f, .8f, 1.f));

			// Draw object
			windmill_arms.drawObject(drawmode);
		}
		model.pop();
	}
	model.pop();
	check_for_gl_error();

	//Torch
	vec3 light_positions_2[2] = {
		vec3(light_x, light_y, light_z),
		vec3(light2_x, light2_y, light2_z)
	};
	for (int i = 0; i < 2; i++) {
		model.push(model.top());
		{
			// Transformations
			//light_x = -10.1;
			//light_y = 1.8;
			//light_z = -1.95;
			model.top() = translate(model.top(), vec3(light_positions_2[i].x, light_positions_2[i].y, light_positions_2[i].z));
			model.top() = scale(model.top(), vec3(model_scale / 3, model_scale / 3, model_scale / 3));
			model.top() = rotate(model.top(), -radians(90.f), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

			// Uniforms
			glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

			// Textures
			glBindTexture(GL_TEXTURE_2D, textureID[7]);

			// Draw object
			torch.drawObject(drawmode);
		}
		model.pop();
		check_for_gl_error();
		// Lightsource 1
		model.push(model.top());
		{
			// Transformations
			model.top() = translate(model.top(), vec3(light_positions_2[i].x, light_positions_2[i].y+.05f, light_positions_2[i].z));
			model.top() = scale(model.top(), vec3(0.035f, 0.035f, 0.035f)); // make a small sphere

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
	}



	


	// Teapots
	GLuint teapot_program = 1;
	glUseProgram(programs[teapot_program]);

	glUniformMatrix4fv(viewID[teapot_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[teapot_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[teapot_program], 2, value_ptr(light_positions[0]));
	glUniform1ui(colourmodeID[teapot_program], colourmode);
	glUniform1ui(attenuationmodeID[teapot_program], attenuationmode);
	emitmode = 0;
	glUniform1ui(emitmodeID[teapot_program], emitmode);
	glUniform1f(ambient_constantID[teapot_program], ambient_constant);

	for (int i = 0; i < 4; i++) {
		model.push(model.top());
		{
			// Transformations
			model.top() = translate(model.top(), teapots[i]);
			model.top() = scale(model.top(), vec3(model_scale * 5, model_scale * 5, model_scale * 5));//scale equally in all axis
			model.top() = rotate(model.top(), -radians(arm_speed), vec3(0, 1, 0)); //rotating in clockwise direction around z-axis

			// Uniforms
			glUniformMatrix4fv(modelID[teapot_program], 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID[teapot_program], 1, GL_FALSE, &normalmatrix[0][0]);

			// Textures
			teapot.overrideColour(vec4(.8f, .8f, .8f, 1.f));

			// Draw object
			teapot.drawObject(drawmode);
		}
		model.pop();
	}
	glUseProgram(programs[current_program]);
	check_for_gl_error();


	the_tree.draw_ltree(projection, view, 0, 0, 0);

	//animation
	arm_speed += arm_speed_inc;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}


static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods){
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// activate
	if (key == 'F' && action == GLFW_PRESS) {
		//if in area 2 bounds
		if (x > -24.8 && x < -14.4 && z > -6.8f && z < 3.6f) {
			//if active
			if (locked_teapot) {
				//increment target
				if (teapot_target < 3) {
					teapot_target++;
				} else {
					//end
					locked_teapot = false;
				}
			} else {
				//start
				locked_teapot = true;
				teapot_target = 0;
			}
		} else {
			//focus on windmill
			locked_view = !locked_view;
		}
		//if in area 1
		//if in door area
		//playdoor anim

		//if(locked_view){
		//cannot turn
		//cannot change view y
		//cannot reset view y
		//}
	}

	//move
	//turn left/right
	if (key == 'Q' && action == GLFW_PRESS && !locked_view) {
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
	if (key == 'E' && action == GLFW_PRESS && !locked_view) {
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
	if (key == 'Z' && view_move_y < 3.6f && !locked_view) { view_move_y += 0.05f; }//up
	if (key == 'X' && view_move_y > -1.8f && !locked_view) { view_move_y -= 0.05f; }//down
	//reset to normal
	if (key == 'V' && action == GLFW_PRESS && !locked_view) {
		view_move_y = 1.8f;
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
	if (key == '0' && action != GLFW_PRESS)
	{
		current_program++;
		if (current_program > 2) current_program = 0;
		cout << "current_program=" << current_program << endl;
	}
	
	//DEBUG CONTROLS
	if (key == 'B' && action != GLFW_PRESS) {
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
	if (key == 'N' && action != GLFW_PRESS) {
		check_program_log();
	}
	//current position
	if (key == 'M' && action != GLFW_PRESS) {
		cout << "view_move_x: " << view_move_x << endl;
		cout << "view_move_y: " << view_move_y << endl;
		cout << "view_move_z: " << view_move_z << endl;
		cout << "x: " << x << endl;
		cout << "y: " << y << endl;
		cout << "z: " << z << endl;
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
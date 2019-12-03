//vertex shader AC41001 Assignment 2
//Deren Vural 29/11/2019

// Specify minimum OpenGL version
#version 440 core

//vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 textures;
layout(location = 3) in vec4 colour;

//out
out vert_data{
	vec4 vert_pos;
	vec3 vert_normal;

	vec2 vert_tex_coord;
	vec4 vert_colour;

	vec3 light_direction1, light_direction2;
	vec4[3] light_directions;
};

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalmatrix;

uniform vec4[3] light_positions;

uniform uint colourmode;

void main()
{
	//custom colours
	if (colourmode == 1){
		vert_colour = colour;
	}else{
		vert_colour = vec4(1.0, 0, 0, 1.0);
	}

	//homogeneous coords
	vec4 position_h = vec4(position, 1.0);

	//precalculated lighting vectors
	mat4 mv_matrix = view * model;
	vert_pos = mv_matrix * position_h;
	vert_normal = normalize(normalmatrix * normal);

	//calculate multiple light source directions
	light_direction1 = vec3(mv_matrix*(light_positions[0] - vert_pos));
	light_direction2 = vec3(mv_matrix*(light_positions[1] - vert_pos));
	
	for(int i=0;i<3;i++){
		light_directions[i] = mv_matrix*(light_positions[i] - vert_pos);
	}

	gl_Position = (projection * view * model) * position_h;

	// Output the texture coordinates
	vert_tex_coord = textures.xy;
}
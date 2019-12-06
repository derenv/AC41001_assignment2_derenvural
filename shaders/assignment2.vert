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

	vec4[3] light_directions;
};

//==TEST==
//struct light_data{
//	vec4 light_position;
//    vec3 colour;
//	float ambient_coefficient;
//
//	float light_attenuation;//for point light
//	float cone_angle;//for point light
//	vec3 cone_direction;//for point light
//};
//uniform light_data[3] light_directions2;
//==TEST==

//uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalmatrix;

uniform vec4[3] light_positions;

uniform uint colourmode;

void main()
{
	// Colours & textures
	if (colourmode == 1){
		vert_colour = colour;
	}else{
		vert_colour = vec4(1.0, 0, 0, 1.0);
	}

	// Homogeneous coords
	vec4 position_h = vec4(position, 1.0);

	// Precalculated lighting vectors
	mat4 mv_matrix = view * model;
	vert_pos = mv_matrix * position_h;
	vert_normal = normalize(normalmatrix * normal);

	// Calculate multiple light source directions
	for(int i=0;i<3;i++){
		//if(light_positions[i].w == 0){
		// Directional
		//light_directions[i] = mv_matrix * light_positions[i];//direction not position so just passed through (normalised in fragment shader)
		//}else{
		// Positional - work out direction from position
		light_directions[i] = mv_matrix * (light_positions[i] - vert_pos);
		//}
	}
	
	// Output the coordinates
	gl_Position = (projection * view * model) * position_h;

	// Output the texture coordinates
	vert_tex_coord = textures.xy;
}
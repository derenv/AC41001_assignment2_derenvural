//fragment shader AC41001 Assignment 1
//Deren Vural 2/11/2019
//based on initial examples

// Specify minimum OpenGL version
#version 440 core

//in
in vert_data{
	vec4 vert_pos;
	vec3 vert_normal;

	vec2 vert_tex_coord;
	vec4 vert_colour;

	vec3 light_direction1, light_direction2;
	vec4[3] light_directions;
};


//out
out vec4 outputColor;

//globals
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);
int  shininess = 8;

//uniforms
uniform uint attenuationmode;
uniform float ambient_constant;
uniform uint emitmode;

vec3 calc_diffuse(vec4 light_direction){
	return max(dot(vert_normal, normalize(light_direction.xyz)), 0.0) * vert_colour.xyz;
}

float calc_attenuation(vec4 light_direction){
	float attenuation_k1 = 0.5;
	float attenuation_k2 = 0.5;
	float attenuation_k3 = 0.5;
	float distanceToLight = length(light_direction);

	return 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + attenuation_k3 * pow(distanceToLight, 2));
}

vec3 calc_specular(vec4 light_direction){
	vec3 V = normalize(-vert_pos.xyz);	
	vec3 R = reflect(-normalize(light_direction.xyz), vert_normal);
	return pow(max(dot(R, V), 0.0), shininess) * specular_albedo;
}

void main()
{
	// Ambient
	vec3 ambient = vert_colour.xyz * ambient_constant;

	// Emissive
	vec4 emissive = vec4(0);
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 0.8, 1.0);
	
	outputColor = vec4(emissive + global_ambient);

	// For each light direction
	for(int i=0;i<3;i++){
		// Calculate diffuse
		vec3 diffuse = calc_diffuse(light_directions[i]);

		// Calculate specular
		vec3 specular = calc_specular(light_directions[i]);

		// Calculate attenuation
		float attenuation;
		if (attenuationmode != 1 || light_directions[i].w == 0.0)
		{
			attenuation = 1.0;
		}
		else
		{
			attenuation = calc_attenuation(light_directions[i]);
		}
		
		// Calculate current source
		vec3 source = attenuation*(ambient + diffuse + specular);

		// Add to total lighting
		outputColor = outputColor + vec4(source, 1);
	}
}
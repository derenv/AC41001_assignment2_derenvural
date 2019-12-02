//fragment shader AC41001 Assignment 1
//Deren Vural 2/11/2019
//based on initial examples

// Specify minimum OpenGL version
#version 440 core

//in
in vert_data{
	vec4 vert_pos, vert_colour;
	vec3 vert_normal, light_direction1, light_direction2;
};

//out
out vec4 outputColor;

//globals
vec3 specular_albedo = vec3(1.0, 0.8, 0.6);
vec3 global_ambient = vec3(0.05, 0.05, 0.05);
int  shininess = 8;

//uniforms
uniform float ambient_constant;
uniform uint emitmode;

vec3 calc_diffuse(vec3 light_direction){
	return max(dot(vert_normal, normalize(light_direction)), 0.0) * vert_colour.xyz;
}

float calc_attenuation(vec3 light_direction){
	float attenuation_k1 = 0.5;
	float attenuation_k2 = 0.5;
	float attenuation_k3 = 0.5;
	float distanceToLight = length(light_direction);

	return 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + attenuation_k3 * pow(distanceToLight, 2));
}

vec3 calc_specular(vec3 light_direction){
	vec3 V = normalize(-vert_pos.xyz);	
	vec3 R = reflect(-normalize(light_direction), vert_normal);
	return pow(max(dot(R, V), 0.0), shininess) * specular_albedo;
}

void main()
{
	//lambertian diffuse
	vec3 diffuse1 = calc_diffuse(light_direction1);
	vec3 diffuse2 = calc_diffuse(light_direction2);
	
	//ambient
	vec3 ambient = vert_colour.xyz * ambient_constant;

	//phong specular
	vec3 specular1 = calc_specular(light_direction1);
	vec3 specular2 = calc_specular(light_direction2);

	//emissive
	vec3 emissive = vec3(0);
	if (emitmode == 1) emissive = vec3(1.0, 1.0, 1.0); 
	
	//attenuation
	float attenuation1 = calc_attenuation(light_direction1);
	float attenuation2 = calc_attenuation(light_direction1);
	
	//source
	vec3 source1 = attenuation1*(ambient + diffuse1 + specular1);
	vec3 source2 = attenuation2*(ambient + diffuse2 + specular2);

	outputColor = vec4(source1 + source2 + emissive + global_ambient, vert_colour.z);
}
//fragment shader AC41001 Assignment 1
// Fragment shader that implements Oren Nayar (diffuse)
// per-fragment lighting for multiple light sources
// based on Oren Nayar shader from Iain Martin
//Deren Vural 12/11/2019

// Specify minimum OpenGL version
#version 440 core

//in
in vert_data{
	vec4 vert_pos;
	vec3 vert_normal;

	vec2 vert_tex_coord;
	vec4 vert_colour;

	vec4[3] light_directions;
};

//out
out vec4 outputColor;

//globals
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);
const float PI = 3.141592653;
const float roughness = 0.5;
float attenuation_k = 0.5;

//uniforms
uniform uint attenuationmode;
uniform uint emitmode;
uniform float ambient_constant;

uniform sampler2D tex1;

vec4 calc_oren_nayar(vec4 light_direction, vec4 emissive, vec4 ambient, vec3 normal, vec3 view_dir){
	//calculate light info
    vec3 light_dir = normalize(light_direction.xyz);
	
	// Calculate these dot products which are used in the Oren Nayar equations below
    float NdotL = dot(normal, light_dir);
    float NdotV = dot(normal, view_dir); 

    float angleVN = acos(NdotV);
    float angleLN = acos(NdotL);

	// Implement the Oren Nayar equations
    float alpha = max(angleVN, angleLN);
    float beta = min(angleVN, angleLN);
    float gamma = dot(view_dir - normal * dot(view_dir, normal), light_dir - normal * dot(light_dir, normal));

    float roughnessSquared = roughness * roughness;
    float roughnessSquared9 = (roughnessSquared / (roughnessSquared + 0.09));

    // calculate C1, C2 and C3
    float C1 = 1.0 - 0.5 * (roughnessSquared / (roughnessSquared + 0.33));
    float C2 = 0.45 * roughnessSquared9;

    if(gamma >= 0.0) {
        C2 *= sin(alpha);
    } else {
        C2 *= (sin(alpha) - pow((2.0 * beta) / PI, 3.0));
    }

    float powValue = (4.0 * alpha * beta) / (PI * PI);
    float C3  = 0.125 * roughnessSquared9 * powValue * powValue;

    // now calculate both main parts of the formula
    float A = gamma * C2 * tan(beta);
    float B = (1.0 - abs(gamma)) * C3 * tan((alpha + beta) / 2.0);

    // put it all together
    float L1 = max(0.0, NdotL) * (C1 + A + B);

    // also calculate interreflection
    float twoBetaPi = 2.0 * beta / PI;

    float L2 = 0.17 * max(0.0, NdotL) * (roughnessSquared / (roughnessSquared + 0.13)) * (1.0 - gamma * twoBetaPi * twoBetaPi);

    vec4 oren_nayar = vec4(vert_colour.xyz * (L1 + L2), 1.0);

	return oren_nayar;
}

float calc_attenuation(vec4 light_direction){
	float distanceToLight = length(light_direction);
		
	return 1.0 / (attenuation_k + attenuation_k*distanceToLight + attenuation_k * pow(distanceToLight, 2));
}


void main()
{
	// Ambient
	vec4 ambient = vert_colour * ambient_constant;

	// Emissive
	vec4 emissive = vec4(0);
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 0.8, 1.0);
	
	// Normalise our input vectors, these may be the same for other BRDFs
	vec3 normal = normalize(vert_normal);
    vec3 view_dir = normalize(-vert_pos.xyz);

	// Initialise output
	outputColor = vec4(emissive + global_ambient);
	
	// For each light direction
	for(int i=0;i<3;i++){
		// Calculate current source
		vec4 oren_nayar = calc_oren_nayar(light_directions[i], emissive, ambient, normal, view_dir);
	
		// Calculate the attenuation factor
		float attenuation;
		if (attenuationmode != 1 || light_directions[i].w == 0)
		{
			attenuation = 1.0;
		}
		else
		{
			attenuation = calc_attenuation(light_directions[i]);
		}
	
		// Calculate current source
		vec4 source = attenuation * oren_nayar;
		
		// Load texture using sample
		vec4 texcolour = texture(tex1, vert_tex_coord);

		// Add to total lighting
		outputColor = (outputColor + source) * texcolour;
	}
}
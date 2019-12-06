/*
fragment shader AC41001 Assignment 2
Deren Vural 03/12/2019
equation from: https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
method from: https://stackoverflow.com/questions/24096041/how-to-properly-implement-cook-torrance-shading-in-three-js

Cook-Torrance Equation:
f(1,v) = D(h)F(v,h)G(l,v,h)
				/
			4(n.l)(n.v)
(where alpha = roughness^2)

D[Bekmann](m) = (alpha^2 / (PI * alpha^2 * (n.m)^4)) * exp[((n.m^2 - 1) / alpha^2 * (n.m)^2)]
G[cook-torrance](l,v,h) = min[1,(2(n.h)(n.v)/v.h),(2(n.h)(n.l)/v.h)]
F[cook-torrance](v,h) = (1/2) * ((g-c)/(g+c))^2 * (1+((g+c)*c - 1 / (g-c)c + 1)^2)
*/

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
vec3 specular_albedo = vec3(.5, .5, .5);
int shininess = 8;
const float PI = 3.141592653;
float roughness = 0.5f;
float F0 = 0.8f;
float attenuation_k = 1.5;

//uniforms
uniform uint attenuationmode;
uniform uint emitmode;
uniform float ambient_constant;

// Microfacet Distribution (by beckmann)
float beckmann(float NdotH){
	float r1 = 1.0f / (4.0f * pow(roughness,2) * pow(NdotH,4.0f) * PI);
	float r2 = exp(-pow(tan(acos(NdotH)),2.0) / (pow(roughness,2)));
	return r1 * r2;
}

// Geometric Shadowing
float geometric_shadow(float NdotH, float NdotV, float VdotH, float NdotL){
	float g1 = (2.0f * NdotH * NdotV) / VdotH;
	float g2 = (2.0f * NdotH * NdotL) / VdotH;
	return min(1.0f, min(g1, g2));
}

// Fresnel Reflectance
vec3 fresnels(float VdotH){
	float F = pow(1.0f - VdotH, 5.0);
	F *= (1.0f - F0);
	F += F0;

	return F * vert_colour.xyz;
}

vec4 CookTorrance(vec4 light_direction){
	vec3 cook_torrance = vec3(0);

	vec3 n = normalize(vert_normal);
	vec3 v = normalize(vert_pos.xyz);
	vec3 l = normalize(light_direction.xyz);
	vec3 h = normalize(v + l);

	float NdotH = max(0, dot(n, h));
	float VdotH = max(0, dot(v, h));
	float NdotV = max(0, dot(n, v));
	float NdotL = max(0, dot(n, l));
	if(NdotL > 0 && NdotV > 0){
		cook_torrance = (beckmann(NdotH) * geometric_shadow(NdotH, NdotV, VdotH, NdotL) * fresnels(VdotH)) / (NdotL * NdotV);
	}
	
	vec3 beta = vec3(1.0,1.0,1.0) / (4.0 * PI * pow(length(light_direction),2.0));
	float ambient = 0.2;
	vec3 Kd = vert_colour.xyz;
	float s = 0.2;
	vec4 xcol = vec4(beta * NdotL * ((1.0-s)*Kd + s*cook_torrance) + ambient*Kd, 1.0);

	return xcol;
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

	// Initialise output
	outputColor = vec4(emissive + global_ambient);
	
	// For each light direction
	for(int i=0;i<3;i++){
		// Calculate current source
		vec4 cook_torrance = CookTorrance(light_directions[i]);// * vec4(specular_albedo,1.0);
	
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
		vec4 source = attenuation * (ambient + cook_torrance);

		// Add to total lighting
		outputColor = outputColor + source;
	}
}
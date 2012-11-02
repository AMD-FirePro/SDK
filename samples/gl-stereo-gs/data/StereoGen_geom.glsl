#version 420 core

//#define POSITION		0
//#define NORMAL			1
////#define TEXCOORD    2

//#define MATERIAL    3
#define LIGHT       4
#define TRANSFORM   5

layout(triangles) in;
in vec3 normalLeft[];
in vec3 normalRight[];
in vec4 color[];

layout(triangle_strip, max_vertices=6) out;

const int numLights = 1;

layout(binding = LIGHT)uniform Light
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 attenuation;
} light[1];  

//
//uniform Matetial
//{
//  vec4 ambient;
//  vec4 diffuse;
//  vec4 specular;
//  vec4 emission;
//  float shininess;
//} material;

layout(binding = TRANSFORM) uniform transform
{
	mat4 MVLeft;
	mat4 MVRight;
	mat4 PLeft;
	mat4 PRight;
} Transform;

out my_PerVertex
{
	vec4 Color;
} Out;

float calculateAttenuation(in int i, in float dist)
{
	return(1.0 / (light[i].attenuation.x +
		light[i].attenuation.y * dist +
		light[i].attenuation.z * dist * dist));
}

void pointLight(
	in int i, in vec3 N, in vec3 V, in float shininess,
	inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	//we don't move the light!!!
	vec3 D = light[i].position.xyz - V;
	vec3 L = normalize(D);

	float dist = length(D);
	float attenuation = calculateAttenuation(i, dist);

	float nDotL = dot(N,L);

	if (nDotL > 0.0  )
	{
		float pf = pow(max(dot( N, normalize(vec3(0.0,0.0,1.0) + L )),0.0), shininess);

		diffuse  += light[i].diffuse  * attenuation * nDotL;
		specular += light[i].specular * attenuation * pf;
	}

	ambient  += light[i].ambient * attenuation;
}

void calculateLighting(
	in int nLights, in vec3 N, in vec3 V, in float shininess,
	inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	// Just loop through each light, and if its enabled add
	// its contributions to the color of the pixel.
	for (int i = 0; i < nLights; i++)
		pointLight(i, N, V, shininess, ambient, diffuse, specular);
}

void main()
{
	//here you need to apply any algorithm to compute the correct normal and color, for example flipping the normal according to the light direction
	vec4 leftEyeVertiecs[3];
	vec4 leftEyeColor[3];
	for(int i=0; i<3;i++)
	{ 
		vec4 ambientL  = vec4(0.0);
		vec4 diffuseL  = vec4(0.0);
		vec4 specularL = vec4(0.0);

		vec4 ambientR  = vec4(0.0);
		vec4 diffuseR  = vec4(0.0);
		vec4 specularR = vec4(0.0);

		//we will apply some basic lighting
		if(numLights>0)
		{
			calculateLighting(numLights, normalLeft[i], (Transform.MVLeft * gl_PositionIn[i]).xyz, /*material.shininess*/3.0,
			ambientL, diffuseL, specularL);
			calculateLighting(numLights, normalRight[i], (Transform.MVRight * gl_PositionIn[i]).xyz, /*material.shininess*/3.0,
				ambientR, diffuseR, specularR);

			vec4 TempcolorL;
			vec4 TempcolorR;

			TempcolorL = (ambientL  * color[i]) + (diffuseL  * color[i]) +(specularL * vec4(0.2,0.2,0.0,0.0));
			TempcolorR = (ambientR  * color[i]) + (diffuseR  * color[i]) + (specularR * vec4(0.2,0.2,0.0,0.0));

			Out.Color = TempcolorR;
			leftEyeColor[i] = TempcolorL;
		}
		else
		{
			Out.Color = color[i];
			leftEyeColor[i] = color[i];
		}

		gl_Position = Transform.PRight*Transform.MVRight *gl_PositionIn[i];
		leftEyeVertiecs[i] = Transform.PLeft*Transform.MVLeft *gl_PositionIn[i];
		gl_Layer = 0;

		EmitVertex(); 
	}

	EndPrimitive();

	for(int i=0; i<3;i++)
	{ 
		Out.Color = leftEyeColor[i];
		gl_Position = leftEyeVertiecs[i];
		gl_Layer = 1;
		EmitVertex();
	}
	EndPrimitive();
}

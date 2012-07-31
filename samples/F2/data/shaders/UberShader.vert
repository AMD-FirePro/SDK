#version 420 core

out vec3 v_normal;
out vec3 v_vertex;
out vec4 v_EyeDirection;
out vec4 v_fragCoord;
out vec2 v_texcoord;
// ShadowMap
out vec4 v_projCoordSM;


uniform bool bTex;
uniform bool bBumpMapping;
uniform bool bEnvTex;
uniform bool bShadowMap;

uniform mat4 matModelViewProj;
uniform mat4 matModelView;
uniform mat3 matNormal;
uniform mat4 matShadow;


#define IN_POSITION	   0
#define IN_NORMAL  	   1
#define IN_TEXCOORD	   2
//#define IN_TANGENT     3
//#define IN_BITANGENT   4

layout(location = IN_POSITION)   in vec3 Position;
layout(location = IN_TEXCOORD)   in vec3 TexCoord;
layout(location = IN_NORMAL)     in vec3 Normal;
//layout(location = IN_TANGENT)    in vec3 vTangentMS;
//layout(location = IN_BITANGENT)  in vec3 vBitangentMS;

void main()
{
	gl_Position = matModelViewProj * vec4(Position, 1.0);
	v_fragCoord = gl_Position;

	// set eye information
	vec4 eyePosition = matModelView * vec4(Position, 1.0);
	v_EyeDirection.xyz = normalize(eyePosition.xyz);

	// shadowmap transformed
	if (bShadowMap)
		v_projCoordSM = matShadow * eyePosition;

	// Calculate the normal
	v_normal = normalize(matNormal * Normal);

	// Transform the vertex position to eye space
	v_vertex = vec3(matModelView * vec4(Position, 1.0));

	if (bTex || bBumpMapping)
		v_texcoord = TexCoord.xy;
}


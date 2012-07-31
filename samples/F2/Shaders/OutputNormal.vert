#version 420 core

//#define POSITION 0
//#define TEXCOORD 1
//#define NORMAL	 2
//
//layout(location = POSITION) in vec3 Position;
//layout(location = TEXCOORD) in vec2 TexCoord;
//layout(location = NORMAL) in vec3 Normal;

#define IN_POSITION	   0
#define IN_NORMAL  	   1
#define IN_TEXCOORD	   2
//#define IN_TANGENT     3
//#define IN_BITANGENT   4

layout(location = IN_POSITION)   in vec3 Position;
layout(location = IN_TEXCOORD)   in vec3 TexCoord;
layout(location = IN_NORMAL)     in vec3 Normal;

uniform mat4 matModelViewProj;
uniform mat3 matNormal;

out vec3 v_Normal;

void main()
{
	// transform the vertex position
	gl_Position = matModelViewProj * vec4(Position, 1.0);
	v_Normal = normalize(matNormal * Normal);
}

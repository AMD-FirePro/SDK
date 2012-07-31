#version 420 core
uniform mat4 matModelViewProj;

#define POSITION 0
#define TEXCOORD 1
#define NORMAL	 2

layout(location = POSITION) in vec3 Position;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = NORMAL) in vec3 Normal;

void main()
{
	// transform the vertex position
	gl_Position = matModelViewProj * vec4(Position, 1.0);
}

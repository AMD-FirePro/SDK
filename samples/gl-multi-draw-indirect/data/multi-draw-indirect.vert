#version 420 core

#define POSITION		0
#define COLOR			3
#define TEXCOORD		4
#define DRAW_ID			5

#define FRAG_COLOR		0

#define TRANSFORM0	1

layout(binding = TRANSFORM0) uniform transform
{
	mat4 MVP;
} Transform;

layout(location = POSITION) in vec3 Position;
layout(location = TEXCOORD) in vec3 Texcoord;
layout(location = DRAW_ID) in int DrawID;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 Texcoord;
	flat uint DrawID;
} Out;

void main()
{
	Out.DrawID = DrawID;
	Out.Texcoord = Texcoord.st;
	gl_Position = Transform.MVP * vec4(Position, 1.0);
}

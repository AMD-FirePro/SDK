#version 420 core

#define POSITION	0
#define COLOR		3
#define TEXCOORD	4
#define FRAG_COLOR	0

in block
{
	vec2 Texcoord;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	vec4 A = mix(vec4(0.0), vec4(1.0), In.Texcoord.x);
	vec4 B = mix(vec4(211.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0), vec4(0.0, 153.0 / 255.0, 102.0 / 255.0, 1.0), In.Texcoord.x);
	Color = mix(A, B, In.Texcoord.y);
}

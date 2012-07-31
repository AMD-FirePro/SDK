#version 420 core


#define IN_FRAG_COLOR 0

#define UNIFORM_UPDATE_EACH_FRAME  0
#define UNIFORM_UPDATE_RESIZE      1


layout(binding = 0) uniform sampler2D texture_Diffuse;

in block
{
	vec2 Texcoord;
} In;

layout(location = IN_FRAG_COLOR, index = 0) out vec4 Color;


void main()
{
	vec4 diffuseTexture = texture2D(texture_Diffuse, In.Texcoord.st);
	Color = diffuseTexture;
}

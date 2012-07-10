#version 420  core

in my_PerVertex
{
	vec4 Color;
} In;

layout(location = 0, index = 0) out vec4 Color;

void main()
{
      Color =In.Color;  
}
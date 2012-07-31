#version 420 core

layout(location = 0) in vec3 Position0;
layout(location = 1) in vec3 Position1;
layout(location = 2) in vec3 Position2;
layout(location = 3) in vec3 Position3;

out block
{
    vec3 Position0;
	vec3 Position1;
	vec3 Position2;
	vec3 Position3;
} Out;

void main()
{	
	Out.Position0 = Position0;
	Out.Position1 = Position1;
	Out.Position2 = Position2;
	Out.Position3 = Position3;
}

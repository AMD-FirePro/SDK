#version 420 core

layout(location = 0) in vec3 Position0;

out block
{
    vec3 Position0;
} Out;

void main()
{	
	Out.Position0 = Position0;
}

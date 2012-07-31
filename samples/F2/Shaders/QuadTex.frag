#version 420 core
uniform sampler2D diffuse2D;

in vec2 v_texcoord;
out vec4 color;


void main()
{
	color = texture2D(diffuse2D, v_texcoord);
}

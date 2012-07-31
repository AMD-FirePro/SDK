#version 420 core

uniform vec4 textureSize;
uniform bool bTex;	// flip UV
out vec2 v_texcoord;

in vec2 position;
in vec2 texcoord;

void main()
{
	vec2 size = textureSize.zw*0.5;
	gl_Position = vec4(position*size+textureSize.xy+vec2(1.0, 1.0), 0.0, 1.0);
	//gl_Position = vec4(position, 0.0, 1.0);
	v_texcoord = texcoord;
	if (bTex)
		v_texcoord.y = 1.0 - v_texcoord.y;

}

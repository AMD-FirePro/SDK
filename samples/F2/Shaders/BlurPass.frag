#version 420 core
uniform sampler2D diffuse2D;
uniform vec4 textureSize;

const int MAX_SAMPLES = 16;
uniform vec2 sampleOffsets[MAX_SAMPLES];
uniform float sampleWeight[MAX_SAMPLES];

uniform int	nbSample;

in vec2 v_texcoord;
out vec4 color;

vec4 BlurFilter(sampler2D s, vec2 position, int nbs, vec2 finalscale)
{
	vec4 vSample = vec4(0.0,0.0,0.0,0.0);
	vec4 vColor = vec4(0.0,0.0,0.0,0.0);

	for (int i = 0 ; i < nbs; ++i)
	{
		vColor =  texture2D(s, position + finalscale*sampleOffsets[i]);
		vSample += sampleWeight[i]*vColor;
	}
	return vSample;
}


void main()
{
	color = BlurFilter(diffuse2D, v_texcoord, nbSample, textureSize.zw);
}

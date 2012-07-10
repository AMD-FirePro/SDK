#version 420 core


uniform sampler2DArray arrayTex;
in vec3 vLeftCoords;
in vec3 vRightCoords;

layout(location = 0, index = 0) out vec4 ColorR;
layout(location = 1, index = 0) out vec4 ColorL;

void main()
{
    ColorL = texture2DArray(arrayTex, vLeftCoords.xyz);
    
    // right eye view
   ColorR = texture2DArray(arrayTex, vRightCoords.xyz);
 
}
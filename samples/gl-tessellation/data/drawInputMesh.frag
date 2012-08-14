#version 420  core

layout(location = 0, index = 0) out vec4 Color;
uniform vec4 wireFrameColor = vec4(1,1,1,1);

void main()
{
      Color = wireFrameColor;  
}
#version 420 core
precision highp float;
out vec4 color0;
layout(r32ui) uniform uimage2D startOffsetBuffer;
void main()
{
    ivec2 offset = ivec2(gl_FragCoord.xy);
    uvec4 vi = uvec4(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);	
    imageStore(startOffsetBuffer, offset, vi);
	color0 = vec4(0.f, 0.f, 0.f, 0.f);
}

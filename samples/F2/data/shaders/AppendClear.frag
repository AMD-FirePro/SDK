#version 420 core
//#define USE_ATOMIC_COUNTER
precision highp float;
out vec4 color0;
layout(r32ui, binding = 0) uniform coherent uimage2D startOffsetBuffer;

#ifndef USE_ATOMIC_COUNTER
layout (r32ui, binding = 2) uniform coherent uimage2D atomicBuff; 
#endif

void main()
{
    ivec2 offset = ivec2(gl_FragCoord.xy);
    uvec4 vi = uvec4(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);	
    imageStore(startOffsetBuffer, offset, vi);

#ifndef USE_ATOMIC_COUNTER
	if (offset == ivec2(0))
		imageStore(atomicBuff, ivec2(0), ivec4(0)); 
#endif

	color0 = vec4(0.f, 0.f, 0.f, 0.f);
}

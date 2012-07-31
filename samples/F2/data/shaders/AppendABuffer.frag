//-------------------------------------------------------------------------------------- 
// Name: FragShader.glsl 
// Desc: Source of Fragment Shaders of the tessellation program 
// 
// Copyright (c) 2008 Advanced Micro Devices, Inc. All rights reserved.
//--------------------------------------------------------------------------------------

#version 130
precision highp float;

in vec4 color;
out vec4 color0;
layout(r32ui) uniform uimage2D startOffsetBuffer;
layout(rgba32ui) uniform uimageBuffer linkedListBuffer;

void main()
{
	vec4 zero = vec4(0.f, 0.f, 0.f, 0.f);
	int vi = 1;	
	uvec4 storeValue;
	ivec2 offset = ivec2(gl_FragCoord.xy);
	const unsigned int id1 = 1u;
	// Retrieve current pixel count and increase counter	
	uint uPixelCount = atomicCounterIncrementARB(id1);
	
	// Exchange indices in StartOffsetTexture corresponding to pixel location 
    uint uOldStartOffset = imageAtomicExchange(startOffsetBuffer, offset, uPixelCount);
	
    int uStartAddr = int(uPixelCount);
	uvec4 uc = uvec4(floor(color*255+0.5));

	storeValue.x = uint((uc.r + (uc.g << 8) + (uc.b << 16) + (uc.a << 24)));    
    storeValue.y = uint(floor((gl_FragCoord.z + 1) *0x7fffffff+0.5));	
    storeValue.z = uOldStartOffset;
    imageStore(linkedListBuffer, uStartAddr, storeValue);
    //uStartAddr++;
    //imageStore(linkedListBuffer, uStartAddr, depth);
    //uStartAddr++;    
    //imageStore(linkedListBuffer, uStartAddr, uOldStartOffset);  
    
    //color write is disabled  
    color0 = zero;
}      	



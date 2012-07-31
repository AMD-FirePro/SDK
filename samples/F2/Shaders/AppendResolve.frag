#version 420 core
#extension GL_ARB_gpu_shader5:enable

precision highp float;
#define MAX_SORTED_FRAGMENTS 20
out vec4 color0;
layout(r32ui) uniform uimage2D startOffsetBuffer;
layout(rgba32ui) uniform uimageBuffer linkedListBuffer;

void main()
{
    int i, j;
    int nNumFragments = 0;
    uvec4 SortedFragments[MAX_SORTED_FRAGMENTS];    
    vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);     
    
    ivec2 coord = ivec2(gl_FragCoord.xy);
    uvec4 vOffset = imageLoad(startOffsetBuffer, coord);  	
    uint uOffset = vOffset.x;
    while (uOffset != 0xFFFFFFFF && nNumFragments < MAX_SORTED_FRAGMENTS)
    {
        SortedFragments[nNumFragments] = imageLoad(linkedListBuffer, int(uOffset));     
        uOffset = SortedFragments[nNumFragments].z;            
        nNumFragments++;
    }
    
    //order
    for (i = 1; i < nNumFragments; i++)
    {
        j = i;
        while ((j>0) && (SortedFragments[j-1].y < SortedFragments[j].y))
        {
            uvec4 tmp = SortedFragments[j];
            SortedFragments[j] = SortedFragments[j-1];
            SortedFragments[j-1] = tmp;
            j--;
        }
    }
           
    //blending
    for (i = 0; i < nNumFragments; i++)
    {
        vec4 src;        
        src = vec4(SortedFragments[i].r&0xff, (SortedFragments[i].r>>8)&0xff, (SortedFragments[i].r>>16)&0xff, (SortedFragments[i].r>>24)&0xff)/255.f;
        color.rgb = src.a * src.rgb + (1.0-src.a) * color.rgb;
		color.a = src.a + (1-src.a)*color.a;
    }            

    color0 = color; 
}


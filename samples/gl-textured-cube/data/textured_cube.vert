#version 420 core

#define IN_POSITION	   0
#define IN_TEXCOORD	   1

#define UNIFORM_UPDATE_EACH_FRAME  0
#define UNIFORM_UPDATE_RESIZE      1


//This struct must be the same that in the cpp file
//comments of each uniform are in the cpp file
layout(shared,  binding = UNIFORM_UPDATE_EACH_FRAME) uniform U_UPDATEEACHFRAME
{
	mat4 mMtoS;
} u_UpdateEachFrame;


layout(location = IN_POSITION)   in vec3 in_Position;
layout(location = IN_TEXCOORD)   in vec2 in_Texcoord;


out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 Texcoord;
} Out;



void main()
{	
	gl_Position = u_UpdateEachFrame.mMtoS * vec4(in_Position.xyz, 1.0f);
	Out.Texcoord = in_Texcoord.xy;
}

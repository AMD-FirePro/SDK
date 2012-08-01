#version 420 core

#define IN_POSITION	   0
#define IN_NORMAL  	   1
#define IN_TEXCOORD	   2
#define IN_TANGENT     3
#define IN_BITANGENT   4

#define UNIFORM_UPDATE_EACH_FRAME  0
#define UNIFORM_UPDATE_RESIZE      1


//This struct must be the same that in the cpp file
//comments of each uniform are in the cpp file
layout(shared,  binding = UNIFORM_UPDATE_EACH_FRAME) uniform U_UPDATEEACHFRAME
{
	mat4 mMtoS;
	vec4 lightDirectionNormalizedWS;
	vec4 viewDirectionNormalizedWS;
	mat4 mWtoM_transposed;
	vec4 lightDiffuseColor;
	vec4 lightAmbientColor;

} u_UpdateEachFrame;



layout(location = IN_POSITION)   in vec3 in_Position;
layout(location = IN_TEXCOORD)   in vec3 in_Texcoord;
layout(location = IN_NORMAL)     in vec3 in_vNormalMS;
layout(location = IN_TANGENT)    in vec3 in_vTangentMS;
layout(location = IN_BITANGENT)  in vec3 in_vBitangentMS;


out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec3 Texcoord;
	vec3 vViewTS;//vector in Tangent Space
	vec3 vLightTS;//vector in Tangent Space
} Out;



void main()
{	

	gl_Position = u_UpdateEachFrame.mMtoS * vec4(in_Position, 1.0f);

	Out.Texcoord = in_Texcoord;

	mat3 mWtoM_transposed_33 = mat3(u_UpdateEachFrame.mWtoM_transposed);

	//here, the model use the B T N axis (sometimes it could be T B N)
	vec3 N_WS = normalize( mWtoM_transposed_33  * in_vNormalMS);
	vec3 T_WS = normalize( mWtoM_transposed_33  * in_vTangentMS);
	vec3 B_WS = cross(T_WS,N_WS);

	mat3 mTtoW_transposed;//matric Tangent Space to World Space, transposed
	mTtoW_transposed[0].xyz = vec3(B_WS.x, T_WS.x, N_WS.x);
	mTtoW_transposed[1].xyz = vec3(B_WS.y, T_WS.y, N_WS.y);
	mTtoW_transposed[2].xyz = vec3(B_WS.z, T_WS.z, N_WS.z);
	
	Out.vViewTS = mTtoW_transposed * u_UpdateEachFrame.viewDirectionNormalizedWS.xyz;
	Out.vLightTS = mTtoW_transposed * u_UpdateEachFrame.lightDirectionNormalizedWS.xyz;
}

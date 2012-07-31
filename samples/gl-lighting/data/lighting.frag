#version 420 core


#define IN_FRAG_COLOR 0

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


layout(binding = 0) uniform sampler2D texture_Diffuse;
layout(binding = 1) uniform sampler2D texture_Normal;
layout(binding = 2) uniform sampler2D texture_Specular;

in block
{
	vec3 Texcoord;
	vec3 vViewTS;//vector in Tangent Space
	vec3 vLightTS;//vector in Tangent Space
} In;

layout(location = IN_FRAG_COLOR, index = 0) out vec4 Color;


/////////////////
//
// vNormalTS : (from normal texture) normalized
// materialDiffuse : color from diffuse texture
// vLightTS : vector from the shape to the light (normalized)
// vViewTS : vector from the shape to the eye (normalized)
// materialSpecular : XYZ: material specular color   W: specular intensity
//
/////////////////
vec3 ComputeIllumination(vec3 vNormalTS, vec3 materialDiffuse, vec3 vLightTS, vec3 vViewTS, vec4 materialSpecular )
{
	//the half vector is the vector between vLightTS and vViewTS (the average vector)
	vec3 halfVector = normalize(vLightTS + vViewTS);

	//the more halfVector is near from normalTS, the more there is specular
	float normalDotHalf = max(dot(vNormalTS,halfVector),0.0f);

	//attenuate the low values
	vec3 specular = clamp( pow( normalDotHalf, materialSpecular.w * 255.0f ),0.0f,1.0f) * materialSpecular.xyz;

	vec3 diffuse = clamp( dot( vNormalTS, vLightTS ),0.0f,1.0f) * u_UpdateEachFrame.lightDiffuseColor.xyz;

	vec3 finalColor = ( u_UpdateEachFrame.lightAmbientColor.xyz + diffuse.xyz ) * materialDiffuse.xyz + specular.xyz ; 

	return finalColor;
}



void main()
{
	vec3 normalTS = normalize(texture2D(texture_Normal, In.Texcoord.st).xyz);
	vec4 materialDiffuse = texture2D(texture_Diffuse, In.Texcoord.st);
	vec4 materialSpecular = texture2D(texture_Specular, In.Texcoord.st); 
	vec3 vLightTS = -normalize(vec3(In.vLightTS.x,In.vLightTS.y,In.vLightTS.z));
	vec3 vViewTS = -normalize(vec3(In.vViewTS.x,In.vViewTS.y,In.vViewTS.z));
	
	Color = vec4(ComputeIllumination( normalTS, materialDiffuse.xyz,  vLightTS,  vViewTS, materialSpecular.xyzw  ),1.0f);
}

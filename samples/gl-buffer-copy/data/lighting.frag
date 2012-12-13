#version 420 core

#define IN_FRAG_COLOR 0

#define UNIFORM_UPDATE_EACH_FRAME  0
#define UNIFORM_UPDATE_RESIZE      1
#define UNIFORM_MATERIAL           2

//These structs must be the same that in the cpp file
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

layout(shared,  binding = UNIFORM_MATERIAL) uniform U_MATERIAL
{
	vec4 diffuse;
	vec4 ambiant;
} u_Material;

layout(binding = 0) uniform sampler2D texture_Diffuse;
layout(binding = 1) uniform sampler2D texture_Normal;
layout(binding = 2) uniform sampler2D texture_Specular;

in block
{
	vec3 Texcoord;
	vec3 vViewTS;//vector in Tangent Space
	vec3 vLightTS;//vector in Tangent Space
	vec3 vNormalWS;
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
	//float specular = clamp( pow( normalDotHalf, materialSpecular.w * 255.0f ),0.0f,1.0f) * materialSpecular.xyz;
	float specular = clamp( pow( normalDotHalf, 80.0f ),0.0f,1.0f);

	vec3 diffuse = clamp( dot( vNormalTS, vLightTS ),0.0f,1.0f) * u_UpdateEachFrame.lightDiffuseColor.xyz;
	vec3 finalColor = ( u_UpdateEachFrame.lightAmbientColor.xyz + diffuse.xyz ) * materialDiffuse + u_Material.ambiant.xyz + specular; 

	//finalColor = clamp( pow( normalDotHalf, 80.0f ),0.0f,1.0f);
	//finalColor = u_Material.ambiant.xyz;

	return finalColor;
}

void main()
{
	vec3 normalTS = vec3(0.0f,0.0f,1.0f);
	#ifdef PRESENCE_NORMALTEXTURE
		normalTS = normalize(texture2D(texture_Normal, In.Texcoord.st).xyz);
	#endif

	vec4 materialTextureDiffuse = vec4(1.0f,1.0f,1.0f,1.0f);
	materialTextureDiffuse = u_Material.diffuse.xyzw;
	#ifdef PRESENCE_DIFFUSETEXTURE
		#ifdef PRESENCE_TEXTCOORD
			materialTextureDiffuse = texture2D(texture_Diffuse, In.Texcoord.st).xyzw;
		#endif
	#endif

	vec4 materialTextureSpecular = vec4(1.0f,1.0f,1.0f,0.8f);
	#ifdef PRESENCE_SPECULARTEXTURE
		materialTextureSpecular = texture2D(texture_Specular, In.Texcoord.st); 
	#endif

	#ifdef PRESENCE_TBN
		vec3 vLightTS = -normalize(vec3(In.vLightTS.x,In.vLightTS.y,In.vLightTS.z));
		vec3 vViewTS = -normalize(vec3(In.vViewTS.x,In.vViewTS.y,In.vViewTS.z));
		Color = vec4(ComputeIllumination( normalTS, materialTextureDiffuse.xyz,  vLightTS,  vViewTS, materialTextureSpecular.xyzw  ),1.0f);
		
		//Color = vec4(1.0f,0.0f,0.0f,1.0f);
	#else
		Color.w = 1.0f;
		Color.xyz = materialTextureDiffuse.xyz  *  max(0.0f,dot(-u_UpdateEachFrame.lightDirectionNormalizedWS.xyz,normalize(In.vNormalWS.xyz)));
		Color.xyz += u_Material.ambiant.xyz;
	#endif

	//Color = vec4(In.vNormalWS.xyz,1.0f);
}

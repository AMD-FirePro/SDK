#version 420 core
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_shader_image_load_store : enable

precision highp float;

layout(r32ui) uniform uimage2D startOffsetBuffer;
layout(rgba32ui) uniform uimageBuffer linkedListBuffer;

layout(binding=0, offset=0) uniform atomic_uint id1;


in vec3 v_normal;
in vec3 v_vertex;
in vec4 v_EyeDirection;
in vec4 v_fragCoord;
in vec2 v_texcoord;
// ShadowMap
in vec4 v_projCoordSM;

out vec4 ColorOut;

uniform int numLights;

uniform bool bTex;
uniform bool bBumpMapping;
uniform bool bShadowMap;
uniform bool bEnvTex;
uniform bool bSSAO;
uniform bool bOIT;
uniform bool bDoubleSided;
uniform float texStrength;		// 0-1 : how much of the diffuse texture take over diffuse
uniform float envStrength;		// 0-1 : how much of the envmap texture take over diffuse
uniform float shadowStrength;	// 0-1 : how much of the dhadow term take over diffuse/specular
uniform float fAlphaOverride;	// dont take alpha from material for extra flexibility

uniform sampler2DShadow shadowMap;
uniform float shadowMapScale; // 1.0/(shadowmap texture size);
uniform samplerCube envMapCube;
uniform sampler2D baseTex;
uniform sampler2D normalMap;
uniform sampler2D occlusionMap;

uniform vec4 FrontMaterial_diffuse;
uniform vec4 FrontMaterial_ambient;
uniform vec4 FrontMaterial_specular;
uniform float FrontMaterial_shininess;

struct LightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;
};

const LightStruct LightSource[3] =
{
	LightStruct(
		vec4(0.35, 0.35, 0.35, 1.0),
		vec4(0.9, 0.85, 0.8, 1.0),
		vec4(0.6, 0.6f, 0.6, 1.0),
		vec4(-0.51832104, 0.60470790, 0.60470790, 0.0)),

	LightStruct(
		vec4(0.35, 0.35, 0.35, 1.0),
		vec4(0.25, 0.3, 0.35, 1.0),
		vec4(0.4, 0.4, 0.4, 1.0),
		vec4(0.59628481, 0.74535602, 0.29814240, 0.0)),

	LightStruct(
		vec4(0.35, 0.35, 0.35, 1.0),
		vec4(0.3, 0.3, 0.3, 1.0 ),
		vec4(0.6, 0.6, 0.6, 1.0),
		vec4(0.34815532, -0.87038827, 0.34815532, 0.0))
};

///////////////////////////////////////////////////////////////////////////////////////////
vec3 getReflection( vec3 viewVec, vec3 eNormal)
{
	return viewVec - 2.0 * eNormal*dot(eNormal, viewVec);
}

////////////////////////////////////////////////////////////////////////
void directionalLight(in int i, in vec3 N, in vec3 V, in float shininess,
					  inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = normalize(LightSource[i].position.xyz);

	float nDotL = dot(N, L);

	if (nDotL > 0.0)
	{   
		vec3 H = normalize(L - V);
		float pf = pow(max(dot(N,H), 0.01), shininess);
		diffuse  += LightSource[i].diffuse  * nDotL;
		specular += LightSource[i].specular * pf;
	}

	ambient  += LightSource[i].ambient;
}


////////////////////////////////////////////////////////////////////////
void calculateLighting(in int nLights, in vec3 N, in vec3 V, in float shininess,
					   inout vec4 ambient, inout vec4 diffuse, inout vec4 specular, in float shadowFromLight0)
{
	// Just loop through each light, and adds its contributions to the color of the pixel.
	for (int i = 0; i < nLights; i++)
	{
		if (bBumpMapping)	// TODO: should be in Tangent space...
		{
			// cheap no-TBN effect
			vec3 Nt = normalize(texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0);
			N.xz += (Nt.xz*0.5);
			N = normalize(N);
		}

		directionalLight(i, N, V, shininess, ambient, diffuse, specular);
		// ShadowMap: this is directional light 0 contribution ONLY
		if (i == 0)
		{
			diffuse *= shadowFromLight0;
			specular *= shadowFromLight0;
		}
	}
}

////////////////////////////////////////////////////////////////////////
// apply a Percentage Close Filtering 4x4 for smoothing shadow edges
float GetShadowColor(sampler2DShadow shadowMap, vec3 shadowUV, float mapScale)
{
	float shadowColor = texture(shadowMap, shadowUV + vec3( -1.5, -1.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -1.5, -0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -1.5,  0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -1.5,  1.5, 0)*mapScale).r;

	shadowColor += texture(shadowMap, shadowUV + vec3( -0.5, -1.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -0.5, -0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -0.5,  0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( -0.5,  1.5, 0)*mapScale).r;

	shadowColor += texture(shadowMap, shadowUV + vec3( 0.5, -1.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 0.5, -0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 0.5,  0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 0.5,  1.5, 0)*mapScale).r;

	shadowColor += texture(shadowMap, shadowUV + vec3( 1.5, -1.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 1.5, -0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 1.5,  0.5, 0)*mapScale).r;
	shadowColor += texture(shadowMap, shadowUV + vec3( 1.5,  1.5, 0)*mapScale).r;
	return shadowColor/16.0;
}

////////////////////////////////////////////////////////////////////////
void AppendABuffer(vec4 color)
{
        color = clamp(color, 0, 1);

        ivec2 offset = ivec2(gl_FragCoord.xy);

        // Retrieve current pixel count and increase counter	
        uint uPixelCount = atomicCounterIncrement(id1);
        	
        // Exchange indices in StartOffsetTexture corresponding to pixel location 
        uint uOldStartOffset = imageAtomicExchange(startOffsetBuffer, offset, uPixelCount);
        	
        int uStartAddr = int(uPixelCount);

		uvec4 storeValue = uvec4(packUnorm4x8(color), floatBitsToUint(gl_FragCoord.z), uOldStartOffset, 0);

        imageStore(linkedListBuffer, uStartAddr, storeValue);
}      	

////////////////////////////////////////////////////////////////////////
void main()
{
	vec4 color;
	// Normalize the normal. A varying variable CANNOT
	// be modified by a fragment shader. So a new variable
	// needs to be created.
	vec3 normal = normalize(v_normal);

	vec2 uv = v_texcoord;

	vec4 ambient  = vec4(0.0);
	vec4 diffuse  = vec4(0.0);
	vec4 specular = vec4(0.0);

	float finalAlpha = fAlphaOverride*FrontMaterial_diffuse.a;

	if (bDoubleSided)
	{
		float checkFace = 0.0;
		if (v_fragCoord.w == 1.0)	// orthographic projection
			checkFace = -normal.z;
		else						// perspective
			checkFace = dot(normal, normalize(v_EyeDirection.xyz));
		if (checkFace > 0.0)	// artifact shown on tangent egdes...
		{
			normal = -normal;
		}
	}

	float shadowTerm = 1.0;	// this is light 0 contribution ONLY
	// attenuate diffuse and specualr contribution according to shadow term
	if (bShadowMap)
	{
		shadowTerm = GetShadowColor(shadowMap, v_projCoordSM.xyz, shadowMapScale);
		shadowTerm = shadowTerm*shadowStrength + (1.0 - shadowStrength);	// make shadow less dark
	}   

	// numLights uniform must be set with the number of lights
	calculateLighting(numLights, normal, v_vertex, FrontMaterial_shininess,
		ambient, diffuse, specular, shadowTerm);


	// modulate with actual material properties
	ambient  *= FrontMaterial_ambient;
	specular *= FrontMaterial_specular;

	// difffuse texture : mix diffuse with texture according to strength
	if (bTex)
	{
		diffuse *= mix (FrontMaterial_diffuse, texture2D(baseTex, v_texcoord), texStrength);
	}
	else
		diffuse *= mix (FrontMaterial_diffuse, vec4(0.2, 0.6, 0.9, 1.0), 1.0 - texStrength);

	// envmap modifier
	if (bEnvTex)
	{
		if (bBumpMapping)
		{
			// cheap no-TBN effect
			vec3 N = normalize(texture2D(normalMap, v_texcoord).xyz * 2.0 - 1.0);
			normal.xz += (N.xz*0.3);
			normal = normalize(normal);
		}

//		vec3 eyeWorldPosition = vec3(0.0,0.0,1.0);	// to be sent as uniform by app for better refl on flat surface
		vec3 viewVec = normalize(v_EyeDirection.xyz - vec3(0.0,0.0,1.0));
		vec3 reflectedVector = reflect(viewVec, normal);
		vec4 envColor = texture(envMapCube, reflectedVector);

		// fresnel + refraction
		if (finalAlpha < 1.0)
		{
			//vec3 refractedVector = refract(viewVec, normal, 0.4);
			//vec4 refractedColor = textureCube(envMapCube, refractedVector);

			float fresnelBias = 0.5;
			float fresnelScale = 0.6;
			float fresnelPower = 1.0;
			// Appoximation of the fresnel equation
			float refFactor = clamp(fresnelBias+fresnelScale*pow(1.0+dot(viewVec,normal), fresnelPower), 0.0, 1.0);
			//float refFactor = 1.0 - clamp(dot(viewVec,-normal) - 0.2, 0.0, 1.0);
			//refFactor = refFactor*refFactor;
			//envColor = mix(refractedColor, envColor, refFactor);
			finalAlpha = min(1.0, refFactor*finalAlpha+specular.r);
		}

		diffuse *= mix (vec4(1.0), envColor*2.5, envStrength);
	}
	else if (finalAlpha < 1.0)	// handle fresnel without envmap
	{
		vec3 viewVec = normalize(v_EyeDirection.xyz - vec3(0.0,0.0,1.0));
		//float fresnelBias = 0.5;
		//float fresnelScale = 0.6;
		//float fresnelPower = 1.0;
		// Appoximation of the fresnel equation
		//float refFactor = clamp(fresnelBias+fresnelScale*pow(1.0+dot(viewVec,normal), fresnelPower), 0.0, 1.0);
		float refFactor = 1.0 - clamp(dot(viewVec,-normal) - 0.3, 0.0, 1.0);
		finalAlpha = min(1.0, refFactor*finalAlpha+specular.r);
	}

	float occlusionTerm = 1.0;
	if (bSSAO)
	{
		vec2 AOcoord = ((v_fragCoord.xy/v_fragCoord.w) + 1.0)/2.0;
		// to be exact, we only kill ambient term with it, but for the sake of showing it...
		occlusionTerm = (texture2D(occlusionMap, AOcoord).r);
	}

	color = occlusionTerm*clamp(0.3 + ambient + diffuse + specular, 0.0, 1.0);
	
	vec4 finalColor = vec4(color.rgb, finalAlpha);

	if (bOIT)
		AppendABuffer(finalColor);

	ColorOut = finalColor;

	// TO DEBUG OBJECT NORMAL: uncomment this
/*	
	normal = normal*0.5+0.5;
	ColorOut = vec4(normal, 1.0);
*/	

	//ColorOut = (gl_FrontFacing ? vec4(1.0,0.0,0.0,1.0) : vec4(0.0,1.0,0.0,1.0));
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

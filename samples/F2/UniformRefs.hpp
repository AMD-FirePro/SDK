#ifndef _UNIFORMREF_H_
#define _UNIFORMREF_H_

#define SetUniform(a,b) GetRenderManager()->GetGpuParams()->SetParam(UniformName::a, b)

namespace UniformName {

// KEEP IN SYNC WITH SemanticName in GLSLBinding.cpp
enum Semantic
{
	bTex,
	bEnvTex,
	bShadowMap,
	shadowStrength,
	numLights,
	bBumpMapping,
	bSSAO,
	bOIT,
	bDoubleSided,
	fAlphaOverride,

	texStrength,
	envStrength,

	shadowMap,
	shadowMapScale,
	envTex,
	baseTex,
	normalMap,

	doubleSided,
	floorPoint,
	textureScale,
	floorAlpha,

	envMapCube,
	detailMapVolume,
	shadowMap2D,
	diffuse2D,

	depthMap,
	cameraRange,
	radiusSSAO,
	occlusionMap,
	noiseMap,

	nbSample,
	sampleOffsets,
	sampleWeight,
	textureSize,

	eyeWorldPosition,

	floorNormal,
	reflectionMap2D,
	reflectionSize,
	animatedMapVolume,

	time_0_X,
	reflectionBumpiness,
	level,

	matModelView,
	matModelViewProj,
	matNormal,
	matShadow,

	FrontMaterial_diffuse,
	FrontMaterial_ambient,
	FrontMaterial_specular,
	FrontMaterial_shininess,

	maxUniform
};

extern const char * SemanticName[maxUniform];

}

#endif

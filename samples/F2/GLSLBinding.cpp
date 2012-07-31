#include "utils.hpp"
#include "glslbinding.hpp"

#define MAX_UNIFORM_NAME_LEN 100

// HAS TO BE IN THE SAME ORDER THAN Semantic in Uniformrefs.h
namespace UniformName {

const char * SemanticName[maxUniform] = 
{
	"bTex",
	"bEnvTex",
	"bShadowMap",
	"shadowStrength",
	"numLights",
	"bBumpMapping",
	"bSSAO",
	"bOIT",
	"bDoubleSided",
	"fAlphaOverride",

	"texStrength",
	"envStrength",

	"shadowMap",
	"shadowMapScale",
	"envTex",
	"baseTex",
	"normalMap",

	"doubleSided",
	"floorPoint",
	"textureScale",
	"floorAlpha",

	"envMapCube",
	"detailMapVolume",
	"shadowMap2D",
	"diffuse2D",

	"depthMap",
	"cameraRange",
	"radiusSSAO",
	"occlusionMap",
	"noiseMap",

	"nbSample",
	"sampleOffsets",
	"sampleWeight",
	"textureSize",

	"eyeWorldPosition",

	"floorNormal",
	"reflectionMap2D",
	"reflectionSize",
	"animatedMapVolume",

	"time_0_X",
	"reflectionBumpiness",
	"level",

	"startOffsetBuffer",
	"linkedListBuffer",

	"matModelView",
	"matModelViewProj",
	"matNormal",
	"matShadow",

	"FrontMaterial_diffuse",
	"FrontMaterial_ambient",
	"FrontMaterial_specular",
	"FrontMaterial_shininess"

	};

}

////////////////////////////////////////////////////////////////////////////////////
GpuParams::GpuParams()
{
	mParams = NULL;
	InitParamNameMap();
	m_bGPUParamsDirty = true;
}

GpuParams::~GpuParams()
{
	delete [] mParams;
}
////////////////////////////////////////////////////////////////////////////////////
// get position in global list
int GpuParams::GetNamedParamIndex(const std::string& name)
{
	// check if name is found
	ParamNameMap::const_iterator i = mParamNameMap.find(name);
	
	if (i == mParamNameMap.end())
	{
		// no valid name found
		return -1;
	}
	else
	{
		// name found: return the entry
		return (i->second);
	}
	
}

////////////////////////////////////////////////////////////////////////////////////
void GpuParams::InitParamNameMap()
{
	for (int i = 0; i < UniformName::maxUniform; i++)
		mParamNameMap[UniformName::SemanticName[i]] = i;

	if (mParams)
		delete [] mParams;
	mParams = new ParamEntry[mParamNameMap.size()];
}

////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const float *val)
{
	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = 1;
	if (memcmp(e->valf, val, sizeof(float) * 4) != 0)
	{
		memcpy(e->valf, val, sizeof(float) * 4);
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const int *val)
{
	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = 1;
	if (memcmp(e->vali, val, sizeof(int) * 4) != 0)
	{
		memcpy(e->vali, val, sizeof(int) * 4);
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const float *val, int type, int nb)
{
	if (type * nb > MAX_UNIFORM_SIZE)
		return;	// TODO: trace or assert here, then increase MAX_UNIFORM_SIZE depending on usage

	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = nb;
	if (memcmp(e->valf, val, sizeof(float) * type * nb) != 0)
	{
		memcpy(e->valf, val, sizeof(float) * type * nb);
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const int *val, int type, int nb)
{
	if (type * nb > MAX_UNIFORM_SIZE)
		return;	// TODO: trace or assert here, then increase MAX_UNIFORM_SIZE depending on usage

	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = nb;
	if (memcmp(e->vali, val, sizeof(int) * type * nb) != 0)
	{
		memcpy(e->vali, val, sizeof(int) * type * nb);
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const float val)
{
	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = 1;
	if (e->valf[0] != val)
	{
		e->valf[0] = val;
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
void GpuParams::SetParam(UniformName::Semantic uniform, const int val)
{
	ParamEntry* e = &(mParams[uniform]);
	e->isSet = true;
	e->nbElt = 1;
	if (e->vali[0] != val)
	{
		e->vali[0] = val;
		m_bGPUParamsDirty = true;
	}
}
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
GLSLBindings::GLSLBindings() :
	m_bUniformRefsBuilt(false)
{
}

GLSLBindings::~GLSLBindings()
{
}

////////////////////////////////////////////////////////////////////////////////////
void GLSLBindings::Finalize(GLhandleARB hProgram, GpuParams * params)
{
	if (m_bUniformRefsBuilt)
		return;

	BuildUniformRefs(hProgram, params);
	m_bUniformRefsBuilt = true;
}

////////////////////////////////////////////////////////////////////////////////////
void GLSLBindings::BuildUniformRefs(GLhandleARB hProgram, GpuParams * params)
{
	// scan through the active uniforms and add them to the reference list
	GLint    uniformCount;
	GLint  size;
	//GLenum type;
	char   uniformName[MAX_UNIFORM_NAME_LEN];
	//GLint location;
	UniformRef newUniformReference;
	
	// get the number of active uniforms
	glGetObjectParameterivARB(hProgram, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount);
	
	// Loop over each of the active uniforms, and add them to the reference container
	// only do this for user defined uniforms, ignore built in gl state uniforms
	for (int index = 0; index < uniformCount; index++)
	{
		glGetActiveUniform(hProgram, index, MAX_UNIFORM_NAME_LEN, NULL, &size, &newUniformReference.mType, uniformName);

		// ATI fix, remove array indices from the uniform name if it is an array
		std::size_t len = strlen(uniformName);
		if (uniformName[len-1] == ']')	// cheap array detection
		{
			int i = int(len)-2;
			while (i > 0)
			{
				if (uniformName[i] == '[')
				{
					uniformName[i] = '\0';
					break;
				}
				i--;
			}
		}

		// don't add built in uniforms 
		newUniformReference.mLocation = glGetUniformLocation(hProgram, uniformName);
		if (newUniformReference.mLocation >= 0)
		{
			// user defined uniform found, add it to the reference list

			// set accelerator index
			int indexParam = -1;
			indexParam = params->GetNamedParamIndex(std::string(uniformName));
			if (indexParam < 0)
				continue;

			newUniformReference.mIndex = indexParam;

			// decode uniform size and type
			switch (newUniformReference.mType)
			{
			case GL_FLOAT:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 1;
				break;
				
			case GL_FLOAT_VEC2_ARB:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 2;
				break;
				
			case GL_FLOAT_VEC3_ARB:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 3;
				break;
				
			case GL_FLOAT_VEC4_ARB:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 4;
				break;
				
			case GL_FLOAT_MAT3:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 9;
				break;

			case GL_FLOAT_MAT4:
				newUniformReference.isReal = true;
				newUniformReference.mElementCount = 16;
				break;

			case GL_INT:
			case GL_BOOL_ARB:
			case GL_SAMPLER_1D_ARB:
			case GL_SAMPLER_2D_ARB:
			case GL_SAMPLER_3D_ARB:
			case GL_SAMPLER_CUBE_ARB:
			case GL_SAMPLER_1D_SHADOW_ARB:
			case GL_SAMPLER_2D_SHADOW_ARB:	
			case GL_SAMPLER_2D_RECT_ARB:	
			case GL_SAMPLER_2D_RECT_SHADOW_ARB:
			case GL_UNSIGNED_INT_IMAGE_2D:
			case GL_UNSIGNED_INT_IMAGE_BUFFER:
			//default:
				newUniformReference.isReal = false;
				newUniformReference.mElementCount = 1;
				break;
				
			case GL_INT_VEC2_ARB:
			case GL_BOOL_VEC2_ARB:
				newUniformReference.isReal = false;
				newUniformReference.mElementCount = 2;
				break;
				
			case GL_INT_VEC3_ARB:
			case GL_BOOL_VEC3_ARB:
				newUniformReference.isReal = false;
				newUniformReference.mElementCount = 3;
				break;
				
			case GL_INT_VEC4_ARB:
			case GL_BOOL_VEC4_ARB:
				newUniformReference.isReal = false;
				newUniformReference.mElementCount = 4;
				break;
			}// end switch
			
			// valid entry, add it
			mUniformRefs.push_back(newUniformReference);
			
		} // end if
	} // end for
}


////////////////////////////////////////////////////////////////////////////////////
void GLSLBindings::UpdateUniforms(const GpuParams & params)
{
	// iterate through uniform reference list and update uniform values
	UniformRefIt currentUniform = mUniformRefs.begin();
	UniformRefIt endUniform = mUniformRefs.end();
	
	GpuParams::ParamEntry* currentParam;
	
	while (currentUniform != endUniform)
	{
		// get the index in the parameter real list
		currentParam = &(params.mParams[currentUniform->mIndex]);
		if (!currentParam->isSet)
		{
			++currentUniform;
			continue;
		}
		
		if (currentUniform->isReal)
		{
			switch (currentUniform->mElementCount)
			{
				case 1:
					glUniform1fv( currentUniform->mLocation, currentParam->nbElt, currentParam->valf );
					break;
						
				case 2:
					glUniform2fv( currentUniform->mLocation, currentParam->nbElt, currentParam->valf );
					break;
						
				case 3:
					glUniform3fv( currentUniform->mLocation, currentParam->nbElt, currentParam->valf );
					break;
						
				case 4:
					glUniform4fv( currentUniform->mLocation, currentParam->nbElt, currentParam->valf );
					break;

				case 9:
					glUniformMatrix3fv( currentUniform->mLocation, 1, GL_FALSE, currentParam->valf );
					break;

				case 16:
					glUniformMatrix4fv( currentUniform->mLocation, 1, GL_FALSE, currentParam->valf );
					break;

						
				} // end switch
		}
		else
		{
			switch (currentUniform->mElementCount)
			{
				case 1:
					glUniform1iv( currentUniform->mLocation, currentParam->nbElt, currentParam->vali );
					break;
						
				case 2:
					glUniform2iv( currentUniform->mLocation, currentParam->nbElt, currentParam->vali );
					break;
						
				case 3:
					glUniform3iv( currentUniform->mLocation, currentParam->nbElt, currentParam->vali );
					break;
					
				case 4:
					glUniform4iv( currentUniform->mLocation, currentParam->nbElt, currentParam->vali );
					break;
				} // end switch
		}
		
		// get the next uniform
		++currentUniform;
		
	} // end while
}
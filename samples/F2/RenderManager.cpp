#include "GLApp.hpp"
#include "RenderManager.hpp"
#include "RenderToTarget.hpp"
#include "GLSLBinding.hpp"
#include "GLSLShader.hpp"

//////////////////////////////////////////////////////////////////////////////////////
RenderManager::RenderManager(GLApp * pApp)
:	m_pApp(pApp),
	m_pCurrentRenderToTarget(NULL),
	m_pActiveShader(NULL),
	m_time(0),
	m_bInitialized(false),
	m_QuadVA(0),
	m_QuadVBV(0),
	m_QuadVBT(0)
{
}

//////////////////////////////////////////////////////////////////////////////////////
RenderManager::~RenderManager(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::Update(unsigned long milliseconds)
{
	m_time += milliseconds;
	float time0X = (float)m_time*0.001f;
	m_GpuParams->SetParam(UniformName::time_0_X, time0X);
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::OnResize(int width, int height)
{
	if (!m_bInitialized)
		return;

//	ResizeAppendBuffers(width, height);
	DeleteAppendBuffers();
	CreateAppendBuffers(width,  height);
}

//////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::VerifyRenderTarget(renderToTargetNature_e eRTTNature)
{
	bool result = ActivateRenderTarget(eRTTNature, 0, true);
	glFinish();// make sure FBO has been actually completed before returning
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::Initialize()
{
	InitRenderTargetInfos();
	m_GpuParams = new GpuParams();

	CreateAppendBuffers(GetApp()->GetWindow()->GetWidth(), GetApp()->GetWindow()->GetHeight());

	glGenVertexArrays(1, &m_QuadVA);
	glBindVertexArray(m_QuadVA);

	glGenBuffers(1, &m_QuadVBV);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBV);
	float* vert = new float[9];	// vertex array
	vert[0] = -1.0f; vert[1] = -1.0f;
	vert[2] =  1.0;  vert[3] = -1.0f;
	vert[4] = -1.0f; vert[5] =  1.0f;
	vert[6] =  1.0f; vert[7] =  1.0f;
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), vert, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0); 

	// resuse vert for texcoord
	glGenBuffers(1, &m_QuadVBT);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBT);
	vert[0] = 0.0f; vert[1] = 0.0f;
	vert[2] =  1.0;  vert[3] = 0.0f;
	vert[4] = 0.0f; vert[5] =  1.0f;
	vert[6] =  1.0f; vert[7] =  1.0f;
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat), vert, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0); 

	glBindVertexArray(0);
	delete vert;

	m_bInitialized = true;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::Deinitialize()
{
	m_bInitialized = false;

	ShaderMap::const_iterator iter;
	for ( iter = m_shaderMap.begin( ); iter != m_shaderMap.end( ); iter++ )
	{
		delete (iter->second);
	}
	m_shaderMap.clear();

	CleanRenderTargetInfos(true);
	DeleteAppendBuffers();

	if (m_QuadVBV)
		glDeleteBuffers(1, &m_QuadVBV);
	if (m_QuadVBT)
		glDeleteBuffers(1, &m_QuadVBT);
	if (m_QuadVA)
		glDeleteVertexArrays(1, &m_QuadVA);
	delete m_GpuParams;
}

///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::InitRenderTargetInfos()
{
	for (int i = 0; i < eRTT_MAX; i++)
		InitRenderTargetInfo((renderToTargetNature_e)i, true);
}
///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::InitRenderTargetInfo(renderToTargetNature_e eRTTNature, bool bFirstTime)
{
	structRenderTargetInfo * pInfo = &m_RTTInfo[eRTTNature];

	pInfo->pRTT = NULL;
	pInfo->nbTextures = 1;	// default
	pInfo->bMipMap = false;
	pInfo->iMultiSample = 0;
	pInfo->fSuperSample = 0.0f;
	pInfo->iWidthSS = 0;
	pInfo->iHeightSS = 0;
	pInfo->bStencil = false;
	pInfo->bPhysicalScreen = false;
	pInfo->m_bBindDepth = false;

	switch (eRTTNature)
	{
	case eRTT_VIEWS:
		pInfo->iWidth = 1024;
		pInfo->iHeight = 1024;
		pInfo->eFormat = GL_RGBA;
		pInfo->eType = GL_UNSIGNED_BYTE;
		pInfo->iFiltering = GL_LINEAR;
		pInfo->iMultiSample = 4;
		pInfo->bStencil = true;
		pInfo->nbTextures = 6;
		pInfo->bPhysicalScreen = false;
		pInfo->fSuperSample = 1.0f;	// appsize
		// WARNING: dont enable mipmap if scale too small (non power of 2)
		pInfo->bMipMap = false;
		break;


	case eRTT_SHADOW_HIGHRES:
		pInfo->iWidth = 1024;
		pInfo->iHeight = 1024;
		pInfo->eFormat = GL_DEPTH_COMPONENT;
		pInfo->eType = GL_FLOAT;
		pInfo->iFiltering = GL_LINEAR;
		break;


	case eRTT_SSAO_NORMAL:
		pInfo->iWidth = 1024;
		pInfo->iHeight = 1024;
		pInfo->eFormat = GL_RGBA;
		pInfo->eType = GL_UNSIGNED_BYTE;
		pInfo->iFiltering = GL_LINEAR;
		pInfo->bPhysicalScreen = false;
		pInfo->fSuperSample = 1.0f;//0.5f;
		pInfo->m_bBindDepth = true;
//		pInfo->bStencil = true;

		// WARNING: dont enable mipmap if scale too small (non power of 2)
		pInfo->bMipMap = false;
		break;

	case eRTT_SSAO_RESULT:
		pInfo->iWidth = 1024;
		pInfo->iHeight = 1024;
		pInfo->eFormat = GL_RGBA;
		pInfo->eType = GL_UNSIGNED_BYTE;
		pInfo->iFiltering = GL_LINEAR;
		pInfo->bPhysicalScreen = false;
		pInfo->fSuperSample = 1.0f;//0.5f;
		// WARNING: dont enable mipmap if scale too small (non power of 2)
		pInfo->bMipMap = false;
		pInfo->nbTextures = 3;
		break;

	default:
		break;
	};

	if (bFirstTime)
	{
		pInfo->tex = new GLuint[pInfo->nbTextures];
		pInfo->texDepth = new GLuint[pInfo->nbTextures];
		pInfo->used = new bool[pInfo->nbTextures];

		for (int iTex = 0; iTex < pInfo->nbTextures; iTex++)
		{
			pInfo->tex[iTex] = 0;
			pInfo->texDepth[iTex] = 0;
			pInfo->used[iTex] = false;
		}
	}

	if (pInfo->bPhysicalScreen && pInfo->fSuperSample == 0.0f)
		pInfo->fSuperSample = 1.0f;

	if (pInfo->bPhysicalScreen)
	{
		pInfo->iWidthSS = GetSystemMetrics(SM_CXSCREEN);
		pInfo->iHeightSS = GetSystemMetrics(SM_CYSCREEN);
	}
	else if (pInfo->fSuperSample > 0)
	{
		pInfo->iWidthSS = GetApp()->GetWindow()->GetWidth();
		pInfo->iHeightSS = GetApp()->GetWindow()->GetHeight();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::CleanRenderTargetInfos(bool bExit)
{
	for (int i = 0; i < eRTT_MAX; i++)
		CleanRenderTargetInfo((renderToTargetNature_e)i, bExit);
}
///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::CleanRenderTargetInfo(renderToTargetNature_e eRTTNature, bool bExit)
{
	structRenderTargetInfo * pInfo = &m_RTTInfo[eRTTNature];

	// this doesnt need to be called on exit anymore since each object releases its associated texture
	if (!bExit)
	{
		for (int iTex = 0; iTex < pInfo->nbTextures; iTex++)
		{
			if (pInfo->tex[iTex])
			{
				glDeleteTextures(1, &pInfo->tex[iTex]);
				pInfo->tex[iTex] = 0;
				// DO NOT RESET THAT ONE !!!
				//pInfo->used[iTex] = false;
			}
			if (pInfo->texDepth[iTex])
			{
				glDeleteTextures(1, &pInfo->texDepth[iTex]);
				pInfo->texDepth[iTex] = 0;
			}

		}
	}
	else
	{
		delete [] pInfo->tex;
		delete [] pInfo->texDepth;
		delete [] pInfo->used;
	}

	if (pInfo->pRTT)
	{
		delete pInfo->pRTT;
		pInfo->pRTT = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
RenderToTarget * RenderManager::GetRenderTarget(renderToTargetNature_e eRTTNature)
{
	RenderToTarget * pRTT = m_RTTInfo[eRTTNature].pRTT;
	if (pRTT == NULL)	// if that type doesnt exist, attempt to create it
	{
		pRTT = new RenderToFrameBuffer();

		if (m_RTTInfo[eRTTNature].bPhysicalScreen)
		{
			m_RTTInfo[eRTTNature].iWidthSS = GetSystemMetrics(SM_CXSCREEN);
			m_RTTInfo[eRTTNature].iHeightSS = GetSystemMetrics(SM_CYSCREEN);
		}
		else if (m_RTTInfo[eRTTNature].fSuperSample > 0)
		{
			m_RTTInfo[eRTTNature].iWidthSS = GetApp()->GetWindow()->GetWidth();
			m_RTTInfo[eRTTNature].iHeightSS = GetApp()->GetWindow()->GetHeight();
		}

		bool success = false;
		bool tryagain = true;
		while (tryagain)
		{
			success = pRTT->Init(m_RTTInfo[eRTTNature]);

			// give AA a chance: try mode 3 and mode 1 before giving up
			if (!success && (m_RTTInfo[eRTTNature].iMultiSample > 0))
			{
				if (m_RTTInfo[eRTTNature].iMultiSample > 3)
					m_RTTInfo[eRTTNature].iMultiSample = 3;
				else if (m_RTTInfo[eRTTNature].iMultiSample >= 3)
					m_RTTInfo[eRTTNature].iMultiSample = 1;
				else
					m_RTTInfo[eRTTNature].iMultiSample = 0;
				tryagain = true;
				// flush OpenGL error and ignore error
				CheckOglError(false);
			}
			else
				tryagain = false;
		}

		if (success)
		{
			m_RTTInfo[eRTTNature].pRTT = pRTT;
			pRTT->SetNature(eRTTNature);
		}
		else
		{
			delete pRTT;
			pRTT = NULL;
		}
	}

	return pRTT;
}


///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::ActivateRenderTarget(renderToTargetNature_e eRTTNature, int iTex, bool bValidationOnly)
{
	if (!(RenderToFrameBuffer::IsAvailable()/*[0]*/))
		return false;

//	assert(m_pCurrentRenderToTarget == NULL);	// should have released previous one first

	bool success = false;
	RenderToTarget * pRTT = GetRenderTarget(eRTTNature);
	if (pRTT)
	{
		if (bValidationOnly)
			return true;

//		assert (iTex < m_RTTInfo[pRTT->GetNature()].nbTextures);
		GLuint * tex = &(m_RTTInfo[pRTT->GetNature()].tex[iTex]);
		GLuint * texDepth = &(m_RTTInfo[pRTT->GetNature()].texDepth[iTex]);

		success = pRTT->RenderBegin(tex, texDepth);
		// only set current if it succeeds to avoid assert on next activate
		if (success)
			m_pCurrentRenderToTarget = pRTT;
	}
	return success;
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::ReleaseActiveRenderTarget()
{
//	assert(m_pCurrentRenderToTarget);	// should have something to release
	if(!m_pCurrentRenderToTarget) return false;

	bool success = m_pCurrentRenderToTarget->RenderEnd();
	m_pCurrentRenderToTarget = NULL;
	// Restore old view port and set rendering back to default frame buffer
	glViewport(0, 0, GetApp()->GetWindow()->GetWidth(), GetApp()->GetWindow()->GetHeight());

	return success;
}


///////////////////////////////////////////////////////////////////////////////////////
GLuint RenderManager::GetTexture(renderToTargetNature_e eRTTNature, int iTex, bool bDepth)
{
	// should not call this if no render to target called for that nature earlier...
	if (m_RTTInfo[eRTTNature].pRTT)
	{
		if (bDepth && m_RTTInfo[eRTTNature].m_bBindDepth)
			return m_RTTInfo[eRTTNature].texDepth[iTex];
		else
			return m_RTTInfo[eRTTNature].tex[iTex];
	}
	else
		return 0;	// RenderToTarget failed or not called yet
}

///////////////////////////////////////////////////////////////////////////////////////
glm::vec2 RenderManager::GetRenderingTargetSize(renderToTargetNature_e eRTTNature)
{
	const structRenderTargetInfo& rttInfo = m_RTTInfo[eRTTNature];
	int size[2];
	if (rttInfo.fSuperSample > 0 && rttInfo.iWidthSS > 0 && rttInfo.iHeightSS > 0)
	{
		size[0] = (int)((float)rttInfo.iWidthSS * rttInfo.fSuperSample + 0.5f);
		size[1] = (int)((float)rttInfo.iHeightSS * rttInfo.fSuperSample + 0.5f);
	}
	else
	{
		size[0] = rttInfo.iWidth;
		size[1] = rttInfo.iHeight;
	}

	return glm::vec2((float)size[0], (float)size[1]);
}

///////////////////////////////////////////////////////////////////////////////////////
void RenderManager::GetCurrentRenderingTargetSize(int& width, int& height)
{
	if (m_pCurrentRenderToTarget)
	{
		glm::vec2 size = GetRenderingTargetSize(m_pCurrentRenderToTarget->GetNature());
		width	= (int)size[0];
		height	= (int)size[1];
	}
	else
	{
		width = 0;
		height = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
renderToTargetNature_e const RenderManager::GetRenderToTargetNature()
{
	if (m_pCurrentRenderToTarget)
		return m_pCurrentRenderToTarget->GetNature();
	else
		return eRTT_MAX;
}

//////////////////////////////////////////////////////////////////////////////////////
GLSLShader * RenderManager::CreateShader(unsigned int shaderProfile)
{
//	if (!IsSupported(caps_Shaders))
//		return NULL;

	DisableShader();

	GLSLShader * pShader = new GLSLShader();
	if (!pShader->Init(shaderProfile, m_GpuParams))
	{
		delete pShader;
		pShader = NULL;
	}

	return pShader;
}

//////////////////////////////////////////////////////////////////////////////////////
GLSLShader * RenderManager::GetShaderFromProfile(unsigned int shaderProfile)
{
	ShaderMap::const_iterator i = m_shaderMap.find(shaderProfile);

	if (i == m_shaderMap.end())
	{
		// not found, create shader on the fly
		GLSLShader * pShader = CreateShader(shaderProfile);
		m_shaderMap[shaderProfile] = pShader;	// add even if NULL, so we dont try to re-create every time we query if it failed
		return pShader;
	}
	else
	{
		// found: return the entry (can be NULL, means bad shader, so we dont try to recreate everytime)
		return (i->second);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::SetupShaderProfile(unsigned int shaderProfile)
{
	switch (shaderProfile)
	{
	case UberShader:
		if (GetApp()->m_bDiffuse)
		{
			glActiveTexture(GL_TEXTURE0_ARB);
			m_GpuParams->SetParam(UniformName::baseTex, 0);	// tex unit 0
			glBindTexture(GL_TEXTURE_2D, GetApp()->m_texDiffuse);
		}

		if (GetApp()->m_bNormal)
		{
			glActiveTexture(GL_TEXTURE1_ARB);
			m_GpuParams->SetParam(UniformName::normalMap, 1);	// tex unit 1
			glBindTexture(GL_TEXTURE_2D, GetApp()->m_texBump);
		}

		if (GetApp()->m_bShadow)
		{
			glActiveTexture(GL_TEXTURE2_ARB);
			m_GpuParams->SetParam(UniformName::shadowMap, 2);	// tex unit 2
			glm::vec2 size = GetRenderingTargetSize(eRTT_SHADOW_HIGHRES);
			m_GpuParams->SetParam(UniformName::shadowMapScale, 1.0f/size[0]);
			glBindTexture(GL_TEXTURE_2D, GetTexture(eRTT_SHADOW_HIGHRES));

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);

			m_GpuParams->SetParam(UniformName::shadowMapScale, 1.0f/size[0]);
			m_GpuParams->SetParam(UniformName::matShadow, &m_mTextureProjectionShadow[0][0], 1, 16);
		}

		if (GetApp()->m_bEnvmap)
		{
			glActiveTexture(GL_TEXTURE3_ARB);
			m_GpuParams->SetParam(UniformName::envMapCube,3);	// tex unit 3
			glBindTexture(GL_TEXTURE_CUBE_MAP, GetApp()->m_texEnvCube);
		}

		if (GetApp()->m_bSSAO)
		{
			glActiveTexture(GL_TEXTURE4_ARB);
			m_GpuParams->SetParam(UniformName::occlusionMap, 4);	// tex unit 4
			glBindTexture(GL_TEXTURE_2D, GetTexture(eRTT_SSAO_RESULT));
		}
		break;

	case ComputeSSAO:
		m_GpuParams->SetParam(UniformName::depthMap, 0);	// tex unit 0
		glBindTexture(GL_TEXTURE_2D, GetTexture(eRTT_SSAO_NORMAL, 0, true));

		glActiveTexture(GL_TEXTURE1_ARB);
		m_GpuParams->SetParam(UniformName::normalMap, 1);	// tex unit 1
		glBindTexture(GL_TEXTURE_2D, GetTexture(eRTT_SSAO_NORMAL));

		glActiveTexture(GL_TEXTURE2_ARB);
		m_GpuParams->SetParam(UniformName::noiseMap, 2);	// tex unit 2
		glBindTexture(GL_TEXTURE_2D, GetApp()->m_texNoise);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		break;
	}

	glActiveTexture(GL_TEXTURE0_ARB);

	CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
// set only thing that are GPU program unique (dont change if the same shader is used)
void RenderManager::CleanShaderProfile(unsigned int shaderProfile)
{
	glActiveTexture(GL_TEXTURE0_ARB); 
}

//////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::EnableShader(unsigned int shaderProfile)
{
	GLSLShader * pShader = GetShaderFromProfile(shaderProfile);
	if (pShader != m_pActiveShader)
		DisableShader();

	if (pShader == NULL)
		return false;

	if (pShader == m_pActiveShader)	// same shader, only update uniforms
	{
		UpdateCurrentShaderUniforms(false);
		return true;
	}

	if (pShader->Activate())
	{
		m_pActiveShader = pShader;
		SetupShaderProfile(shaderProfile);
		UpdateCurrentShaderUniforms(true);//false);//true);	// interesting optimization: uniform are persistent? only send changes.
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::UpdateCurrentShaderUniforms(bool forceUpdate)
{
	if (!m_pActiveShader)
		return false;

	if (m_GpuParams->IsDirty() || forceUpdate)
	{
		m_pActiveShader->GetBindings().UpdateUniforms(*m_GpuParams);
		m_GpuParams->SetDirty(false);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::DisableShader()
{
	if (m_pActiveShader)
	{
		m_pActiveShader->Deactivate();
		CleanShaderProfile(m_pActiveShader->GetProfile());
		m_pActiveShader = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::BlurTarget(renderToTargetNature_e eRTTNature, int targetId, int targetTemp, int blurLevel, float focus, float clearCol, float clearAlpha )
{
	// two pass gaussian blur: original map to tmp map on horizontal then back to original on vertical
	if (blurLevel < 1)
		blurLevel = 1;
	if (blurLevel > 16)
		blurLevel = 16;

	glm::vec2 rttSize = GetRenderingTargetSize(eRTTNature);

	float scale[4] = {0.0, 0.0, focus / rttSize[0], focus / rttSize[1]};

	std::vector<glm::vec2> offsets;
	std::vector<float> weights;
	offsets.resize(blurLevel);
	weights.resize(blurLevel);

	float weightsSum = 0.0f;
	for (int i = 0; i < blurLevel; ++i)
	{
		float offset = (float) i + 0.5f - ((float) blurLevel / 2.0f);
		offsets[i] = glm::vec2(offset, 0.0);
		weights[i] = 1.0f / (fabs(offset) + 1.0f);
		weightsSum += weights[i];
	}

	// normalize total weight to 1.0
	for (int i = 0; i < blurLevel; ++i)
		weights[i] /= weightsSum;

	// horizontal pass
	bool success = ActivateRenderTarget(eRTTNature, targetTemp);	// store result in 2nd texture

	glClearColor(clearCol, clearCol, clearCol, clearAlpha);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, GetTexture(eRTTNature, targetId));	// bind original texture
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	m_GpuParams->SetParam(UniformName::nbSample, blurLevel);
	m_GpuParams->SetParam(UniformName::diffuse2D, 0);	// tex unit 0
	m_GpuParams->SetParam(UniformName::textureSize, scale);
	m_GpuParams->SetParam(UniformName::sampleOffsets, &(offsets[0][0]), 2, int(offsets.size()));
	m_GpuParams->SetParam(UniformName::sampleWeight, &(weights[0]), 1, int(weights.size()));

	EnableShader(BlurPass);

	DrawScreenQuadDirectWithTex();

	ReleaseActiveRenderTarget();

	// vertical pass, move offsets from x to y
	for (int i = 0; i < blurLevel; ++i)
	{
		offsets[i][1] = offsets[i][0];
		offsets[i][0] = 0.0f;
	}

	// destination: reuse original map
	success = ActivateRenderTarget(eRTTNature, targetId);	// result back in 1st texture

	glClearColor(clearCol, clearCol, clearCol, clearAlpha);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	glBindTexture(GL_TEXTURE_2D, GetTexture(eRTTNature,targetTemp));	// bind previous texture
	m_GpuParams->SetParam(UniformName::textureSize, scale);
	m_GpuParams->SetParam(UniformName::sampleOffsets, &(offsets[0][0]), 2, int(offsets.size()));
	m_GpuParams->SetParam(UniformName::sampleWeight, &(weights[0]), 1, int(weights.size()));
	UpdateCurrentShaderUniforms();

	DrawScreenQuadDirectWithTex();

	DisableShader();

	ReleaseActiveRenderTarget();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
const glm::mat4 mBias(0.5, 0.0, 0.0, 0.0, 
					   0.0, 0.5, 0.0, 0.0,
					   0.0, 0.0, 0.5, 0.0,
					   0.5, 0.5, 0.5, 1.0);


///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::RenderShadowMap()
{
	bool success =  ActivateRenderTarget(eRTT_SHADOW_HIGHRES);
	if (!success)
		return false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(/*GL_COLOR_BUFFER_BIT|*/GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	float sceneSize = GetApp()->GetModelSize()*0.75f;
	glm::mat4 mShadowProj = glm::ortho(-sceneSize, sceneSize, -sceneSize, sceneSize, -sceneSize, sceneSize);

	// set view matrix from light point of view
	glm::vec4 eye = GetApp()->mModelViewInv * GetApp()->GetLightDirection0();
	glm::mat4 mShadowModelView = glm::lookAt(glm::vec3(eye), glm::vec3(0.0f), glm::vec3(0,1,0));

	// compute texture projection matrix
	glm::mat4 mShadowModelViewProj = mShadowProj * mShadowModelView;
	m_mTextureProjectionShadowBase = mBias * mShadowModelViewProj;
	GetGpuParams()->SetParam(UniformName::matModelViewProj, &mShadowModelViewProj[0][0], 1, 16);

// actual render
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glColorMask(FALSE, FALSE, FALSE, FALSE);
	EnableShader(ShadowPass);

	glDisable(GL_CULL_FACE);
	GetApp()->RenderModel();

	glColorMask(TRUE, TRUE, TRUE, TRUE);
	DisableShader();
	glDisable(GL_POLYGON_OFFSET_FILL);

    ReleaseActiveRenderTarget();

	// restore matviewproj
	GetGpuParams()->SetParam(UniformName::matModelViewProj, &GetApp()->mModelViewProj[0][0], 1, 16);

	return CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::RenderSSAONormal()
{
	bool success =  ActivateRenderTarget(eRTT_SSAO_NORMAL);
	if (!success)
		return false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	bool bShader = EnableShader(OutputNormal);

	GetApp()->RenderModel();

	if (bShader)
		DisableShader();

    ReleaseActiveRenderTarget();

	return bShader && CheckOglError();
}
//////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::RenderSSAOResult()
{
	bool success =  ActivateRenderTarget(eRTT_SSAO_RESULT);
	if (!success)
		return false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	float camRng[4];
	camRng[0] = GetApp()->GetWindow()->GetNearPlane();
	camRng[1] = GetApp()->GetWindow()->GetFarPlane();
	camRng[2] = camRng[0]*camRng[1];
	camRng[3] = camRng[1] - camRng[0];
	GetGpuParams()->SetParam(UniformName::cameraRange, camRng);
	GetGpuParams()->SetParam(UniformName::radiusSSAO, 0.06f);

	bool bShader = EnableShader(ComputeSSAO);

	DrawScreenQuadDirectWithTex();

	if (bShader)
		DisableShader();

    ReleaseActiveRenderTarget();

	// now blur result (id 0, using id 1, result back in id 0)
	BlurTarget(eRTT_SSAO_RESULT, 0, 1, 8 );

	return CheckOglError() && bShader;
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderManager::RenderGlowMap()
{
	bool success =  ActivateRenderTarget(eRTT_VIEWS, 2);
	if (!success)
		return false;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glLineWidth(1.0f);
	glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// <- not core 4.2... will need GS to render edges ?

	bool bShader = EnableShader(GlowPass);
	GetApp()->RenderModel();
	if (bShader)
		DisableShader();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);


    ReleaseActiveRenderTarget();

	// now blur result (id 2, using id 3, result back in id 2)
	BlurTarget(eRTT_VIEWS, 2, 3, 4, 1.0f );

	return CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//ABuffer for OIT
void RenderManager::CreateAppendBuffers(int iWidth, int iHeight)
{
	glGenTextures(2, &m_globalbuffer[0]);

    glBindTexture(GL_TEXTURE_2D, m_globalbuffer[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI,  iWidth, iHeight, 0, GL_RED_INTEGER_EXT, GL_UNSIGNED_INT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	m_linkedListBuffer = 0;
	int listBufSize = iWidth * iHeight * MAX_SORTED_FRAGMENT * sizeof(LinkedListBuffer);
    glGenBuffers(1, &m_linkedListBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_linkedListBuffer);
    glBufferData(GL_ARRAY_BUFFER, listBufSize, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0 );

    glBindTexture(GL_TEXTURE_BUFFER_ARB, m_globalbuffer[1]);    
    glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_RGBA32UI, m_linkedListBuffer);      

	glGenBuffers(1,&m_atomicCounter);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER_EXT, m_atomicCounter);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER_EXT, 32, NULL, GL_DYNAMIC_DRAW);

	//glBindBufferBaseEXT2(GL_ATOMIC_COUNTER_BUFFER_EXT, 0, m_atomicCounter);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glBindImageTextureEXT(0,  m_globalbuffer[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTextureEXT(1,  m_globalbuffer[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32UI);

	CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
//fast resize
void RenderManager::ResizeAppendBuffers(int iWidth, int iHeight)
{
    glBindTexture(GL_TEXTURE_2D, m_globalbuffer[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI,  iWidth, iHeight, 0, GL_RED_INTEGER_EXT, GL_INT, NULL);

	int listBufSize = iWidth * iHeight * MAX_SORTED_FRAGMENT * sizeof(LinkedListBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_linkedListBuffer);
    glBufferData(GL_ARRAY_BUFFER, listBufSize, NULL, GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0 );

    glBindTexture(GL_TEXTURE_BUFFER_ARB, m_globalbuffer[1]);    
    glTexBuffer(GL_TEXTURE_BUFFER_ARB, GL_RGBA32UI, m_linkedListBuffer);      

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::DeleteAppendBuffers()
{
	if (m_linkedListBuffer > 0)
	{
		glDeleteBuffers(1, &m_linkedListBuffer);
		m_linkedListBuffer = 0;
	}
	if (m_atomicCounter > 0)
	{
		glDeleteBuffers(1, &m_atomicCounter);
		m_atomicCounter = 0;
	}
	if (m_globalbuffer > 0)
	{
		glDeleteTextures(2, m_globalbuffer);
		m_globalbuffer[0] = 0;
		m_globalbuffer[1] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::ClearAppendBuffers()
{
	static GLuint ZeroMem = 0;

	GetGpuParams()->SetParam(UniformName::startOffsetBuffer, 0);
	GetGpuParams()->SetParam(UniformName::linkedListBuffer, 1);

	bool bShader = EnableShader(AppendClear);

	glBindTexture(GL_TEXTURE_2D, GetApp()->m_texBackground);

	DrawScreenQuadDirect();

	glMemoryBarrierEXT(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    //glBindBufferBaseEXT2(GL_ATOMIC_COUNTER_BUFFER_EXT, 0, m_atomicCounter);
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER_EXT, 0, sizeof(GLuint), &ZeroMem);

	CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::ResolveAppendBuffers()
{
	glMemoryBarrierEXT(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	bool bShader = EnableShader(AppendResolve);

	DrawScreenQuadDirect();

	DisableShader();
	glFinish();
	CheckOglError();
}

//////////////////////////////////////////////////////////////////////
//	draw a full screen quad with texture coordinate in [0,1]x[0,1]
void RenderManager::DrawScreenQuad(float x, float y, float w, float h, bool bVFlip)
{
	float scale[4] = {x,y,w,h};
	m_GpuParams->SetParam(UniformName::diffuse2D, 0);	// tex unit 0
	m_GpuParams->SetParam(UniformName::bTex, bVFlip ? 1 : 0);
	m_GpuParams->SetParam(UniformName::textureSize, scale);
	EnableShader(QuadTex);
	DrawScreenQuadDirectWithTex();
	DisableShader();
}

//////////////////////////////////////////////////////////////////////
void RenderManager::DrawScreenQuadDirect()
{
	glBindVertexArray(m_QuadVA);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	CheckOglError();
}

//////////////////////////////////////////////////////////////////////////////////////
void RenderManager::DrawScreenQuadDirectWithTex()
{
	glBindVertexArray(m_QuadVA);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	CheckOglError();
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

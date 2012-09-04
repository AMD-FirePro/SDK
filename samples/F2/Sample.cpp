#include "sample.hpp"
#include "utils.hpp"
#include "rendermanager.hpp"
#include "glslshader.hpp"

#include <assimp.h>
#include "ModelAndTextureLoader.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
GLApp * GLApp::Create(const char * class_name)
{
	GLSample * example = new GLSample(class_name);
	return reinterpret_cast<GLApp *>(example);
}

/////////////////////////////////////////////////////////////////////////////////////////////
GLSample::GLSample(const char * class_name) :
GLApp(class_name),
	m_bBlurTest(false),
	m_doubleSided(true),
	m_numLights(3),
	m_texStrength(1.0f),
	m_envStrength(0.2f),
	m_Angle(0.0f),
	m_bAnimate(true),
	m_blurLevel(0.0f),
	m_texStrengthGoal(0.0f),
	m_shadowStrength(0.8f),
	m_shadowStrengthGoal(0.8f),
	m_envStrengthGoal(0.2f),
	m_fAlpha(0.6f),
	m_bUseSSAO(false),
	m_bDebugRTT(false),
	fullModel(NULL)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
static void CALLBACK debugOutput
	(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	GLvoid* userParam
	)
{
	char debSource[32], debType[32], debSev[32];
	if(source == GL_DEBUG_SOURCE_API_ARB)
		strcpy_s(debSource, 31, "OpenGL");
	else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
		strcpy_s(debSource, 31, "Windows");
	else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
		strcpy_s(debSource, 31, "Shader Compiler");
	else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
		strcpy_s(debSource, 31, "Third Party");
	else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
		strcpy_s(debSource, 31, "Application");
	else if(source == GL_DEBUG_SOURCE_OTHER_ARB)
		strcpy_s(debSource, 31, "Other");

	if(type == GL_DEBUG_TYPE_ERROR_ARB)
	{
		strcpy_s(debType, 31, "error");
	}
	else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
		strcpy_s(debType, 31, "deprecated behavior");
	else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
		strcpy_s(debType, 31, "undefined behavior");
	else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
		strcpy_s(debType, 31, "portability");
	else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
		strcpy_s(debType, 31, "performance");
	else if(type == GL_DEBUG_TYPE_OTHER_ARB)
		strcpy_s(debType, 31, "message");

	if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)
		strcpy_s(debSev, 31, "high");
	else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
		strcpy_s(debSev, 31, "medium");
	else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
		strcpy_s(debSev, 31, "low");

	static char tmp[4096];
	sprintf_s(tmp, 4095, "%s: %s(%s) %d: %s\n", debSource, debType, debSev, id, message);
	OutputDebugString( tmp );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::SetDebug()
{
	if (m_disableGLDEbug)
	{
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
		glDebugMessageCallbackARB(NULL, NULL);
	}
	else
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallbackARB(debugOutput, NULL);
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::Initialize()
{
	SetDebug();		
	GLApp::Initialize();

	SetupOpenGLStates();
	ResetMaterialProperties();
	ResizeDraw(true);

	m_texDiffuse = CreateTexture2D("data/Stones_Diffuse.dds");
	m_texBump = CreateTexture2D("data/Stones_NormalHeight.tga");
	m_texEnvCube = CreateTextureCube("data/EnvMap.dds");
	m_texBackground = CreateTexture2D("data/logo.dds");

	m_texNoise = CreateTexture2D("data/Noise.tga");

	SetUniform(envMapCube, 3);	// has to be set even if not evaluated... or error while rendering geom
	SetUniform(startOffsetBuffer, 0);
	SetUniform(linkedListBuffer, 1);

	fullModel = new ModelAndTextureLoader("data/models/F40/","data/models/F40/F40.obj"); 
	SetModelSize(fullModel->GetSize());
	SetModelCenter(fullModel->GetCenter());

	return CheckOglError();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::Deinitialize()
{
	SAFE_DELETE(fullModel);

	if (m_texDiffuse)
		glDeleteTextures( 1, &m_texDiffuse);
	if (m_texBump)
		glDeleteTextures( 1, &m_texBump);
	if (m_texEnvCube)
		glDeleteTextures( 1, &m_texEnvCube);
	if (m_texBackground)
		glDeleteTextures( 1, &m_texBackground);
	if (m_texNoise)
		glDeleteTextures( 1, &m_texNoise);

	GLApp::Deinitialize();
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::SetupOpenGLStates()
{
	glClearColor(0.9f,0.9f,0.9f,0.0f);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glm::vec4 ambient(0.35f, 0.35f, 0.35f, 1.0f);
	glm::vec4 diffuse0(0.9f, 0.85f, 0.8f, 1.0f);
	glm::vec4 specular0(0.6f, 0.6f, 0.6f, 1.0f);
	glm::vec4 position0(-3.0f, 3.5f, 3.5f, 0.0f);

	glm::vec4 diffuse1(0.25f, 0.3f, 0.35f, 1.0f);
	glm::vec4 specular1(0.4f, 0.4f, 0.4f, 1.0f);
	glm::vec4 position1(4.0f, 5.0f, 2.0f, 0.0f);

	glm::vec4 diffuse2(0.3f, 0.3f, 0.3f, 1.0f);
	glm::vec4 specular2(0.4f, 0.4f, 0.4f, 1.0f);
	glm::vec4 position2(2.0f, -5.0f, 2.0f, 0.0f);

	position0 = glm::normalize(position0);
	position1 = glm::normalize(position1);
	position2 = glm::normalize(position2);

	SetLightDirection0(position0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::ResetMaterialProperties()
{
	static const glm::vec4 no_mat(0.0f, 0.0f, 0.0f, 1.0f);
	static const glm::vec4 mat_ambient(0.1f, 0.1f, 0.1f, 1.0f);
	static const glm::vec4 mat_diffuse(0.2f, 0.6f, 0.9f, m_fAlpha);
	static const glm::vec4 mat_specular(0.9f, 0.9f, 0.9f, 1.0f);
	static const float no_shininess = 0.0f;
	static const float low_shininess = 10.0f;
	static const float high_shininess = 80.0f;
	static const glm::vec4 mat_emission(0.3f, 0.2f, 0.2f, 0.0f);

	SetUniform(FrontMaterial_diffuse, &mat_diffuse[0]);
	SetUniform(FrontMaterial_ambient, &mat_ambient[0]);
	SetUniform(FrontMaterial_specular, &mat_specular[0]);
	SetUniform(FrontMaterial_shininess, high_shininess);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::Update(DWORD milliseconds)
{
	if (m_bAnimate)
	{
		m_Angle += (float)(milliseconds) / 15.0f;
		m_texStrengthGoal = cosf(m_Angle/90.0f)*0.5f + 0.5f;
		m_fAlpha = sinf(m_Angle/180.0f)*0.4f + 0.6f;
	}

	UpdateProperty(m_blurLevel, m_bBlurTest ? 8.0f : 0, 0.02f, milliseconds);
	UpdateProperty(m_texStrength, m_texStrengthGoal, 0.001f, milliseconds);
	UpdateProperty(m_shadowStrength, m_shadowStrengthGoal, 0.001f, milliseconds);
	UpdateProperty(m_envStrength, m_envStrengthGoal, 0.001f, milliseconds);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnKeyDown(unsigned int key)
{
	switch (key)
	{
	case 'f' - 32:
		m_disableGLDEbug = !m_disableGLDEbug;
		break;

	case 'a' -32:
		m_bUseSSAO = !m_bUseSSAO;
		break;

	case 'v' -32:
		m_bDebugRTT = !m_bDebugRTT;
		break;

	case 'd'-32:
		m_doubleSided = !m_doubleSided;
		break;

	case 'b'-32:
		m_bBlurTest = !m_bBlurTest;
		break;

	case 32:	// space bar
		m_bAnimate = !m_bAnimate;
		break;

	case 's'-32:
		if (m_shadowStrengthGoal > 0.0f)
			m_shadowStrengthGoal = 0.0f;
		else
			m_shadowStrengthGoal = 0.7f;
		break;

	case 't'-32:
		if (m_texStrengthGoal > 0.0f)
			m_texStrengthGoal = 0.0f;
		else
			m_texStrengthGoal = 1.0f;
		break;

	case 'n'-32:
		m_bNormal = !m_bNormal;
		break;

	case 'o'-32:
		m_bOIT = !m_bOIT;
		if (m_bOIT && m_fAlpha > 0.99f)
			m_fAlpha = 0.99f;
		break;

	case 'e'-32:
		if (m_envStrengthGoal > 0.0f)
			m_envStrengthGoal = 0.0f;
		else
			m_envStrengthGoal = 0.4f;
		break;

	case 'g'-32:
		m_bGlow = !m_bGlow;
		break;

	case VK_ADD:
		if (m_numLights < 3)
			m_numLights++;
		break;

	case VK_SUBTRACT:
		if (m_numLights > 0)
			m_numLights--;
		break;

	case VK_NUMPAD0:
		if (m_texStrengthGoal > -1.0f)
			m_texStrengthGoal -= 0.1f;
		break;

	case VK_NUMPAD1:
		if (m_texStrengthGoal < 1.0f)
			m_texStrengthGoal += 0.1f;
		break;

	case VK_NUMPAD2:
		if (m_shadowStrengthGoal > 0.0f)
			m_shadowStrengthGoal -= 0.1f;
		break;

	case VK_NUMPAD3:
		if (m_shadowStrengthGoal < 1.0f)
			m_shadowStrengthGoal += 0.1f;
		break;

	case VK_NUMPAD4:
		if (m_envStrengthGoal > 0.0f)
			m_envStrengthGoal -= 0.1f;
		break;

	case VK_NUMPAD5:
		if (m_envStrengthGoal < 1.0f)
			m_envStrengthGoal += 0.1f;
		break;

	case VK_NUMPAD6:
		if (m_fAlpha > 0.4f)
			m_fAlpha -= 0.01f;
		break;

	case VK_NUMPAD7:
		if (m_fAlpha < 1.0f)
			m_fAlpha += 0.01f;
		break;

	}
	return GLApp::OnKeyDown(key);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnKeyUp(unsigned int key)
{
	return GLApp::OnKeyUp(key);
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnLeftButtonDown(int x, int y)
{
	StartCaptureMouse();
	m_bLeftMouseDown = true;
	m_iLeftMouseDownXlast = x;
	m_iLeftMouseDownYlast = y;
	return true; 
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnLeftButtonUp(int x, int y)
{
	StopCaptureMouse();
	m_bLeftMouseDown = false;
	return true; 
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnMouseMove(int x, int y)
{
	if (m_bLeftMouseDown)
	{
		int deltaX = x - m_iLeftMouseDownXlast;
		int deltaY = y - m_iLeftMouseDownYlast;

		m_fAngleX += (float)deltaX*0.1f;
		m_fAngleY -= (float)deltaY*0.1f;
	}

	m_iLeftMouseDownXlast = x;
	m_iLeftMouseDownYlast = y;

	return true; 
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool GLSample::OnMouseWheel(int x, int y, int w)
{
	m_fZoom *= (1.0f + (float)w*0.0004f);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::SetViewTransform()
{
	glViewport(0, 0, GetWindow()->GetWidth(), GetWindow()->GetHeight());

	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f*m_fZoom, (float)GetWindow()->GetWidth()/(float)GetWindow()->GetHeight(), GetWindow()->GetNearPlane(), GetWindow()->GetFarPlane());
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -6.0f));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, m_Angle * 0.7f + m_fAngleY, glm::vec3(-1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, m_Angle + m_fAngleX, glm::vec3(0.f, 1.f, 0.f));

	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0/GetModelSize()));
	glm::mat4 Model = glm::translate(Scale, -GetModelCenter());

	mModelViewProj = Projection * View * Model;
	mModelView = View* Model;

	mModelViewInv = glm::inverse(mModelView);

	glm::mat3 NormalMatrix = glm::mat3(View);//glm::transpose(glm::inverse(glm::mat3(View)));

	GetRenderManager()->GetGpuParams()->SetParam(UniformName::matModelView, &mModelView[0][0], 1, 16);
	GetRenderManager()->GetGpuParams()->SetParam(UniformName::matModelViewProj, &mModelViewProj[0][0], 1, 16);
	GetRenderManager()->GetGpuParams()->SetParam(UniformName::matNormal, &NormalMatrix[0][0], 1, 9);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void  GLSample::recursiveMeshRendering(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh)
{
	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

	//apply material
		const struct aiMaterial* material = sc->mMaterials[mesh->mMaterialIndex];

		//aiString texPath;
		//material->GetTexture(aiTextureType_DIFFUSE,0,&texPath);

		ModelAndTextureLoader::MATERIAL_TEXTUREID textureIDs = fullModel->GetGLtextureOfMaterial(mesh->mMaterialIndex);

		glActiveTexture(GL_TEXTURE0);
		if (textureIDs.idDiffuse)
			SetUniform(bTex, 1);
		else
			SetUniform(bTex, 0);
		glBindTexture(GL_TEXTURE_2D, textureIDs.idDiffuse);

		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, textureIDs.idNormal);	
	
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, textureIDs.idSpecular);	

		C_STRUCT aiColor4D ambient;
		aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
		SetUniform(FrontMaterial_ambient, &ambient[0]);

		C_STRUCT aiColor4D diffuse;
		aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
		SetUniform(FrontMaterial_diffuse, &diffuse[0]);

		C_STRUCT aiColor4D specular;
		aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
		SetUniform(FrontMaterial_specular, &specular[0]);

		GetRenderManager()->UpdateCurrentShaderUniforms(false);

	//render mesh
		glBindVertexArray(fullModel->GetMeshVertexArray(*currentMesh));
		glDrawElements(GL_TRIANGLES, fullModel->GetNb3PointsFace(*currentMesh)*3, GL_UNSIGNED_INT, NULL);
		(*currentMesh) ++;
	}


	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		recursiveMeshRendering(sc, nd->mChildren[n],currentMesh);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::RenderModel()
{
	if (fullModel)
	{
		unsigned int iMesh = 0;
		recursiveMeshRendering(fullModel->GetAssimpScene(),fullModel->GetAssimpScene()->mRootNode,&iMesh);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::DrawBackground()
{
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, m_texBackground);
	GetRenderManager()->DrawScreenQuad(-1.89f, -1.89f, 0.2f, 0.2f, true);

	if (m_bDebugRTT)
	{
		// Debug SSAO
		if (m_bUseSSAO)
		{
			glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_SSAO_NORMAL));
			GetRenderManager()->DrawScreenQuad(-1.5f, -1.6f, 1.0f, 1.0f, false);

			glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_SSAO_RESULT));
			GetRenderManager()->DrawScreenQuad(-0.5f, -1.6f, 1.0f, 1.0f, false);
		}
		else if (m_bGlow)
		{
			glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_VIEWS, 2));
			GetRenderManager()->DrawScreenQuad(-1.5f, -1.6f, 1.0f, 1.0f, false);
		}
		else if (m_bShadow)
		{
			glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_SHADOW_HIGHRES));
			GetRenderManager()->DrawScreenQuad(-1.5f, -1.6f, 1.0f, 1.0f, false);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::DrawGlow()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glActiveTexture(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_VIEWS, 2));
	float scale[4] = {-1,-1,2,2};
	SetUniform(diffuse2D, 0);	// tex unit 0
	SetUniform(bTex, 0);
	SetUniform(textureSize, scale);

	GetRenderManager()->EnableShader(QuadTex);
	GetRenderManager()->DrawScreenQuadDirectWithTex();
	GetRenderManager()->DisableShader();
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GLSample::Render()
{
	SetDebug();		

	SetViewTransform();
	ResetMaterialProperties();

	m_bSSAO = false;
	// prepass that output both normal in one texture and depth in another
	if (m_bUseSSAO && GetRenderManager()->RenderSSAONormal())
		// actual screen space computation from depth and normal
		m_bSSAO = GetRenderManager()->RenderSSAOResult();

	m_bShadow = m_shadowStrength > 0 ? true : false;
	m_bDiffuse = m_texStrength > 0 ? true : false;
	m_bEnvmap = m_envStrength > 0 ? true : false;

	bool bShadow = m_bShadow && GetRenderManager()->RenderShadowMap();
	bool bGlow = m_bGlow && GetRenderManager()->RenderGlowMap();

	bool bOIT = m_bOIT && (m_fAlpha < 1.0f);

	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawBackground();

	bool bRenderTotarget = (m_blurLevel > 0) && GetRenderManager()->ActivateRenderTarget(eRTT_VIEWS);

	if (bRenderTotarget)
	{
		// warning: this will be the color blended to on AA or blurred edges: pick according to background
		glClearColor(0.75f,0.65f, 0.5f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
	}

	//transparency requires disabling depthwrite, culling, and enable double sided lighting
	if (m_fAlpha < 1.0f)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		//m_doubleSided = true;
		//m_shadowStrength = 0.2f;
	}

	if (bShadow)
	{
		// update m_mTextureProjectionShadow from base with CURRENT PASS inverse modelview
		GetRenderManager()->m_mTextureProjectionShadow = GetRenderManager()->m_mTextureProjectionShadowBase * mModelViewInv;
	}

	if (bOIT)
	{
		//glDisable(GL_BLEND);								// Turn Blending Off
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		//m_doubleSided = true;

		GetRenderManager()->ClearAppendBuffers();
	}

	SetUniform(numLights, m_numLights);
	SetUniform(bDoubleSided, m_doubleSided ? 1 : 0);
	SetUniform(bShadowMap, bShadow ? 1 : 0);
	if (bShadow)
		SetUniform(shadowStrength, m_shadowStrength);

	SetUniform(bTex, m_texStrength > 0 ? 1 : 0);
	SetUniform(texStrength, m_texStrength);

	SetUniform(bBumpMapping, m_bNormal ? 1 : 0);

	SetUniform(bEnvTex, m_bEnvmap ? 1 : 0);
	SetUniform(envStrength, m_envStrength);

	SetUniform(bSSAO, m_bSSAO ? 1: 0);
	SetUniform(bOIT, bOIT ? 1 : 0);

	SetUniform(fAlphaOverride, m_fAlpha);
	bool bShader = GetRenderManager()->EnableShader(UberShader);

	// ACTUAL MODEL RENDER
	RenderModel();

	if (bOIT)
	{
		GetRenderManager()->ResolveAppendBuffers();
	}

	if (m_fAlpha < 1.0f)
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		//glEnable(GL_CULL_FACE);
	}


	GetRenderManager()->DisableShader();

	if (bRenderTotarget)
	{
		GetRenderManager()->ReleaseActiveRenderTarget();

		glDisable(GL_CULL_FACE);
		if (m_blurLevel > 0)
			GetRenderManager()->BlurTarget(eRTT_VIEWS, 0, 1, (int)m_blurLevel);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, GetRenderManager()->GetTexture(eRTT_VIEWS, 0));
		float scale[4] = {-1,-1,2,2};
		SetUniform(diffuse2D, 0);	// tex unit 0
		SetUniform(bTex, 0);
		SetUniform(textureSize, scale);
		GetRenderManager()->EnableShader(QuadTex);
		GetRenderManager()->DrawScreenQuadDirectWithTex();
		GetRenderManager()->DisableShader();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	if (m_bGlow)
		DrawGlow();

	CheckOglError();
	glFlush();
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
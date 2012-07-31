#pragma once

#include <map>

class RenderToTarget;
class GpuParams;
class GLSLShader;
class GLApp;


#define MAX_SORTED_FRAGMENT 20
#define GL_ATOMIC_COUNTER_BUFFER_EXT                       0x92C0
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT_EXT             0x00000020

typedef GLvoid* (APIENTRY *PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);

enum renderToTargetNature_e
{ 
	eRTT_VIEWS = 0,
	eRTT_SHADOW_HIGHRES,
	eRTT_SSAO_NORMAL,
	eRTT_SSAO_RESULT,

	eRTT_MAX
};


typedef struct _structRenderTargetInfo
{
	int iWidth;
	int iHeight;
	GLenum	eFormat;
	GLenum	eType;
	GLint	iFiltering;

	bool	bMipMap;			// generate mipmap chain
	int		iMultiSample;		// enable AA when rendering to FBA
	float	fSuperSample;		// 0 = specified size, otherwise a factor of the current modelview size
	int		iWidthSS;
	int		iHeightSS;
	bool	bStencil;			// need stencil layer
	bool 	bPhysicalScreen;	// use physical monitor size
	bool	m_bBindDepth;		// when not a depth only target, also bind depth as an additional texture

	int nbTextures;
	GLuint	*tex;
	GLuint	*texDepth;
	bool	*used;

	RenderToTarget * pRTT;
} structRenderTargetInfo;



class RenderManager
{
public:
	RenderManager(GLApp * pApp);
	~RenderManager(void);

	bool	Initialize();
	void	Deinitialize();
	void	Update(unsigned long milliseconds);
	void	OnResize(int width, int height);

	bool	VerifyRenderTarget(renderToTargetNature_e eRTTNature);

	void SetupOpenGLStates();

	GLApp * GetApp()	{ return m_pApp;	}

	bool RenderShadowMap();
	bool RenderSSAONormal();
	bool RenderSSAOResult();
	bool RenderGlowMap();


	/////////////////////////////////////////////////////////////
	// RenderToTarget resource management part
	bool ActivateRenderTarget(renderToTargetNature_e eRTTNature, int iTex = 0, bool bValidationOnly = false);
	bool ReleaseActiveRenderTarget();
	GLuint GetTexture(renderToTargetNature_e eRTTNature, int iTex = 0, bool bDepth = false);
	glm::vec2 GetRenderingTargetSize(renderToTargetNature_e eRTTNature);
	void GetCurrentRenderingTargetSize(int& width, int& height);
	bool BlurTarget(renderToTargetNature_e eRTTNature, int targetId, int targetTemp, int blurLevel = 8, float focus = 1.0f, float clearCol = 0.0f, float clearAlpha = 0.0f );
	float GetTargetSuperSample(renderToTargetNature_e eRTTNature)			{ return m_RTTInfo[eRTTNature].fSuperSample;	}
	bool IsRenderingToTarget()	{ return m_pCurrentRenderToTarget != NULL; }

	const structRenderTargetInfo & getRTTinfo(renderToTargetNature_e eRTTNature) { return m_RTTInfo[eRTTNature];	}
	renderToTargetNature_e const getRenderToTargetNature();
	bool	InitSecondaryLayerFromMainlayer(bool color, bool depth);

	RenderToTarget * GetRenderTarget(renderToTargetNature_e eRTTNature);
	renderToTargetNature_e const GetRenderToTargetNature();

	// ABuffer for OIT
	void CreateAppendBuffers(int iWidth, int iHeight);
	void ResizeAppendBuffers(int iWidth, int iHeight);
	void DeleteAppendBuffers();
	void ClearAppendBuffers();
	void ResolveAppendBuffers();

	// for all
	void InitRenderTargetInfos();
	void CleanRenderTargetInfos(bool bExit);

	// for one
	void InitRenderTargetInfo(renderToTargetNature_e eRTTNature, bool bFirstTime);
	void CleanRenderTargetInfo(renderToTargetNature_e eRTTNature, bool bExit);

	// Shader
	GpuParams * GetGpuParams()			{ return m_GpuParams;	}
	GLSLShader * CreateShader(unsigned int shaderProfile);
	bool EnableShader(unsigned int shaderProfile);
	void DisableShader();
	bool UpdateCurrentShaderUniforms(bool forceUpdate = false);
	GLSLShader * GetShaderFromProfile(unsigned int shaderProfile);
	void SetupShaderProfile(unsigned int shaderProfile);
	void CleanShaderProfile(unsigned int shaderProfile);

	// cache for fast access
	glm::mat4		m_mTextureProjectionShadowBase;		// cache Base texture projection matrix (for shadowmap)
	glm::mat4		m_mTextureProjectionShadow;			// Base + inverse of CURRENT PASS modelview

	void DrawScreenQuad(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f, bool bVFlip= true);
	void DrawScreenQuadDirect();
	void DrawScreenQuadDirectWithTex();

private:
	bool m_bInitialized;

	RenderToTarget * m_pCurrentRenderToTarget;
	structRenderTargetInfo m_RTTInfo[eRTT_MAX];

	// Shader
	GpuParams * m_GpuParams;
	typedef std::map<unsigned int, GLSLShader *>	ShaderMap;
	ShaderMap m_shaderMap;
	GLSLShader * m_pActiveShader;

	unsigned long m_time;
	GLApp * m_pApp;

	// OIT
	GLuint	m_globalbuffer[2];
	GLuint	m_atomicCounter;
	GLuint	m_linkedListBuffer;

	// DrawScreenQuad
	GLuint m_QuadVA;
	GLuint m_QuadVBV;
	GLuint m_QuadVBT;
};

typedef struct LinkedListBufferSturct
{
	unsigned char color[4];
	float depth;
	unsigned int pNext;
    unsigned int reserverd;
} LinkedListBuffer;

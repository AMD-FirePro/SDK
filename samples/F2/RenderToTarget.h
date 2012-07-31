#pragma once



/////////////////////////////////////////////////////////////////////////////////
// RenderToTarget base class, resource managed by the RenderManager
class RenderToTarget
{
public:
	RenderToTarget();
	virtual ~RenderToTarget();

	virtual bool Init(const structRenderTargetInfo & rttInfo) = 0;
	virtual void Clean() = 0;
	virtual bool RenderBegin(GLuint * tex, GLuint * texDepth) = 0;
	virtual bool RenderEnd() = 0;
	void SetNature(renderToTargetNature_e nature)	{ m_eRTTNature = nature; }
	renderToTargetNature_e GetNature()				{ return m_eRTTNature; }

	// FBO specific
	virtual bool Restore(bool color, bool depth)	{ return false; }
	virtual GLuint GetFBOhandle()	{ return 0; }
	virtual GLuint GetFBOhandleTarget()	{ return 0; }

	virtual void ResetCurrentTexture(int tex, int texDepth)	{ }


protected:
	int		m_iWidth;
	int		m_iHeight;
	int		m_iUsedWidth;
	int		m_iUsedHeight;
	GLint	m_iFiltering;
	GLenum	m_eFormat;
	GLenum	m_eType;

	// these only works with FBOs:
	bool	m_bMipMap;			// generate mipmap chain
	bool	m_bBindDepth;		// also bind depth as a texture, even if not depthOnly
	int		m_iMultiSample;		// enable AA when rendering to FBA
	int		m_iSuperSample;		// 0 = specified size, otherwise a factor of the current modelview size


	bool	m_bInitialized;
	renderToTargetNature_e m_eRTTNature;
};


class RenderToFrameBuffer : public RenderToTarget
{
public:
	RenderToFrameBuffer();
	virtual ~RenderToFrameBuffer();

	static bool IsAvailable();

	virtual bool Init(const structRenderTargetInfo & rttInfo);
	virtual void Clean();

	virtual bool RenderBegin(GLuint * tex, GLuint * texDepth);
	virtual bool RenderEnd();
	virtual bool Restore(bool color, bool depth);
	virtual GLuint GetFBOhandle()		{ return m_fboMS ? m_fboMS : m_fbo; }
	virtual GLuint GetFBOhandleTarget()	{ return m_fbo; }

	void ResetCurrentTexture(int tex, int texDepth);

private:
	GLuint m_fbo;					// handle to the FBO
	GLuint m_depthBuffer;			// handle to the depth render buffer
	GLuint m_fboMS;					// handle to the MultiSampled (if any) FBO
	GLuint m_colorMS;				// handle to the color Multisample (if any) render buffer

	GLint m_currentTex;
	GLint m_currentTexDepth;		// in case we wante dpeth to be bound in addition to color (if depth only, tex is used)
};


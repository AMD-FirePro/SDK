#include "utils.hpp"
#include "rendermanager.hpp"
#include "rendertotarget.hpp"

///////////////////////////////////////////////////////////////////////////////////////
// RenderToTarget
///////////////////////////////////////////////////////////////////////////////////////
RenderToTarget::RenderToTarget() :
	m_bInitialized(false),
	m_iWidth(0),
	m_iHeight(0),
	m_iUsedWidth(0),
	m_iUsedHeight(0),
	m_eFormat(GL_RGBA),
	m_eType(GL_UNSIGNED_BYTE),
	m_iFiltering(GL_NEAREST),
	m_eRTTNature(eRTT_MAX),
	m_bMipMap(false),
	m_iMultiSample(0),
	m_iSuperSample(0),
	m_bBindDepth(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////
RenderToTarget::~RenderToTarget()
{
}




///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// FBO
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

// the CSAA/MSAA modes depend on the # of color samples and # of coverage samples
#define MSAA_4x		4, 0
#define CSAA_8x		4, 8
#define CSAA_8xQ	8, 8
#define CSAA_16x	4, 16
#define CSAA_16xQ	8, 16
#define MAX_AAModes		5
struct SAAMode { int ds; int cs; };
SAAMode g_AAModes[] = {
    {MSAA_4x},
    {CSAA_8x},
    {CSAA_8xQ},
    {CSAA_16x},
    {CSAA_16xQ}
};

///////////////////////////////////////////////////////////////////////////////////////
// RenderToFrameBuffer
///////////////////////////////////////////////////////////////////////////////////////

bool RenderToFrameBuffer::IsAvailable()
{
	static bool alreadyTested = false;
	static bool result;
	if (alreadyTested)
		return result;
/*
	if (!glewIsSupported("GL_EXT_framebuffer_object "))
	{
		result = Bool4(false, false, false, false);
		alreadyTested = true;
		return result;
	}
*/
	bool bFrameBufferAvailable =   (glGenFramebuffersEXT != NULL) &&
								(glBindFramebufferEXT != NULL) &&
								(glGenRenderbuffersEXT != NULL) &&
								(glBindRenderbufferEXT != NULL) &&
								(glRenderbufferStorageEXT != NULL) &&
								(glFramebufferTexture2DEXT != NULL) &&
								(glFramebufferRenderbufferEXT != NULL) &&
								(glDeleteFramebuffersEXT != NULL) &&
								(glDeleteRenderbuffersEXT != NULL);
	
	bool bFrameBufferMipMapAvaialble = (glGenerateMipmapEXT != NULL);

	bool bFrameBufferMultiSample = 	(glRenderbufferStorageMultisampleEXT != NULL) &&
									(glGetRenderbufferParameterivEXT != NULL);

	bool bBlitFramebuffer = (glBlitFramebufferEXT != NULL);

	result = bFrameBufferAvailable;//, bFrameBufferMipMapAvaialble, bFrameBufferMultiSample, bBlitFramebuffer);
	alreadyTested = true;
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RenderToFrameBuffer::RenderToFrameBuffer()
:	m_fbo(0),
	m_depthBuffer(0),
	m_colorMS(0),
	m_fboMS(0),
	m_currentTex(0),
	m_currentTexDepth(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////
RenderToFrameBuffer::~RenderToFrameBuffer()
{
	Clean();
}



///////////////////////////////////////////////////////////////////////////////////////
bool RenderToFrameBuffer::Init(const structRenderTargetInfo & rttInfo)
{
	// Setup our FBO
	glGenFramebuffersEXT(1, &m_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	if ((rttInfo.fSuperSample > 0 || rttInfo.bPhysicalScreen) && rttInfo.iWidthSS > 0 && rttInfo.iHeightSS > 0)
	{
		m_iWidth = (int)((float)rttInfo.iWidthSS * rttInfo.fSuperSample + 0.5f);
		m_iHeight = (int)((float)rttInfo.iHeightSS * rttInfo.fSuperSample + 0.5f);
	}
	else
	{
		m_iWidth = rttInfo.iWidth;
		m_iHeight = rttInfo.iHeight;
	}

	bool bCoverage = false;
	if ((rttInfo.iMultiSample > 0) && RenderToFrameBuffer::IsAvailable()/*[2]*/)
	{
		m_iMultiSample = rttInfo.iMultiSample;
		if (m_iMultiSample > MAX_AAModes)
			m_iMultiSample = MAX_AAModes;

		// coverage supported ?
//		if (glRenderbufferStorageMultisampleCoverageNV)
//			bCoverage = true;
//		else
			bCoverage = false;
	}
	else
		m_iMultiSample = 0;

	int ds = 0;
	int cs = 0;
	if (m_iMultiSample > 0)
	{
		ds = g_AAModes[m_iMultiSample-1].ds;
		cs = bCoverage ? g_AAModes[m_iMultiSample-1].cs : 0;
	}

	if (rttInfo.eFormat != GL_DEPTH_COMPONENT)
	{
		// Create the render buffer for depth	
		glGenRenderbuffersEXT(1, &m_depthBuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer);

		GLenum	internalFormat = rttInfo.bStencil ? GL_DEPTH24_STENCIL8_EXT : GL_DEPTH_COMPONENT;
		if (m_iMultiSample > 0)
		{
	        glGenFramebuffersEXT(1, &m_fboMS);
	        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboMS);

			if (cs==0)
			{
				glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, ds, internalFormat, m_iWidth, m_iHeight);
	            // check the number of samples
				GLint qds = 0;
		        glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &qds);
				if (qds < ds)
				{
					Clean();
					return false;
				}
			}
			else
			{
//				glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER_EXT, cs, ds, internalFormat, m_iWidth, m_iHeight);
	            // check the number of samples
				GLint qds = 0;
				GLint qcs = 0;
	            glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_COVERAGE_SAMPLES_NV, &qcs);
	            glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_COLOR_SAMPLES_NV, &qds);
				if ((qcs < cs) || (qds < ds))
				{
					Clean();
					return false;
				}
			}
		}
		else
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, m_iWidth, m_iHeight);


		// Attach the depth render buffer to the FBO as it's depth attachment
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer);
		if (rttInfo.bStencil)
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthBuffer); 


		if (m_iMultiSample > 0)
		{
	        glGenRenderbuffersEXT(1, &m_colorMS);
	        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_colorMS);

			if (cs==0)
			{
				glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, ds, GL_RGBA8, m_iWidth, m_iHeight);
	            // check the number of samples
				GLint qds = 0;
		        glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &qds);
				if (qds < ds)
				{
					Clean();
					return false;
				}
			}
			else
			{
//				glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER_EXT, cs, ds, GL_RGBA8, m_iWidth, m_iHeight);
	            // check the number of samples
				GLint qds = 0;
				GLint qcs = 0;
	            glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_COVERAGE_SAMPLES_NV, &qcs);
	            glGetRenderbufferParameterivEXT( GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_COLOR_SAMPLES_NV, &qds);
				if ((qcs < cs) || (qds < ds))
				{
					Clean();
					return false;
				}
			}

	        // attach the multisampled color buffer
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, m_colorMS);
		}
	}
	else
	{
		// The following is REQUIRED or the FBO will not be resolved (since we have no color buffer here)
		// Also note that these states are only valid for the FBO state, so can be done once during init
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	m_eFormat = rttInfo.eFormat;
	m_eType = rttInfo.eType;
	m_iFiltering = rttInfo.iFiltering;
	m_bMipMap = RenderToFrameBuffer::IsAvailable()/*[1]*/ && rttInfo.bMipMap;
	m_bBindDepth = rttInfo.m_bBindDepth;

	m_bInitialized = true;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);	// Unbind the FBO for now

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
void RenderToFrameBuffer::Clean()
{
	if (m_depthBuffer)
		glDeleteRenderbuffersEXT(1, &m_depthBuffer);
	if (m_colorMS)
		glDeleteRenderbuffersEXT(1, &m_colorMS);
	if (m_fbo)
		glDeleteFramebuffersEXT(1, &m_fbo);
	if (m_fboMS)
		glDeleteFramebuffersEXT(1, &m_fboMS);
	m_bInitialized = false;
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderToFrameBuffer::RenderBegin(GLuint * tex, GLuint * texDepth)
{
	if (!m_bInitialized)
		return false;

	bool bCreateTex = (*tex == 0);
		//!glIsTexture(*tex);
	if (bCreateTex)
	{
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// Now setup a texture to render to
		glGenTextures(1, tex);
		glBindTexture(GL_TEXTURE_2D, *tex);
		glTexImage2D(GL_TEXTURE_2D, 0, m_eFormat,  m_iWidth, m_iHeight, 0, m_eFormat, m_eType, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_iFiltering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_iFiltering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Enable mipmap filtering and generate the mipmap data
		if (m_bMipMap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmapEXT(GL_TEXTURE_2D);
		}
	}

	bool bCreateTexDepth = m_bBindDepth && (*texDepth == 0);
	if (bCreateTexDepth)
	{
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// Now setup a texture to render to
		glGenTextures(1, texDepth);
		glBindTexture(GL_TEXTURE_2D, *texDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  m_iWidth, m_iHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_iWidth, m_iHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// bind the tex if its a different one
	if (bCreateTex || (*tex != m_currentTex) || bCreateTexDepth || (*texDepth != m_currentTexDepth))
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// attach it to the FBO so we can render to it
		if (m_eFormat == GL_DEPTH_COMPONENT)
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, *tex, 0);
		else
		{
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, *tex, 0);
			if (*texDepth)
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, *texDepth, 0);
		}

		// if this is for a newly created texture, verify that the FBO is consistent
		if (bCreateTex)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboMS ? m_fboMS : m_fbo);
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				Clean();
				return false;
			}
		}
	}

	m_currentTex = *tex;
	m_currentTexDepth = *texDepth;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fboMS ? m_fboMS : m_fbo);
	glViewport(0, 0, m_iWidth, m_iHeight);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
bool RenderToFrameBuffer::RenderEnd()
{
	{
		//Xpl_CriticalSection::StLock lock(desktopDomainLock);
		glFlush();
	}
	
	if (m_fboMS)
    {
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, m_fboMS);
        glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, m_fbo);
        glBlitFramebufferEXT( 0, 0, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (m_bMipMap)
	{
		glBindTexture(GL_TEXTURE_2D, m_currentTex);
		glGenerateMipmapEXT(GL_TEXTURE_2D);
	}
	
	return CheckOglError();
}

///////////////////////////////////////////////////////////////////////////////////////
bool RenderToFrameBuffer::Restore(bool color, bool depth)
{
	if (!color && !depth)
		return false;

//	if (!glBindFramebufferEXT || !glBlitFramebufferEXT)
//		return FALSE;

	if (m_fboMS)
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, m_fboMS);
	else
        glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, m_fbo);

	glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0);

	GLbitfield bits = 0;
	if (color)
		bits |= GL_COLOR_BUFFER_BIT;
	if (depth)
		bits |= GL_DEPTH_BUFFER_BIT;

	glBlitFramebufferEXT( 0, 0, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, bits, GL_NEAREST);
	return CheckOglError();
}


void RenderToFrameBuffer::ResetCurrentTexture(int tex, int texDepth)
{
	if (tex && tex == m_currentTex)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
		// Unbind current attachment texture before deleting the texture, or it would crash on exit
		if (m_eFormat == GL_DEPTH_COMPONENT)
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
		else
		{
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
			if (texDepth && texDepth == m_currentTexDepth)
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
		}
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		m_currentTex = 0;
		m_currentTexDepth = 0;
	}
}

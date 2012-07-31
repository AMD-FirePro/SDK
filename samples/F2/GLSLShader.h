#ifndef _GLSLSHADER_H_
#define _GLSLSHADER_H_

#include "GLSLBinding.h"


// NEED To BE IN SYNC WITH shaderTemplateName
enum shaderTemplate
{
	UberShader		= 1,
	ShadowPass,

	ComputeSSAO,
	OutputNormal,

	BlurPass,

	AppendClear,
	AppendResolve,

	QuadTex,
	GlowPass,

	shaderTemplateMax
};


class GLSLShader
{
public:
	//--------------------------------------------------
	//!
	//!	define one uniform (not its value, just where and what he is)
	//!
	//--------------------------------------------------
	class GLSLAttribute
	{
	public:
		std::string name;
		GLint size;
		GLenum type;
		GLint  location;
	public:
		GLSLAttribute();
		GLSLAttribute(GLhandleARB hProgram, int index);
		bool IsBuiltin() const;
		bool IsValid() const;
	};

public:
	GLSLShader();

	virtual ~GLSLShader();
	
	//--------------------------------------------------
	//!
	//!	Initialisation
	//!	\param filename to load the code from (no extension, .vert and .frag will be added)  
	//!	\param global gpu-param to cache value before sending. Can be 0
	//!
	//--------------------------------------------------
	bool Init(unsigned int, GpuParams * params);
	
	void Clean();
	bool Activate();
	bool Deactivate();
	bool IsActive() const;

	GLSLBindings& GetBindings() {return m_UniformBinding;}
	const GLSLBindings& GetBindings() const {return m_UniformBinding;}
	bool WillRunSoftware(GLhandleARB hProgram);

	unsigned int GetProfile() const		{ return m_shaderProfile;	}

private:
	// create and compile and GLSL object
	GLhandleARB CreateObject(const std::string& prog, GLenum progType);
	// do the core if the init
	bool CreateGPUProgram(const std::string& vertexProg, const std::string& shaderProg, GpuParams * params);
	// sanity check
	bool SanityCheck();
protected:
	// Files if custom
	std::string m_ProgramName;

	// program object if init is sucessful
	GLhandleARB  m_programObject;

	// true if GLSL is available
	bool  m_bGLSLAvailable;

	// true is shader is active
	bool  m_bShaderActive;
	
	unsigned int m_shaderProfile;

	// list of active uniform
	GLSLBindings	m_UniformBinding;

};




#endif // end of _GLSLSHADER_H_

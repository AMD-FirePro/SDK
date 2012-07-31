#include "utils.hpp"
#include "glslshader.hpp"
#include <fstream>

// NEED To BE IN SYNC WITH enum shaderTemplate
const char * shaderTemplateName[shaderTemplateMax] = {

	"UberShader",
	"ShadowPass",

	"ComputeSSAO",
	"OutputNormal",

	"BlurPass",

	"AppendClear",
	"AppendResolve",

	"QuadTex",
	"GlowPass"
};


//////////////////////////////////////////////////////////////////////
std::string ReadFileContent(const std::string& filename)
{
	std::ifstream ifs;
	ifs.open(filename.c_str());
	if(ifs.bad())
	{
		return std::string();
	}
	
	std::string res((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	return res;
}

//////////////////////////////////////////////////////////////////////
void WriteFileContent(const std::string& filename, const std::string& filecontent)
{
	std::ofstream myfile;
	myfile.open(filename.c_str());
	myfile << filecontent;
	myfile.close();
}

///////////////////////////////////////////////////////////////////////////////////////
GLSLShader::GLSLShader()
	:m_programObject(0)
	,m_bGLSLAvailable(true)
	,m_bShaderActive(false)
	,m_shaderProfile(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////
GLSLShader::~GLSLShader()
{
	Clean();
}

///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::IsActive() const
{
	return m_bShaderActive;
}

///////////////////////////////////////////////////////////////////////////////////////
void OutputLog(GLuint obj)
{
	int infologLength = 0;
	int maxLength = 256;
	
	if(glIsShader(obj))
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	else
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
			
	char * infoLog = new char[maxLength];
 
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
 
	if (infologLength > 0)
		OutputDebugString(infoLog);

	delete infoLog;
}
///////////////////////////////////////////////////////////////////////////////////////
GLhandleARB GLSLShader::CreateObject(const std::string& prog, GLenum progType)
{
	// create
	GLhandleARB obj = glCreateShaderObjectARB(progType);
	if(!CheckOglError())
	{
		return static_cast<GLhandleARB>(0);
	}

	// set source
	GLint size = static_cast<GLint>(prog.size());
	const GLcharARB* progString = static_cast<const GLcharARB*>(&prog[0]);
	glShaderSource(obj, 1, &progString, &size);
	if(!CheckOglError())
	{
		glDeleteObjectARB(obj);
		return static_cast<GLhandleARB>(0);
	}

	// compile
	glCompileShader(obj);
	GLint compiled; 
	glGetObjectParameterivARB(obj, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	OutputLog(obj);
	if (compiled==0 )
	{
		return static_cast<GLhandleARB>(0);
	}

	// return
	return obj;
}

///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::CreateGPUProgram(const std::string& vertexProg, const std::string& fragmentProg, GpuParams * params)
{
 	// vertex
	GLhandleARB vtxObj = CreateObject(vertexProg,GL_VERTEX_SHADER_ARB);
	if(vtxObj==static_cast<GLhandleARB>(0))
	{
		OutputDebugString("Error while creating Vertex shader\n");
		return false;
	}
	// pixel
	GLhandleARB fragObj = CreateObject(fragmentProg,GL_FRAGMENT_SHADER_ARB);
	if(fragObj==static_cast<GLhandleARB>(0))
	{
		OutputDebugString("Error while creating Fragment shader\n");
		return false;
	}

	// program - create
	GLhandleARB progObj = glCreateProgramObjectARB();
	if(!CheckOglError())
	{
		OutputDebugString("Error While creating program");
		glDeleteObjectARB(vtxObj);
		glDeleteObjectARB(fragObj);
		return false;
	}

	// program - attached
	glAttachObjectARB(progObj, vtxObj);
	glAttachObjectARB(progObj, fragObj);
	if(!CheckOglError())
	{
		OutputDebugString("Error While attaching object");
		glDeleteObjectARB(vtxObj);
		glDeleteObjectARB(fragObj);
		glDeleteObjectARB(progObj);
		return false;
	}

	// Don't need the shader objs any more
	glDeleteObjectARB(vtxObj);
	glDeleteObjectARB(fragObj);
	if(!CheckOglError())
	{
		OutputDebugString("Error While deleting object");
		glDeleteObjectARB(progObj);
		return false;
	}
	
	// program - link
	GLint linked = TRUE;
	{
		try
		{
			glLinkProgram(progObj);
		}
		catch(...)
		{
			linked = FALSE;
			OutputDebugString("ERROR in GLSLShader: Cannot CreateGPUProgram - link\n");
		}
	}

	OutputLog(progObj);

	if (linked)
	{
		glGetObjectParameterivARB(progObj, GL_OBJECT_LINK_STATUS_ARB, &linked);
		if (linked && WillRunSoftware(progObj))
			linked = FALSE;
	}

	if (!linked)
	{
		OutputDebugString("Error While linking object!\n");
		glDeleteObjectARB(progObj);
		return false;
	}
	
	// unform stuff
	m_UniformBinding.Finalize(progObj, params);

	// we are done with opengl
	m_programObject = progObj;
	
	// should be true
	return CheckOglError();
}

///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::SanityCheck()
{
	// Return if GLSL is not available, eventually will trigger fallback to fixed-pipeline
	if (!m_bGLSLAvailable)
	{
		OutputDebugString("GLSLShader_c::Init: GLSL not available!\n");
		return false;
	}

	// Check errors
	if (!CheckOglError())
	{
		OutputDebugString("GLSLShader_c::Init: there are previous OPENGL error!\n");
		return false;
		
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::Init(unsigned int shaderProfile, GpuParams * params)
{
	m_shaderProfile = shaderProfile;

	// Return if GLSL is not available, eventually will trigger fallback to fixed-pipeline
	if (!SanityCheck())
	{
		//GL::NotifyFailure();
		return false;
	}

	int id = (shaderProfile & 0x000000FF) - 1;
	if (id < 0 || id > shaderTemplateMax)
		return false;

	m_ProgramName = shaderTemplateName[id];

	// filename
	std::string VertexProgramName = m_ProgramName + ".vert";
	std::string FragmentProgramName = m_ProgramName + ".frag";

	// path
	std::string ProgramPath = "Shaders/";
	
	// load
	std::string vertexShaderCode = ReadFileContent(ProgramPath + VertexProgramName);
	std::string pixelShaderCode = ReadFileContent(ProgramPath + FragmentProgramName);
	if(vertexShaderCode.size()==0 || pixelShaderCode.size()==0)
	{
		OutputDebugString("GLSLShader_c::Init: Cannot load code from file!\n");
		return false;
	}
	
	// create shader
	char tmp[256];
	sprintf_s(tmp, 255, "\n----- SHADER CREATION: %s -----\n", m_ProgramName.c_str());
	OutputDebugString(tmp);

	if(!CreateGPUProgram(vertexShaderCode, pixelShaderCode, params))
	{
		OutputDebugString("GLSLShader_c::Init: Something wrong during the init...\n");
		return false;
	}

	// everything looks good!
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
void GLSLShader::Clean()
{
	if (m_programObject)
	{
		glDeleteObjectARB(m_programObject);
		CheckOglError();
	}
	m_programObject = 0;
}



///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::Activate()
{
	//assert(!m_bShaderActive);	// OZ: if we are here, we forgot to call Finish

	CheckOglError();

	glUseProgramObjectARB(m_programObject);
	if(!CheckOglError())
	{
		glUseProgramObjectARB(0);
		return false;
	}
	
	m_bShaderActive = true;
	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::Deactivate()
{
	if (m_bShaderActive)  // Avoid when following a failed Setup()
	{
		// Load the NULL shader to revert to fixed fxn OGL
		glUseProgramObjectARB(0);
		m_bShaderActive = false;
	}

	// Check errors
	return CheckOglError();
}

///////////////////////////////////////////////////////////////////////////////////////
bool GLSLShader::WillRunSoftware(GLhandleARB hProgram)
{
/*
	const GLsizei logSize = 4096;
	GLcharARB log[logSize]; log[0]='\0';
	glGetInfoLogARB(hProgram, logSize, NULL, log);
	CString findSoftwareRun(log);
	return (findSoftwareRun.Find(_T("software")) != -1);
*/
	return false;
}



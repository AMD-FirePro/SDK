

#include <amd/amd.hpp>


namespace
{
	extern bool isWireFrame;
	extern bool drawInputMesh;

	std::string const SAMPLE_NAME = "OpenGL Tessellation";	
	std::string const SAMPLE_VERTEX_SHADER(amd::DATA_DIRECTORY + "tess.vert");
	std::string const SAMPLE_CONTROL_SHADER(amd::DATA_DIRECTORY + "tess.cont");
	std::string const SAMPLE_EVALUATION_SHADER(amd::DATA_DIRECTORY + "tess.eval");
	std::string const SAMPLE_FRAGMENT_SHADER(amd::DATA_DIRECTORY + "tess.frag");
	std::string const SAMPLE_VERTEX_SHADER2(amd::DATA_DIRECTORY + "drawInputMesh.vert");
	std::string const SAMPLE_FRAGMENT_SHADER2(amd::DATA_DIRECTORY + "drawInputMesh.frag");
	int const SAMPLE_SIZE_WIDTH(800);
	int const SAMPLE_SIZE_HEIGHT(600);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount(16);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec3);
	const float VertexData[VertexCount*3] =
	{
	   1.000000, 1.000000, 1.000000, 1.000000, -1.000000, 1.000000, -1.000000, -1.000000, 1.000000, -1.000000, 1.000000, 1.000000,
	   1.000000, -1.000000, -1.000000, 1.000000, 1.000000, -1.000000, -1.000000, 1.000000, -1.000000, -1.000000, -1.000000, -1.000000,
	   3.000000, 2.000000, 3.000000, -3.000000, 2.000000, 3.000000, -3.000000, -2.000000, 3.000000, 3.000000, -2.000000, 3.000000,
	   3.000000, 2.000000, -3.000000, 3.000000, -2.000000, -3.000000, -3.000000, 2.000000, -3.000000, -3.000000, -2.000000, -3.000000,
	};
		
	GLsizei const PatchIndicesCount = 16*16;
	GLsizeiptr const PatchIndicesSize = PatchIndicesCount * sizeof(glm::uint32);
	glm::uint32 PatchIndices[PatchIndicesCount] = 
	{
		0, 1, 2, 3, 9, 8, 12, 5, 4, 13, 11, 10, 15, 7, 6, 14, 
		5, 4, 1, 0, 8, 12, 14, 6, 7, 15, 13, 11, 10, 2, 3, 9, 
		6, 3, 2, 7, 4, 5, 12, 14, 9, 8, 0, 1, 11, 10, 15, 13, 
		6, 7, 4, 5, 12, 14, 9, 3, 2, 10, 15, 13, 11, 1, 0, 8, 
		8, 9, 10, 11, 13, 12, 5, 0, 3, 6, 14, 15, 7, 2, 1, 4, 
		12, 8, 11, 13, 15, 14, 6, 5, 0, 3, 9, 10, 2, 1, 4, 7, 
		14, 12, 13, 15, 10, 9, 3, 6, 5, 0, 8, 11, 1, 4, 7, 2, 
		14, 15, 10, 9, 3, 6, 5, 12, 13, 4, 7, 2, 1, 11, 8, 0, 
		0, 3, 9, 8, 12, 5, 4, 1, 2, 7, 6, 14, 15, 10, 11, 13, 
		5, 0, 8, 12, 14, 6, 7, 4, 1, 2, 3, 9, 10, 11, 13, 15, 
		6, 5, 12, 14, 9, 3, 2, 7, 4, 1, 0, 8, 11, 13, 15, 10, 
		6, 14, 9, 3, 2, 7, 4, 5, 12, 13, 15, 10, 11, 8, 0, 1, 
		1, 11, 10, 2, 3, 0, 5, 4, 13, 12, 8, 9, 14, 15, 7, 6, 
		4, 13, 11, 1, 0, 5, 6, 7, 15, 14, 12, 8, 9, 10, 2, 3, 
		7, 15, 13, 4, 5, 6, 3, 2, 10, 9, 14, 12, 8, 11, 1, 0, 
		7, 2, 10, 15, 13, 4, 5, 6, 3, 0, 1, 11, 8, 9, 14, 12, 
	};
		
	GLsizei const FaceIndicesCount = 16*4;
	GLsizeiptr const FaceIndicesSize = FaceIndicesCount * sizeof(glm::uint32);
	glm::uint32 FaceIndices[FaceIndicesCount] = 
	{
      0, 1, 2, 3,  
	  5, 4, 1, 0,  
	  6, 3, 2, 7,  
	  6, 7, 4, 5,  
	  8, 9, 10, 11,  
	  12, 8, 11, 13,  
	  14, 12, 13, 15,  
	  14, 15, 10, 9,  
	  0, 3, 9, 8,  
	  5, 0, 8, 12,  
	  6, 5, 12, 14,  
	  6, 14, 9, 3,  
	  1, 11, 10, 2,  
	  4, 13, 11, 1,  
	  7, 15, 13, 4,  
	  7, 2, 10, 15,  
	};
	glm::uint32 LineIndices[FaceIndicesCount*2];

	const float weights[16*16]= {
		0.444444, 0.444444, 0.222222, 0.111111, 0.444444, 0.444444, 0.222222, 0.111111, 0.222222, 0.222222, 0.111111, 0.055556, 0.111111, 0.111111, 0.055556, 0.027778, 
		0.111111, 0.222222, 0.444444, 0.444444, 0.111111, 0.222222, 0.444444, 0.444444, 0.055556, 0.111111, 0.222222, 0.222222, 0.027778, 0.055556, 0.111111, 0.111111, 
		0.027778, 0.055556, 0.111111, 0.111111, 0.055556, 0.111111, 0.222222, 0.222222, 0.111111, 0.222222, 0.444444, 0.444444, 0.111111, 0.222222, 0.444444, 0.444444,
		0.111111, 0.111111, 0.055556, 0.027778, 0.222222, 0.222222, 0.111111, 0.055556, 0.444444, 0.444444, 0.222222, 0.111111, 0.444444, 0.444444, 0.222222, 0.111111,
		0.027778, 0.000000, 0.000000, 0.000000, 0.055556, 0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000,  
		0.111111, 0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000, 0.055556, 0.000000, 0.000000, 0.000000, 0.027778, 0.000000, 0.000000, 0.000000,
		0.027778, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,  
		0.111111, 0.111111, 0.055556, 0.027778, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
		0.027778, 0.055556, 0.111111, 0.111111, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 
		0.000000, 0.000000, 0.000000, 0.027778, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,  
		0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000, 0.055556, 0.000000, 0.000000, 0.000000, 0.027778,  
		0.000000, 0.000000, 0.000000, 0.027778, 0.000000, 0.000000, 0.000000, 0.055556, 0.000000, 0.000000, 0.000000, 0.111111, 0.000000, 0.000000, 0.000000, 0.111111, 
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.027778, 
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.027778, 0.055556, 0.111111, 0.111111,
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.111111, 0.111111, 0.055556, 0.027778, 
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.027778, 0.000000, 0.000000, 0.000000, 
	};

	GLuint ProgramName(0);
	GLuint ProgramName2(0);
	GLuint ArrayBufferName(0);
	GLuint VertexArrayName(0);
	GLuint PatchIndicesBufferName(0);
	GLuint FaceIndicesBufferName(0);
	GLint UniformMVP(0);
	GLint UniformMVP2(0);
	GLint UniformView(0);
	GLint UniformWeights(0);
}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		ProgramName = glCreateProgram();
		GLuint VertexShader = amd::createShader(GL_VERTEX_SHADER, SAMPLE_VERTEX_SHADER);
		GLuint ControlShader = amd::createShader(GL_TESS_CONTROL_SHADER, SAMPLE_CONTROL_SHADER);
		GLuint EvaluationShader = amd::createShader(GL_TESS_EVALUATION_SHADER, SAMPLE_EVALUATION_SHADER);
		GLuint FragmentShader = amd::createShader(GL_FRAGMENT_SHADER, SAMPLE_FRAGMENT_SHADER);

		glAttachShader(ProgramName, VertexShader);
		glAttachShader(ProgramName, ControlShader);
		glAttachShader(ProgramName, EvaluationShader);
		glAttachShader(ProgramName, FragmentShader);
		glDeleteShader(VertexShader);
		glDeleteShader(ControlShader);
		glDeleteShader(EvaluationShader);
		glDeleteShader(FragmentShader);
		glLinkProgram(ProgramName);
		Validated = amd::checkProgram(ProgramName);
	}
	if(Validated)
	{
	    UniformMVP = glGetUniformLocation(ProgramName, "matMVP");
		UniformView = glGetUniformLocation(ProgramName, "matView");
		UniformWeights = glGetUniformLocation(ProgramName, "weights");
	}
	if(Validated)
	{
		ProgramName2 = glCreateProgram();
		GLuint DrawInputMesh_VS = amd::createShader(GL_VERTEX_SHADER, SAMPLE_VERTEX_SHADER2);
		GLuint DrawInputMesh_FS = amd::createShader(GL_FRAGMENT_SHADER, SAMPLE_FRAGMENT_SHADER2);
		glAttachShader(ProgramName2, DrawInputMesh_VS);
		glAttachShader(ProgramName2, DrawInputMesh_FS);
		glDeleteShader(DrawInputMesh_VS);
		glDeleteShader(DrawInputMesh_FS);
		glLinkProgram(ProgramName2);
		Validated = amd::checkProgram(ProgramName2);
	}
	if(Validated)
	{
	    UniformMVP2 = glGetUniformLocation(ProgramName2, "matMVP");
	}

	for (int i=0; i<16; i++)
	{
	    LineIndices[8*i] = FaceIndices[4*i]; LineIndices[8*i+1] = FaceIndices[4*i+1];
		LineIndices[8*i+2] = FaceIndices[4*i+1]; LineIndices[8*i+3] = FaceIndices[4*i+2];
		LineIndices[8*i+4] = FaceIndices[4*i+2]; LineIndices[8*i+5] = FaceIndices[4*i+3];
		LineIndices[8*i+6] = FaceIndices[4*i+3]; LineIndices[8*i+7] = FaceIndices[4*i];
	}

	return Validated && amd::checkError("initProgram");
}

bool initVertexArray()
{
	// Build a vertex array object
	glGenVertexArrays(1, &VertexArrayName);
    glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), AMD_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	return amd::checkError("initVertexArray");
}

bool initBuffer()
{
	glGenBuffers(1, &ArrayBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &PatchIndicesBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PatchIndicesBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, PatchIndicesSize, PatchIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &FaceIndicesBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FaceIndicesBufferName);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, FaceIndicesSize*2, LineIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return amd::checkError("initBuffer");
}

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&amd::debugOutput, NULL);

	return Validated;
}

bool begin()
{
	bool Validated = amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && amd::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

    // Set initial rendering states
	glEnable(GL_DEPTH_TEST);


	return Validated && amd::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteBuffers(1, &ArrayBufferName);
	glDeleteProgram(ProgramName);

	return amd::checkError("end");
}

void display()
{
	glViewport(0, 0, Window.Size.x, Window.Size.y);
	//clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (amd::isWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(VertexArrayName);


	// Update the transformation matrix
	glm::mat4 Projection = glm::perspective(40.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y-8));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y + 45.0f, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x + 45.0f, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f); // object sapce and world space are the same here
	glm::mat4 matMVP = Projection * View * Model; 
	glm::mat4 matView = View * Model;

	glUseProgram(ProgramName);
	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &matMVP[0][0]);
	glUniformMatrix4fv(UniformView, 1, GL_FALSE, &matView[0][0]);
	glUniform1fv(UniformWeights, 256, &weights[0]);

	
	glPatchParameteri(GL_PATCH_VERTICES, 16);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PatchIndicesBufferName);
	glDrawElements(GL_PATCHES, PatchIndicesCount, GL_UNSIGNED_INT, 0);
	
	if (amd::drawInputMesh)
	{
		glUseProgram(ProgramName2);
		glUniformMatrix4fv(UniformMVP2, 1, GL_FALSE, &matMVP[0][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FaceIndicesBufferName);
		glDrawElements(GL_LINES, FaceIndicesCount*2, GL_UNSIGNED_INT, 0);
	}
	
	amd::checkError("display");
	amd::swapBuffers();
}

int main(int argc, char* argv[])
{

	int result = amd::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, ::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);

	return result;
}

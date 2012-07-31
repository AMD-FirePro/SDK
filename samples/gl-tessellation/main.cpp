

#include <amd/amd.hpp>


namespace
{
	extern bool isWireFrame;

	std::string const SAMPLE_NAME = "OpenGL Tessellation";	
	std::string const SAMPLE_VERTEX_SHADER(amd::DATA_DIRECTORY + "tess.vert");
	std::string const SAMPLE_CONTROL_SHADER(amd::DATA_DIRECTORY + "tess.cont");
	std::string const SAMPLE_EVALUATION_SHADER(amd::DATA_DIRECTORY + "tess.eval");
	std::string const SAMPLE_FRAGMENT_SHADER(amd::DATA_DIRECTORY + "tess.frag");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const patchCount=16;
	GLsizei const VertexCount(patchCount*4);
	GLsizeiptr const VertexSize = VertexCount * sizeof(glm::vec3) * 4;
	
	const float VertexData[VertexCount*3*4] =
	{0.888889, 0.833333, 0.888889,  0.666667, 0.333333, 0.666667, 0.666667, -0.333333, 0.666667, 0.888889, -0.833333, 0.888889,
     0.444444, 0.833333, 1.333333,  0.333333, 0.333333, 1.000000, 0.333333, -0.333333, 1.000000, 0.444444, -0.833333, 1.333333, 
	-0.444444, 0.833333, 1.333333, -0.333333, 0.333333, 1.000000, -0.333333, -0.333333, 1.000000, -0.444444, -0.833333, 1.333333,
	-0.888889, 0.833333, 0.888889, -0.666667, 0.333333, 0.666667, -0.666667, -0.333333, 0.666667, -0.888889, -0.833333, 0.888889,
	 
	0.888889, 0.833333, -0.888889, 0.666667, 0.333333, -0.666667, 0.666667, -0.333333, -0.666667, 0.888889, -0.833333, -0.888889,
	1.333333, 0.833333, -0.444444, 1.000000, 0.333333, -0.333333, 1.000000, -0.333333, -0.333333, 1.333333, -0.833333, -0.444444, 
	1.333333, 0.833333, 0.444444,  1.000000, 0.333333, 0.333333,  1.000000, -0.333333, 0.333333,  1.333333, -0.833333, 0.444444,
	0.888889, 0.833333, 0.888889, 0.666667, 0.333333, 0.666667, 0.666667, -0.333333, 0.666667, 0.888889, -0.833333, 0.888889,

	-0.888889, 0.833333, -0.888889 , -1.333333, 0.833333,-0.444444 ,-1.333333, 0.833333, 0.444444 ,-0.888889, 0.833333, 0.888889,
	-0.666667, 0.333333, -0.666667 ,-1.000000, 0.333333, -0.333333 ,-1.000000, 0.333333, 0.333333 ,-0.666667, 0.333333, 0.666667 ,
	-0.666667, -0.333333, -0.666667 ,-1.000000, -0.333333, -0.333333 ,-1.000000, -0.333333, 0.333333, -0.666667, -0.333333, 0.666667 ,
	-0.888889, -0.833333, -0.888889, -1.333333, -0.833333, -0.444444 ,-1.333333, -0.833333, 0.444444,-0.888889, -0.833333, 0.888889 ,
        
    -0.888889, 0.833333, -0.888889 ,-0.666667, 0.333333, -0.666667 ,-0.666667, -0.333333, -0.666667 ,-0.888889, -0.833333, -0.888889 ,
	-0.444444, 0.833333, -1.333333, -0.333333, 0.333333, -1.000000 ,-0.333333, -0.333333, -1.000000, -0.444444, -0.833333, -1.333333 ,
	0.444444, 0.833333, -1.333333, 0.333333, 0.333333, -1.000000 ,0.333333, -0.333333, -1.000000, 0.444444, -0.833333, -1.333333 ,
	0.888889, 0.833333, -0.888889, 0.666667, 0.333333, -0.666667 ,0.666667, -0.333333, -0.666667, 0.888889, -0.833333, -0.888889,

	1.777778, 1.166667, 1.777778  ,  0.888889, 1.166667, 2.666667 , -0.888889, 1.166667, 2.666667 , -1.777778, 1.166667, 1.777778 , 
    2.000000, 0.666667, 2.000000  ,  1.000000, 0.666667, 3.000000  , -1.000000, 0.666667, 3.000000  ,  -2.000000, 0.666667, 2.000000  , 
    2.000000, -0.666667, 2.000000  ,  1.000000, -0.666667, 3.000000  , -1.000000, -0.666667, 3.000000  ,  -2.000000, -0.666667, 2.000000 , 
    1.777778, -1.166667, 1.777778  ,  0.888889, -1.166667, 2.666667 , -0.888889, -1.166667, 2.666667  ,  -1.777778, -1.166667, 1.777778,

    1.777778, 1.166667, -1.777778  ,  2.666667, 1.166667, -0.888889,  2.666667, 1.166667, 0.888889  ,  1.777778, 1.166667, 1.777778 , 
    2.000000, 0.666667, -2.000000  ,  3.000000, 0.666667, -1.000000 , 3.000000, 0.666667, 1.000000  ,  2.000000, 0.666667, 2.000000  , 
    2.000000, -0.666667, -2.000000  ,  3.000000, -0.666667, -1.000000 , 3.000000, -0.666667, 1.000000  ,  2.000000, -0.666667, 2.000000 , 
    1.777778, -1.166667, -1.777778  ,  2.666667, -1.166667, -0.888889,  2.666667, -1.166667, 0.888889  ,  1.777778, -1.166667, 1.777778,
          
	-1.777778, 1.166667, -1.777778  ,  -0.888889, 1.166667,-2.666667  ,  0.888889, 1.166667, -2.666667  ,  1.777778, 1.166667,-1.777778  ,  
	-2.000000, 0.666667, -2.000000  ,  -1.000000, 0.666667, -3.000000  ,  1.000000, 0.666667, -3.000000  ,  2.000000, 0.666667, -2.000000,  
	-2.000000, -0.666667, -2.000000  , -1.000000, -0.666667, -3.000000  ,  1.000000, -0.666667, -3.000000 , 2.000000, -0.666667, -2.000000  ,  
	-1.777778, -1.166667, -1.777778,  -0.888889, -1.166667, -2.666667  ,  0.888889, -1.166667,-2.666667  ,  1.777778, -1.166667, -1.777778 ,  
          
	-1.777778, 1.166667, -1.777778  ,  -2.000000, 0.666667,-2.000000  ,  -2.000000, -0.666667, -2.000000  ,  -1.777778,-1.166667, -1.777778  ,  
	-2.666667, 1.166667, -0.888889  , -3.000000, 0.666667, -1.000000  ,  -3.000000, -0.666667, -1.000000 , -2.666667, -1.166667, -0.888889  ,  
	-2.666667, 1.166667, 0.888889,  -3.000000, 0.666667, 1.000000  ,  -3.000000, -0.666667, 1.000000,  -2.666667, -1.166667, 0.888889  ,  
	-1.777778, 1.166667, 1.777778,  -2.000000, 0.666667, 2.000000  ,  -2.000000, -0.666667, 2.000000,  -1.777778, -1.166667, 1.777778 , 

	0.888889, 0.833333, 0.888889 , 0.444444, 0.833333, 1.333333 , -0.444444, 0.833333, 1.333333 , -0.888889, 0.833333, 0.888889 ,
    1.111111, 1.333333, 1.111111 , 0.555556, 1.333333, 1.666667 , -0.555556, 1.333333, 1.666667 , -1.111111, 1.333333, 1.111111 ,
    1.555556, 1.666667, 1.555556 , 0.777778, 1.666667, 2.333333 , -0.777778, 1.666667, 2.333333 , -1.555556, 1.666667, 1.555556 ,
    1.777778, 1.166667, 1.777778 , 0.888889, 1.166667, 2.666667 , -0.888889, 1.166667, 2.666667 , -1.777778, 1.166667, 1.777778 ,

    0.888889, 0.833333, -0.888889 , 1.333333, 0.833333, -0.444444, 1.333333, 0.833333, 0.444444 , 0.888889, 0.833333, 0.888889 ,
    1.111111, 1.333333, -1.111111 , 1.666667, 1.333333, -0.555556 , 1.666667, 1.333333, 0.555556 , 1.111111, 1.333333, 1.111111 ,
    1.555556, 1.666667, -1.555556 , 2.333333, 1.666667, -0.777778 , 2.333333, 1.666667, 0.777778 , 1.555556, 1.666667, 1.555556 ,
    1.777778, 1.166667, -1.777778 , 2.666667, 1.166667, -0.888889 , 2.666667, 1.166667, 0.888889 , 1.777778, 1.166667, 1.777778 , 
           
	-0.888889, 0.833333, -0.888889 , -0.444444, 0.833333, -1.333333 , 0.444444, 0.833333, -1.333333 , 0.888889, 0.833333, -0.888889 , 
	-1.111111, 1.333333, -1.111111 , -0.555556, 1.333333, -1.666667 , 0.555556, 1.333333, -1.666667 , 1.111111, 1.333333, -1.111111 , 
	-1.555556, 1.666667, -1.555556 , -0.777778, 1.666667, -2.333333 , 0.777778, 1.666667, -2.333333 , 1.555556, 1.666667, -1.555556 , 
	-1.777778, 1.166667, -1.777778 , -0.888889, 1.166667, -2.666667 , 0.888889, 1.166667, -2.666667 , 1.777778, 1.166667, -1.777778 , 
           
	-0.888889, 0.833333, -0.888889 , -1.111111, 1.333333, -1.111111 , -1.555556, 1.666667, -1.555556 , -1.777778, 1.166667, -1.777778 , 
	-1.333333, 0.833333, -0.444444 , -1.666667, 1.333333, -0.555556 , -2.333333, 1.666667, -0.777778 , -2.666667, 1.166667, -0.888889 , 
	-1.333333, 0.833333, 0.444444 , -1.666667, 1.333333, 0.555556 , -2.333333, 1.666667, 0.777778 , -2.666667, 1.166667, 0.888889 , 
	-0.888889, 0.833333, 0.888889 , -1.111111, 1.333333, 1.111111 , -1.555556, 1.666667, 1.555556 , -1.777778, 1.166667, 1.777778 , 

     0.888889, -0.833333, 0.888889 , 1.111111, -1.333333, 1.111111, 1.555556, -1.666667, 1.555556 , 1.777778, -1.166667, 1.777778, 
	 0.444444, -0.833333, 1.333333 , 0.555556, -1.333333, 1.666667, 0.777778, -1.666667, 2.333333 , 0.888889, -1.166667, 2.666667, 
	 -0.444444, -0.833333, 1.333333 , -0.555556, -1.333333, 1.666667 , -0.777778, -1.666667, 2.333333 , -0.888889, -1.166667, 2.666667 , 
	 -0.888889, -0.833333, 0.888889 , -1.111111, -1.333333, 1.111111 , -1.555556, -1.666667, 1.555556 , -1.777778, -1.166667, 1.777778 , 
         
	 0.888889, -0.833333, -0.888889 , 1.111111, -1.333333, -1.111111 , 1.555556, -1.666667, -1.555556 , 1.777778, -1.166667, -1.777778 , 
	 1.333333, -0.833333, -0.444444 , 1.666667, -1.333333, -0.555556 , 2.333333, -1.666667, -0.777778 , 2.666667, -1.166667, -0.888889 , 
	 1.333333, -0.833333, 0.444444 , 1.666667, -1.333333, 0.555556 , 2.333333, -1.666667, 0.777778 , 2.666667, -1.166667, 0.888889 , 
	 0.888889, -0.833333, 0.888889 , 1.111111, -1.333333, 1.111111 , 1.555556, -1.666667, 1.555556 , 1.777778, -1.166667, 1.777778 ,
           
	 -0.888889, -0.833333, -0.888889 , -1.111111, -1.333333, -1.111111 , -1.555556, -1.666667, -1.555556 , -1.777778, -1.166667, -1.777778 , 
	 -0.444444, -0.833333, -1.333333 , -0.555556, -1.333333, -1.666667 , -0.777778, -1.666667, -2.333333, -0.888889, -1.166667, -2.666667 , 
	 0.444444, -0.833333, -1.333333 , 0.555556, -1.333333, -1.666667 , 0.777778, -1.666667, -2.333333 , 0.888889, -1.166667, -2.666667 ,
     0.888889, -0.833333, -0.888889 , 1.111111, -1.333333, -1.111111 , 1.555556, -1.666667, -1.555556 , 1.777778, -1.166667, -1.777778,

     -0.888889, -0.833333, -0.888889 , -1.333333, -0.833333, -0.444444 , -1.333333, -0.833333, 0.444444 , -0.888889, -0.833333, 0.888889 , 
	 -1.111111, -1.333333, -1.111111 , -1.666667, -1.333333, -0.555556 , -1.666667, -1.333333, 0.555556 , -1.111111, -1.333333, 1.111111 , 
	 -1.555556, -1.666667, -1.555556, -2.333333, -1.666667, -0.777778 , -2.333333, -1.666667, 0.777778 , -1.555556, -1.666667, 1.555556 , 
	 -1.777778, -1.166667, -1.777778 , -2.666667, -1.166667, -0.888889 , -2.666667, -1.166667, 0.888889 , -1.777778, -1.166667, 1.777778 ,
	
	};


	GLuint ProgramName(0);
	GLuint ArrayBufferName(0);
	GLuint VertexArrayName(0);
	GLint UniformMVP(0);
	GLint UniformView(0);
}//namespace

bool initProgram()
{
	bool Validated = true;

	if(Validated)
	{
		GLuint VertexShader = amd::createShader(GL_VERTEX_SHADER, SAMPLE_VERTEX_SHADER);
		GLuint ControlShader = amd::createShader(GL_TESS_CONTROL_SHADER, SAMPLE_CONTROL_SHADER);
		GLuint EvaluationShader = amd::createShader(GL_TESS_EVALUATION_SHADER, SAMPLE_EVALUATION_SHADER);
		GLuint FragmentShader = amd::createShader(GL_FRAGMENT_SHADER, SAMPLE_FRAGMENT_SHADER);

		ProgramName = glCreateProgram();
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
	}

	return Validated && amd::checkError("initProgram");
}

bool initVertexArray()
{
	// Build a vertex array object
	glGenVertexArrays(1, &VertexArrayName);
    glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*sizeof(glm::vec3), AMD_BUFFER_OFFSET(0));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 4*sizeof(glm::vec3), AMD_BUFFER_OFFSET(sizeof(glm::vec3)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 4*sizeof(glm::vec3), AMD_BUFFER_OFFSET(2*sizeof(glm::vec3)));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 4*sizeof(glm::vec3), AMD_BUFFER_OFFSET(3*sizeof(glm::vec3)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	glBindVertexArray(0);

	return amd::checkError("initVertexArray");
}

bool initBuffer()
{
	glGenBuffers(1, &ArrayBufferName);
    glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

	// Update the transformation matrix
	glm::mat4 Projection = glm::perspective(40.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y-4));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y + 45.0f, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x + 45.0f, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f); // object sapce and world space are the same here
	glm::mat4 matMVP = Projection * View * Model; 
	glm::mat4 matView = View * Model;

	glUseProgram(ProgramName);
	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &matMVP[0][0]);
	glUniformMatrix4fv(UniformView, 1, GL_FALSE, &matView[0][0]);

	glViewport(0, 0, Window.Size.x, Window.Size.y);
	//clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (amd::isWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(VertexArrayName);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &glm::vec2(16.f)[0]);
	//glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &glm::vec4(16.f)[0]);
	glDrawArraysInstanced(GL_PATCHES, 0, VertexCount, 1);

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

#include <amd/amd.hpp>

namespace
{
	std::string const SAMPLE_NAME("OpenGL Empty Window");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

}//namespace

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
	bool Validated(true);
	Validated = Validated && amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	if(Validated && amd::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();

	return Validated;
}

bool end()
{
	bool Validated(true);

	return Validated;
}

void display()
{
	//clear the color buffer
	glm::vec4 clearColor = glm::vec4(221.0f, 194.0f, 165.0f, 255.0f) / 255.0f;
	glViewportIndexedf(0, 0, 0, GLfloat(Window.Size.x), GLfloat(Window.Size.y));
	glClearBufferfv(GL_COLOR, 0, &clearColor[0]);

	amd::swapBuffers();
}

int main(int argc, char* argv[])
{
	return amd::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}


#include <amd/amd.hpp>

#include "ModelAndTextureLoader.hpp"


namespace MySemantic
{
	namespace attr
	{
		//each vertices is defined by:
		enum type
		{
			IN_POSITION   = 0,
			IN_NORMAL     = 1,
			IN_TEXCOORD   = 2,
			IN_TANGENT    = 3,
			IN_BITANGENT  = 4,
		};
	}
	namespace uniform
	{
		//list of buffer uniforms
		enum type
		{
			UNIFORM_UPDATE_EACH_FRAME = 0,//struct that contains all uniform updated at each frame
			UNIFORM_UPDATE_RESIZE = 1,//struct that contains all uniform updated when window is resized
		};
	}
}



namespace
{
	std::string const SAMPLE_NAME("OpenGL - Classic Lighting");
	std::string const VERT_SHADER_SOURCE(amd::DATA_DIRECTORY + "lighting.vert");
	std::string const FRAG_SHADER_SOURCE(amd::DATA_DIRECTORY + "lighting.frag");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	

	namespace program
	{
		enum type
		{
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			MAX
		};
	}//namespace program

	namespace buffer
	{
		enum type
		{
			UNIFORM_UPDATE_EACH_FRAME,

			MAX
		};
	}//namespace buffer



	//This struct must be the same that in the shader(s)
	//be cafefull with padding if using other dimensions that mat4 and vec4
	struct UPDATE_EACH_FRAME
	{
		glm::mat4 mMtoS;//matrix Model space to Screen space ( = matrix Model View Projection )
		glm::vec4 lightDirectionNormalizedWS;//vector in World Space
		glm::vec4 viewDirectionNormalizedWS;//vector in World Space
		glm::mat4 mWtoM_transposed;//matrix World space to Model Space Transposed
		glm::vec4 lightDiffuseColor;
		glm::vec4 lightAmbientColor;
		
	} ;


	GLuint PipelineName(0);
	GLuint ProgramName[program::MAX] = {0};

	GLuint VertexArrayModel(0);

	GLuint BufferName[buffer::MAX] = {0};

	float rotateModel = 0.0f;

	ModelAndTextureLoader* fullModel;

}//namespace

bool initProgram()
{
	bool Validated(true);
	
	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		GLuint VertShaderName = amd::createShader(GL_VERTEX_SHADER, VERT_SHADER_SOURCE);
		GLuint FragShaderName = amd::createShader(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE);

		ProgramName[program::VERTEX_SHADER] = glCreateProgram();
		glProgramParameteri(ProgramName[program::VERTEX_SHADER], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::VERTEX_SHADER], VertShaderName);
		glLinkProgram(ProgramName[program::VERTEX_SHADER]);
		glDeleteShader(VertShaderName);
		Validated = Validated && amd::checkProgram(ProgramName[program::VERTEX_SHADER]);

		ProgramName[program::FRAGMENT_SHADER] = glCreateProgram();
		glProgramParameteri(ProgramName[program::FRAGMENT_SHADER], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::FRAGMENT_SHADER], FragShaderName);
		glLinkProgram(ProgramName[program::FRAGMENT_SHADER]);
		glDeleteShader(FragShaderName);
		Validated = Validated && amd::checkProgram(ProgramName[program::FRAGMENT_SHADER]);
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERTEX_SHADER]);
		glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAGMENT_SHADER]);
	}

	return Validated;
}


bool initModels()
{
	bool Validated(true);

	fullModel = new ModelAndTextureLoader((amd::SHARED_DATA_DIRECTORY+"dragon2_LQ\\").c_str(),(amd::SHARED_DATA_DIRECTORY +  "dragon2_LQ\\dragon2.obj"  ).c_str()); 

	return Validated;
}

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&amd::debugOutput, NULL);

	return Validated;
}


bool initBuffer()
{

	glGenBuffers(buffer::MAX, BufferName);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::UNIFORM_UPDATE_EACH_FRAME]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(UPDATE_EACH_FRAME), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	
  
	return true;
}

bool begin()
{
	bool Validated(true);
	Validated = Validated && amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	//move origin camera
	Window.TranlationCurrent.y = 60;
	Window.TranlationOrigin.y = Window.TranlationCurrent.y;
	Window.RotationCurrent.y = 10;
	Window.RotationOrigin.y = Window.RotationCurrent.y;

	if(Validated && amd::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initModels();

	return Validated;
}

bool end()
{
	bool Validated(true);

	if ( fullModel ) { delete fullModel; fullModel = NULL; }

	glDeleteProgramPipelines(1, &PipelineName);
	glDeleteProgram(ProgramName[program::FRAGMENT_SHADER]);
	glDeleteProgram(ProgramName[program::VERTEX_SHADER]);
	glDeleteBuffers(buffer::MAX, BufferName);
	glDeleteVertexArrays(1, &VertexArrayModel);

	return Validated;
}



void recursiveMeshRendering(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh)
{
	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

		//apply material
		const struct aiMaterial* material = sc->mMaterials[mesh->mMaterialIndex];

		ModelAndTextureLoader::MATERIAL_TEXTUREID textureIDs = fullModel->GetGLtextureOfMaterial(mesh->mMaterialIndex);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIDs.idDiffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureIDs.idNormal);		
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureIDs.idSpecular);		

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



void display()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	
	// Update of the uniform buffer
	{

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::UNIFORM_UPDATE_EACH_FRAME]);
		UPDATE_EACH_FRAME* uniformBufferUpdatedEachFrame = (UPDATE_EACH_FRAME*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(UPDATE_EACH_FRAME),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		float yScale = 1.0f / tanf(    (3.14f / 4.0f)   /2.0f);
		float xScale = yScale / ((float)Window.Size.x / (float)Window.Size.y);
		float zNear = 1.0f;
		float zFar = 500.0f;

		//glm::mat4 Projection = glm::perspectiveFov(45.f, (float)Window.Size.x, (float)Window.Size.y, zNear, zFar);

		glm::mat4 Projection;
		Projection[0] = glm::vec4(xScale,      0.0f,       0.0f,                     0.0f);
		Projection[1] = glm::vec4(0.0f,        yScale,     0.0f,                     0.0f);
		Projection[2] = glm::vec4(0.0f,        0.0f,       zFar/(zNear-zFar),        -1.0f);
		Projection[3] = glm::vec4(0.0f,        0.0f,       -zNear*zFar/(zFar-zNear), 0.0f);
		
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		
		glm::mat4 ModelRotate = glm::rotate(glm::mat4(1.0f), rotateModel, glm::vec3(0.0f,1.0f,0.0f));
		glm::mat4 Model = ModelRotate;
		rotateModel += 0.001f;

		glm::mat4 mWtoM_transposed_4 =  glm::transpose( glm::inverse( Model ) );
		glm::mat3 mWtoM_transposed_3;
		mWtoM_transposed_3[0] = glm::vec3(mWtoM_transposed_4[0].x,mWtoM_transposed_4[0].y,mWtoM_transposed_4[0].z);
		mWtoM_transposed_3[1] = glm::vec3(mWtoM_transposed_4[1].x,mWtoM_transposed_4[1].y,mWtoM_transposed_4[1].z);
		mWtoM_transposed_3[2] = glm::vec3(mWtoM_transposed_4[2].x,mWtoM_transposed_4[2].y,mWtoM_transposed_4[2].z);

		glm::vec4 lookAtVector = - glm::normalize( glm::vec4(View[0][2], View[1][2], View[2][2], 0.0f) );
		glm::vec4 lightDirection = glm::normalize( glm::vec4(0.3f, -0.1f, -1.0f, 0.0f) );

		uniformBufferUpdatedEachFrame->mMtoS = Projection * View * Model;
		uniformBufferUpdatedEachFrame->lightDirectionNormalizedWS = lightDirection;
		uniformBufferUpdatedEachFrame->viewDirectionNormalizedWS = lookAtVector;
		uniformBufferUpdatedEachFrame->mWtoM_transposed = mWtoM_transposed_4;
		uniformBufferUpdatedEachFrame->lightDiffuseColor = glm::vec4(1.0f,1.0f,1.0f,1.0f);
		uniformBufferUpdatedEachFrame->lightAmbientColor = glm::vec4(0.6f,0.6f,0.6f,1.0f);

		// Make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	glViewportIndexedf(0, 0, 0, GLfloat(Window.Size.x), GLfloat(Window.Size.y));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.7f, 0.8f, 1.0f, 1.0f)[0]);

	const GLfloat clearDepthValue = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &clearDepthValue);
	const GLint clearStencilValue = 0;
	glClearBufferiv(GL_STENCIL, 0, &clearStencilValue);

	// Bind rendering objects
	glBindProgramPipeline(PipelineName);
	glBindVertexArray(VertexArrayModel);
	glBindBufferBase(GL_UNIFORM_BUFFER, MySemantic::uniform::UNIFORM_UPDATE_EACH_FRAME, BufferName[buffer::UNIFORM_UPDATE_EACH_FRAME]);

	//render Mesh
	unsigned int iMesh = 0;
	recursiveMeshRendering(fullModel->GetAssimpScene(),fullModel->GetAssimpScene()->mRootNode,&iMesh);

	amd::swapBuffers();
}

int main(int argc, char* argv[])
{
	
	return amd::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 8,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
		
	//return amd::run();
}

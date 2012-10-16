#include "ModelAndTextureLoader_V2.hpp"

namespace MySemantic
{
  namespace uniform
  {
    //list of buffer uniforms
    enum type
    {

      UNIFORM_UPDATE_EACH_FRAME = 0,//struct that contains all uniform updated at each frame
      UNIFORM_UPDATE_RESIZE = 1,//struct that contains all uniform updated when window is resized
      UNIFORM_MATERIAL = 2,
    };
  }
}


namespace
{
  std::string const SAMPLE_NAME("OpenGL - Base Vertex");
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
      VERTEX,
      FRAGMENT__PresenceAll,
      FRAGMENT__Nothing,
      FRAGMENT__DiffuseTexture_TextCoord,
      MAX
    };
  }//namespace program

  namespace buffer
  {
    enum type
    {
      UNIFORM_UPDATE_EACH_FRAME,
      UNIFORM_MATERIAL,

      MAX
    };
  }//namespace buffer



  //These struct must be the same that in the shader(s)
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
  struct MATERIAL
  {
    glm::vec4 diffuse;
    glm::vec4 ambiant;
  };


  MATERIAL CurrentMat;
  GLuint CurrentFragProgram=-1;
  GLuint CurrentDiffuseTex=-1;
  GLuint CurrentDiffuseNormal=-1;
  GLuint CurrentDiffuseSpecular=-1;

  GLuint PipelineName(0);
  GLuint ProgramName[program::MAX] = {0};

  GLuint BufferName[buffer::MAX] = {0};


  float rotateModel = 0.0f;

	ModelAndTextureLoader_V2* fullModel;
}//namespace

bool initProgram()
{
	bool Validated(true);

	amd::compiler Compiler;

	glGenProgramPipelines(1, &PipelineName);

	if(Validated)
	{
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, VERT_SHADER_SOURCE);
		Validated = Validated && Compiler.check();
		assert(Validated);

		ProgramName[program::VERTEX] = glCreateProgram();
		glProgramParameteri(ProgramName[program::VERTEX], GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(ProgramName[program::VERTEX], VertShaderName);
		glLinkProgram(ProgramName[program::VERTEX]);
		glDeleteShader(VertShaderName);
		Validated = Validated && amd::checkProgram(ProgramName[program::VERTEX]);

		for(int i=1; i<program::MAX; i++)
		{
			ProgramName[i] = glCreateProgram();
			glProgramParameteri(ProgramName[i], GL_PROGRAM_SEPARABLE, GL_TRUE);

			std::string Arguments;
			if(i == program::FRAGMENT__PresenceAll)
				Arguments = "--version 420 --profile core -DPRESENCE_NORMALTEXTURE -DPRESENCE_DIFFUSETEXTURE -DPRESENCE_TEXTCOORD -DPRESENCE_SPECULARTEXTURE -DPRESENCE_TBN";
			else if ( i == program::FRAGMENT__Nothing )
				Arguments = "--version 420 --profile core";
			else if(i == program::FRAGMENT__DiffuseTexture_TextCoord)
				Arguments = "--version 420 --profile core -DPRESENCE_DIFFUSETEXTURE -DPRESENCE_TEXTCOORD";
			else 
				Arguments = "--version 420 --profile core";

			GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, FRAG_SHADER_SOURCE, Arguments);
			Validated = Validated && Compiler.check();

			glAttachShader(ProgramName[i], FragShaderName);
			glLinkProgram(ProgramName[i]);
			glDeleteShader(FragShaderName);
			Validated = Validated && amd::checkProgram(ProgramName[i]);

			assert(Validated);
		}
	}

	if(Validated)
	{
		glUseProgramStages(PipelineName, GL_VERTEX_SHADER_BIT, ProgramName[program::VERTEX]);
		glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, ProgramName[program::FRAGMENT__PresenceAll]);
	}

	return Validated;
}

bool initModels()
{
	bool Validated(true);

	fullModel = new ModelAndTextureLoader_V2((amd::DATA_DIRECTORY+"dragon2\\").c_str(),(amd::DATA_DIRECTORY +  "dragon2\\dragon2.obj"  ).c_str()); 

	CurrentMat.ambiant=glm::vec4(-1.0);
	CurrentMat.diffuse=glm::vec4(-1.0);
	return Validated;
}

bool initDebugOutput()
{
#ifdef _DEBUG
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&amd::debugOutput, NULL);

	return Validated;
#else
	return true;
#endif
}


bool initBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::UNIFORM_UPDATE_EACH_FRAME]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UPDATE_EACH_FRAME), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::UNIFORM_MATERIAL]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(MATERIAL), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	

	return true;
}

void BindFragmentProgram(GLuint FragProgram)
{
	if (CurrentFragProgram!=FragProgram)
	{
		glUseProgramStages(PipelineName, GL_FRAGMENT_SHADER_BIT, FragProgram);
		CurrentFragProgram=FragProgram;
	}
}

bool begin()
{
	bool Validated(true);
	Validated = Validated && amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	//move origin camera
	Window.TranlationCurrent.y = 30;
	Window.TranlationOrigin.y = Window.TranlationCurrent.y;
	Window.RotationCurrent.y = 10;
	Window.RotationOrigin.y = Window.RotationCurrent.y;
	Window.RotationCurrent.x = 40;
	Window.RotationOrigin.x = Window.RotationCurrent.x;

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

	glDeleteProgramPipelines(1, &PipelineName);

	for(int i=0; i<program::MAX; i++)
		glDeleteProgram(ProgramName[i]);

	glDeleteBuffers(buffer::MAX, BufferName);

	if(fullModel) 
	{
		delete fullModel; 
		fullModel = NULL; 
	}

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

    //aiString texPath;
    //material->GetTexture(aiTextureType_DIFFUSE,0,&texPath);

    ModelAndTextureLoader_V2::MATERIAL_TEXTUREID textureIDs = fullModel->GetGLtextureOfMaterial(mesh->mMaterialIndex);

    if(CurrentDiffuseTex!=textureIDs.idDiffuse)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureIDs.idDiffuse);
      CurrentDiffuseTex = textureIDs.idDiffuse;
    }

    if(CurrentDiffuseNormal!=textureIDs.idNormal)
    {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, textureIDs.idNormal);	
      CurrentDiffuseNormal = textureIDs.idNormal;
    }
    
    if(CurrentDiffuseSpecular!=textureIDs.idSpecular)
    {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, textureIDs.idSpecular);	
      CurrentDiffuseSpecular = textureIDs.idSpecular;
    }

    if ( textureIDs.idDiffuse == 0 )
    {
      BindFragmentProgram(ProgramName[program::FRAGMENT__Nothing]);
    }
    else
    {
      if ( textureIDs.idNormal == 0 )
        BindFragmentProgram(ProgramName[program::FRAGMENT__DiffuseTexture_TextCoord]);
      else
        BindFragmentProgram(ProgramName[program::FRAGMENT__PresenceAll]);
    }
  

    C_STRUCT aiColor4D diffuse;
    aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);

    C_STRUCT aiColor4D ambiant;
    aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambiant);

    if(CurrentMat.ambiant!=glm::vec4(ambiant.r,ambiant.g,ambiant.b,ambiant.a) &&
       CurrentMat.diffuse!=glm::vec4(diffuse.r,diffuse.g,diffuse.b,diffuse.a))
    {

      glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::UNIFORM_MATERIAL]);
      MATERIAL* uniformBufferMaterial = (MATERIAL*)glMapBufferRange(
        GL_UNIFORM_BUFFER, 0, sizeof(MATERIAL),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

      uniformBufferMaterial->diffuse = glm::vec4(diffuse.r,diffuse.g,diffuse.b,diffuse.a);
      uniformBufferMaterial->ambiant = glm::vec4(ambiant.r,ambiant.g,ambiant.b,ambiant.a);

      // Make sure the uniform buffer is uploaded
      glUnmapBuffer(GL_UNIFORM_BUFFER);
    }

    //render mesh
    glDrawElementsBaseVertex(GL_TRIANGLES, fullModel->GetNb3PointsFaceAdvMesh(*currentMesh)*3, GL_UNSIGNED_INT, (void*)(fullModel->GetIndicesMesh(*currentMesh)*sizeof(unsigned int)), fullModel->GetBaseVertexMesh(*currentMesh));
    (*currentMesh)++;

  }


  // draw all children
  int ChildrenToProcess = 0;
  if (nd->mNumChildren)
    ChildrenToProcess=2;
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

    float aspectRatio;
    if ( Window.Size.x > Window.Size.y ) { aspectRatio = (float)Window.Size.x / (float)Window.Size.y; }
    else { aspectRatio = (float)Window.Size.y / (float)Window.Size.x; }

    float xScale = yScale / aspectRatio;
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
    //rotateModel += 0.001f;

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
    uniformBufferUpdatedEachFrame->lightAmbientColor = glm::vec4(0.4f,0.4f,0.4f,1.0f);

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
  glBindBufferBase(GL_UNIFORM_BUFFER, MySemantic::uniform::UNIFORM_UPDATE_EACH_FRAME, BufferName[buffer::UNIFORM_UPDATE_EACH_FRAME]);
  glBindBufferBase(GL_UNIFORM_BUFFER, MySemantic::uniform::UNIFORM_MATERIAL, BufferName[buffer::UNIFORM_MATERIAL]);


  unsigned int iMesh = 0;
  glBindVertexArray(fullModel->GetMeshVertexArray());
  recursiveMeshRendering(fullModel->GetAssimpScene(),fullModel->GetAssimpScene()->mRootNode,&iMesh);
  glBindVertexArray(0);

  amd::swapBuffers();
}

int main(int argc, char* argv[])
{
	return amd::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT),
		8, 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		::SAMPLE_MAJOR_VERSION, ::SAMPLE_MINOR_VERSION);
}

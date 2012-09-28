#include <amd/amd.hpp>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "ModelAndTextureLoader.hpp"

#define NUM_LIGHT_MAX 8
#define EYE_SEPARATION 0.02f
#define ZDIST 0.6f

using namespace std;

namespace MySemantic
{
	namespace attr
	{
		enum type
		{
			POSITION = 0,
			NORMAL	 = 1,
			TEXCOORD = 2
		};
	}

	namespace uniform
	{
		enum type
		{
			MATERIAL  = 3,
			LIGHT     = 4,
			TRANSFORM = 5
		};
	}
}//MySemantic

namespace
{
	string const SAMPLE_NAME("OpenGL Stereo Quad-buufer GS");
	string const VERT_SHADER_GEN_SOURCE(amd::DATA_DIRECTORY + "StereoGen_vertex.glsl");
	string const GEOM_SHADER_GEN_SOURCE(amd::DATA_DIRECTORY + "StereoGen_geom.glsl");
	string const FRAG_SHADER_GEN_SOURCE(amd::DATA_DIRECTORY + "StereoGEn_frag.glsl");
	string const VERT_SHADER_RES_SOURCE(amd::DATA_DIRECTORY + "StereoResolve_vertex.glsl");  
	string const FRAG_SHADER_RES_SOURCE(amd::DATA_DIRECTORY + "StereoResolve_frag.glsl");

	int gWidth(640);
	int gHeight(480);
	int const SAMPLE_MAJOR_VERSION(4);
	int const SAMPLE_MINOR_VERSION(2);

	amd::window Window(glm::ivec2(gWidth, gHeight));

	GLuint ProgramResName(0);
	GLuint ProgramGenName(0);
	GLuint CurrentProgramName(0);

	GLuint stereoBuffer(0);
	GLuint stereoColorTexARRAY(0);
	GLuint stereoDepthTexARRAY(0);

	GLuint VertexArrayName(0);
	GLuint VertexArrayDrawQuadName(0);

	int StereoTextLoc(-1);

	namespace buffer
	{
		enum type
		{
			POSITION,  // will be used a draw quad screen
			//NORMAL,
			ELEMENT,
			MATERIAL,
			LIGHT,
			TRANSFORM,
			MAX
		};
	}//namespace buffer

	GLuint BufferName[buffer::MAX];

	//need to match the shader definition
	struct Material
	{
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float shininess;
	};

	struct Light
	{
		glm::vec4 position;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		glm::vec3 attenuation;
	};

	ModelAndTextureLoader* fullModel;
}//namespace

bool initDebugOutput()
{
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&amd::debugOutput, NULL);

	return true;
}

void SwicthProgram(GLuint ProgramToBind)
{
	if(CurrentProgramName!=ProgramToBind)
	{
		CurrentProgramName = ProgramToBind;
		glUseProgram(CurrentProgramName);
	}
}

bool CompileAndLink(GLuint &ProgramName, string VS, string GS, string FS)
{
	bool Success(true);

	GLuint VertShaderName = amd::createShader(GL_VERTEX_SHADER, VS);
	GLuint GeomShaderName(0);
	if(!GS.empty())
		GeomShaderName = amd::createShader(GL_GEOMETRY_SHADER, GS);
	GLuint FragShaderName = amd::createShader(GL_FRAGMENT_SHADER, FS);

	ProgramName = glCreateProgram();

	glAttachShader(ProgramName, VertShaderName);
	if(GeomShaderName)
		glAttachShader(ProgramName, GeomShaderName);
	glAttachShader(ProgramName, FragShaderName);

	glDeleteShader(VertShaderName);
	if(GeomShaderName)
	glDeleteShader(GeomShaderName);
	glDeleteShader(FragShaderName);

	glLinkProgram(ProgramName);

	Success = amd::checkProgram(ProgramName);

	return Success; 
}

bool initProgram()
{
	bool Success(true);

	Success = CompileAndLink(ProgramGenName, VERT_SHADER_GEN_SOURCE, GEOM_SHADER_GEN_SOURCE, FRAG_SHADER_GEN_SOURCE);

	if(Success)
		Success=CompileAndLink(ProgramResName, VERT_SHADER_RES_SOURCE, "", FRAG_SHADER_RES_SOURCE);

	StereoTextLoc = glGetUniformLocation(ProgramResName, "arrayTex");
	//we will always be using tex unit 0
	SwicthProgram(ProgramResName);
	glUniform1i(StereoTextLoc, 0);

	return Success;
}

bool initFBOandTextures()
{
	bool Success(true);

	glGenFramebuffers(1,  &stereoBuffer);
	glGenTextures(1, &stereoColorTexARRAY);
	glGenTextures(1, &stereoDepthTexARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, stereoColorTexARRAY);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, Window.Size.x, Window.Size.y,2, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D_ARRAY, stereoDepthTexARRAY);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, Window.Size.x, Window.Size.y,2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, stereoBuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  stereoColorTexARRAY, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  stereoDepthTexARRAY, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return Success;
}

bool initBuffers()
{
	glGenBuffers(buffer::MAX, BufferName);

	float PositionTexCoordData[]=
	{
		-1.0f, -1.0f, 0.0,0.0, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0,0.0, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0,0.0, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0, 0.0, 1.0f, 1.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*6*sizeof(float), PositionTexCoordData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned short ElementData[]={0,1,2,3};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(unsigned short), ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::MATERIAL]);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), NULL, GL_DYNAMIC_DRAW);  //potentially we will update them often, so DYNAMIC_DRAW
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);	

	float lightParameters[19]= {1.0f, 3.0f,-5.0f, 1.0f,  //position
								0.5f, 0.5f, 0.5f, 1.0f,   //ambient
								0.5f, 0.5f, 0.0f, 1.0f,   //diffuse
								0.1f, 0.6f, 0.0f, 1.0f,   //specular
								0.1f, 0.1f, 0.1f};

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::LIGHT]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light)*NUM_LIGHT_MAX, lightParameters, GL_DYNAMIC_DRAW); //potentially we will update them often, so DYNAMIC_DRAW
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*4, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	

	return true;
}

bool initModels()
{
	bool Validated(true);

	fullModel = new ModelAndTextureLoader(amd::DATA_DIRECTORY.c_str(), (amd::DATA_DIRECTORY +  "duck.dae"  ).c_str()); 

	return Validated;
}

bool begin()
{
	bool Success(true);

	// Validate OpenGL support
	Success = Success && amd::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);
	if(Success && amd::checkExtension("GL_ARB_debug_output"))
		Success = initDebugOutput();
	if(Success)
		Success = initBuffers();
	if(Success)
		Success = initModels();
	if(Success)
		Success = initProgram();
	if(Success)
		Success = initFBOandTextures();

	return Success && amd::checkError("Init");
}

bool end()
{
	glDeleteProgram(ProgramGenName);
	glDeleteProgram(ProgramResName);

	return true;
}

void SetUpMatrices()
{
	float tmp = fullModel->GetSize();
	tmp = 5.0f / tmp;

	glm::mat4 MV_P_System[4];

	glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
	glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 4*sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	// scale the whole asset to fit into our view frustum 
	glm::mat4 Scale =  glm::scale(glm::mat4(1.0f), glm::vec3(tmp));  
	glm::mat4 Model =  glm::translate(Scale, -fullModel->GetCenter());
	
	//zoom
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));

	//translate right view
	glm::mat4 ViewTranslateR = glm::translate(ViewTranslate, glm::vec3(-EYE_SEPARATION*(0.5+Window.TranlationCurrent.y), 0.0f, 0.0f ));

	//translate left view
	glm::mat4 ViewTranslateL = glm::translate(ViewTranslate, glm::vec3(EYE_SEPARATION*(0.5+Window.TranlationCurrent.y), 0.0f, 0.0f ));

	//rotation
	glm::mat4 ViewRotateXR = glm::rotate(ViewTranslateR, Window.RotationCurrent.y + 45.0f, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 ViewR = glm::rotate(ViewRotateXR, Window.RotationCurrent.x + 45.0f, glm::vec3(0.f, 1.f, 0.f));

	glm::mat4 ViewRotateXL = glm::rotate(ViewTranslateL, Window.RotationCurrent.y + 45.0f, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 ViewL = glm::rotate(ViewRotateXL, Window.RotationCurrent.x + 45.0f, glm::vec3(0.f, 1.f, 0.f));

	MV_P_System[0] = ViewL * Model;
	MV_P_System[1] = ViewR * Model;

	//classic camera projection
	//http://paulbourke.net/miscellaneous/stereographics/stereorender/
	float Near = 0.1f;
	float ratio  = (float)gWidth / (float)gHeight;
	float radians = glm::pi<float>() * 0.125f;
	float wd2     = Near * tan(radians);
	float ndfl    = Near / ZDIST;
	float top    =   wd2;
	float bottom = - wd2;

	float left  = - ratio * wd2 + 0.5f * EYE_SEPARATION * ndfl;
	float right =   ratio * wd2 + 0.5f * EYE_SEPARATION * ndfl;
	MV_P_System[2] = glm::frustum(left,right,bottom,top,Near,1000.0f);

	left  = - ratio * wd2 - 0.5f * EYE_SEPARATION * ndfl;
	right =   ratio * wd2 - 0.5f * EYE_SEPARATION * ndfl;
	MV_P_System[3] = glm::frustum(left,right,bottom,top,Near,1000.0f); 

	for (int i=0;i<4;i++)
		Pointer[i] = MV_P_System[i];

	glUnmapBuffer(GL_UNIFORM_BUFFER);
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
	//clear back buffer
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(VertexArrayName);

	//draw
	glBindFramebuffer(GL_FRAMEBUFFER, stereoBuffer);
	//clear our FBO
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwicthProgram(ProgramGenName);
	SetUpMatrices();

	unsigned int iMesh = 0;
	recursiveMeshRendering(fullModel->GetAssimpScene(),fullModel->GetAssimpScene()->mRootNode,&iMesh);
	//glDrawElements(GL_TRIANGLES, pCurrentScene->children[0]->NbFaces*3, GL_UNSIGNED_SHORT, NULL);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
	SwicthProgram(ProgramResName);

	GLenum buffers[] = {GL_BACK_LEFT,GL_BACK_RIGHT};
	glDrawBuffers(2, buffers);

	glBindVertexArray(VertexArrayDrawQuadName);
	glBindTexture(GL_TEXTURE_2D_ARRAY, stereoColorTexARRAY);  
	glBindBufferBase(GL_UNIFORM_BUFFER, MySemantic::uniform::TRANSFORM, BufferName[buffer::TRANSFORM]);
	glBindBufferBase(GL_UNIFORM_BUFFER, MySemantic::uniform::LIGHT, BufferName[buffer::LIGHT]);

	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);

	// Swap framebuffers
	amd::swapBuffers();
}

int main(int argc, char* argv[])
{
	// it will call freeglut to set a stereo pixel format
	return amd::run(
		argc, argv,
		glm::ivec2(::gWidth, ::gHeight), 8, 
		WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, ::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);
}




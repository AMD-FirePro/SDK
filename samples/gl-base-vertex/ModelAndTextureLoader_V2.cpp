#include "ModelAndTextureLoader_V2.hpp"

namespace gli{
namespace detail
{
	//GL_COMPRESSED_RED, GL_COMPRESSED_RG, GL_COMPRESSED_RGB, GL_COMPRESSED_RGBA, GL_COMPRESSED_SRGB, GL_COMPRESSED_SRGB_ALPHA, 
	//GL_SRGB, GL_SRGB8, GL_SRGB_ALPHA, or GL_SRGB8_ALPHA8
	struct texture_desc
	{
		GLint InternalFormat;
		GLint InternalFormatCompressed;
		GLint InternalFormatSRGB;
		GLint InternalFormatCompressedSRGB;
		GLenum ExternalFormat;
		GLenum ExternalFormatRev;
		GLenum Type;
	};

	//GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
	//GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, 
	//GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, 
	//GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, 
	//GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, 
	//GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, 
	//GL_UNSIGNED_INT_2_10_10_10_REV

#	ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
#	define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB 0x8E8C
#	endif

#	ifndef GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB
#	define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB 0x8E8D
#	endif

#	ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
#	define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB 0x8E8E
#	endif

#	ifndef GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
#	define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB 0x8E8F
#	endif

	inline texture_desc gli2ogl_cast(format const & Format)
	{
		texture_desc Cast[] = 
		{
			{GL_NONE, GL_NONE, GL_NONE,	GL_NONE, GL_NONE, GL_NONE, GL_NONE},

			//// Normalized
			//{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_BYTE},
			//{GL_RG,		GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_BYTE},
			//{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_BYTE},
			//{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_BYTE},

			//{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_SHORT},
			//{GL_RG,		GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_SHORT},
			//{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_SHORT},
			//{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_SHORT},

			//{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_INT},
			//{GL_RG,		GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_INT},
			//{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_INT},
			//{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_INT},

			// Unsigned
			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_BYTE},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_BYTE},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_BYTE},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_BYTE},

			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_SHORT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_SHORT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_SHORT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_SHORT},

			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_UNSIGNED_INT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_UNSIGNED_INT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_UNSIGNED_INT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_UNSIGNED_INT},

			// Signed
			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_BYTE},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_BYTE},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_BYTE},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_BYTE},

			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_SHORT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_SHORT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_SHORT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_SHORT},

			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_INT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_INT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_INT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_INT},

			// Float
			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_HALF_FLOAT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_HALF_FLOAT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_HALF_FLOAT},

			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_FLOAT},
			{GL_RG,			GL_COMPRESSED_RG,		GL_RG,				GL_COMPRESSED_RG,				GL_RG,			GL_RG,		GL_FLOAT},
			{GL_RGB,		GL_COMPRESSED_RGB,		GL_SRGB8,			GL_COMPRESSED_SRGB,				GL_RGB,			GL_BGR,		GL_FLOAT},
			{GL_RGBA,		GL_COMPRESSED_RGBA,		GL_SRGB8_ALPHA8,	GL_COMPRESSED_SRGB_ALPHA,		GL_RGBA,		GL_BGRA,	GL_FLOAT},

			// Packed
			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_RGB9_E5,	GL_RGB9_E5,				GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_R11F_G11F_B10F,	GL_R11F_G11F_B10F,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_RED,		GL_COMPRESSED_RED,		GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_RGBA4,		GL_RGBA4,				GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},
			{GL_RGB10_A2,	GL_RGB10_A2,			GL_RED,				GL_COMPRESSED_RED,				GL_RED,			GL_RED,		GL_HALF_FLOAT},

			// Depth
			{GL_DEPTH_COMPONENT16,	GL_DEPTH_COMPONENT16,	GL_DEPTH_COMPONENT16,	GL_DEPTH_COMPONENT16,	GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,		GL_UNSIGNED_SHORT},
			{GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT24,	GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,		GL_UNSIGNED_INT},
			{GL_DEPTH24_STENCIL8,	GL_DEPTH24_STENCIL8,	GL_DEPTH24_STENCIL8,	GL_DEPTH24_STENCIL8,	GL_DEPTH_COMPONENT,		GL_DEPTH_STENCIL,		GL_UNSIGNED_INT},
			{GL_DEPTH_COMPONENT32F,	GL_DEPTH_COMPONENT32F,	GL_DEPTH_COMPONENT32F,	GL_DEPTH_COMPONENT32F,	GL_DEPTH_COMPONENT,		GL_DEPTH_COMPONENT,		GL_FLOAT},
			{GL_DEPTH32F_STENCIL8,	GL_DEPTH32F_STENCIL8,	GL_DEPTH32F_STENCIL8,	GL_DEPTH32F_STENCIL8,	GL_DEPTH_COMPONENT,		GL_DEPTH_STENCIL,		GL_UNSIGNED_INT},

			// Compressed formats
			{GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,			GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,			GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,			GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RED_RGTC1,					GL_COMPRESSED_RED_RGTC1,					GL_COMPRESSED_RED_RGTC1,					GL_COMPRESSED_RED_RGTC1,					GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_SIGNED_RED_RGTC1,			GL_COMPRESSED_SIGNED_RED_RGTC1,				GL_COMPRESSED_SIGNED_RED_RGTC1,				GL_COMPRESSED_SIGNED_RED_RGTC1,				GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RG_RGTC2,					GL_COMPRESSED_RG_RGTC2,						GL_COMPRESSED_RG_RGTC2,						GL_COMPRESSED_RG_RGTC2,						GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_SIGNED_RG_RGTC2,				GL_COMPRESSED_SIGNED_RG_RGTC2,				GL_COMPRESSED_SIGNED_RG_RGTC2,				GL_COMPRESSED_SIGNED_RG_RGTC2,				GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,	GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,	GL_NONE, GL_NONE, GL_NONE},
			{GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,			GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,			GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,	GL_NONE, GL_NONE, GL_NONE},
		};

		return Cast[Format];
	}

}//namespace detail

	inline GLuint createTexture2D(std::string const & Filename)
	{
		gli::texture2D Texture = gli::load(Filename);

		assert(!Texture.empty());

		detail::texture_desc TextureDesc = detail::gli2ogl_cast(Texture.format());

		GLint Alignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &Alignment);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLuint Name = 0;
		glGenTextures(1, &Name);
		glBindTexture(GL_TEXTURE_2D, Name);
		glTexStorage2D(GL_TEXTURE_2D, GLint(Texture.levels()), TextureDesc.InternalFormat, GLsizei(Texture[0].dimensions().x), GLsizei(Texture[0].dimensions().y));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture.levels() > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if(gli::bits_per_pixel(Texture.format()) == gli::block_size(Texture.format()) << 3)
		{
			for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
			{
				glTexSubImage2D(
					GL_TEXTURE_2D, 
					GLint(Level), 
					0, 0, 
					GLsizei(Texture[Level].dimensions().x), 
					GLsizei(Texture[Level].dimensions().y), 
					TextureDesc.ExternalFormatRev, 
					TextureDesc.Type, 
					Texture[Level].data());
			}
		}
		else
		{
			for(gli::texture2D::size_type Level = 0; Level < Texture.levels(); ++Level)
			{
				GLsizei LevelSize = GLsizei((Texture[Level].dimensions().x * Texture[Level].dimensions().y * gli::bits_per_pixel(Texture.format())) >> 3);
				glm::uvec2 Dimensions = glm::uvec2(Texture[Level].dimensions());

				glCompressedTexSubImage2D(
					GL_TEXTURE_2D,
					GLint(Level),
					0, 0,
					GLsizei(Dimensions.x), 
					GLsizei(Dimensions.y), 
					TextureDesc.InternalFormat, 
					LevelSize, 
					Texture[Level].data());
			}
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);

		return Name;
	}

}//namespace gli


/////////////////////////////////////////////////////////////////////////////////////////////
void ModelAndTextureLoader_V2::get_bounding_box_for_node
(
	aiNode const * Node, 
	aiVector3D* Min, 
	aiVector3D* Max, 
	aiMatrix4x4* Trafo
)
{
	aiMatrix4x4 prev = *Trafo;
	unsigned int n = 0, t;

	aiMultiplyMatrix4(Trafo, &Node->mTransformation);

	for(; n < Node->mNumMeshes; ++n) 
	{
		const struct aiMesh* mesh = m_assimpScene->mMeshes[Node->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) 
		{
			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, Trafo);

			Min->x = aisgl_min(Min->x,tmp.x);
			Min->y = aisgl_min(Min->y,tmp.y);
			Min->z = aisgl_min(Min->z,tmp.z);

			Max->x = aisgl_max(Max->x,tmp.x);
			Max->y = aisgl_max(Max->y,tmp.y);
			Max->z = aisgl_max(Max->z,tmp.z);
		}
	}

	for(n = 0; n < Node->mNumChildren; ++n)
		get_bounding_box_for_node(Node->mChildren[n], Min, Max, Trafo);

	*Trafo = prev;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ModelAndTextureLoader_V2::get_bounding_box ( aiVector3D* min,  aiVector3D* max)
{
  aiMatrix4x4 trafo;
  aiIdentityMatrix4(&trafo);

  min->x = min->y = min->z =  1e10f;
  max->x = max->y = max->z = -1e10f;
  get_bounding_box_for_node(m_assimpScene->mRootNode,min,max,&trafo);
}

ModelAndTextureLoader_V2::ModelAndTextureLoader_V2
(
	const char* TextureDirectory,
	const char* fullPathToModel
) : 
	glBuffer_VertexBufferPositionSize(0),
	glBuffer_VertexBufferNormalSize(0),
	glBuffer_VertexBufferTextureCoorSize(0),
	glBuffer_VertexBufferTangentSize(0),
	glBuffer_VertexBufferBitangentSize(0),
	m_AdvmeshStruct(NULL),
	m_nbTotalMesh(0)
{
  //Before Assimp 3.0 :
  //force to recompute normal tangent and bitangent
  //Assimp::Importer::SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
  //aiSetImportPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
  //aiSetImportPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_TANGENTS_AND_BITANGENTS);
  //m_assimpScene = aiImportFile(fullPathToModel,  aiProcessPreset_TargetRealtime_MaxQuality
  //	 | aiProcess_RemoveComponent | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals //force to recompute normal tangent and bitangent
  //	 );

  m_myImporter = new Assimp::Importer();

  //force to recompute normal tangent and bitangent
  m_myImporter->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
  m_myImporter->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_TANGENTS_AND_BITANGENTS);

  m_assimpScene = m_myImporter->ReadFile(fullPathToModel,aiProcessPreset_TargetRealtime_MaxQuality
    | aiProcess_RemoveComponent | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals //force to recompute normal tangent and bitangent
    );

  if ( m_assimpScene )
  {
    aiVector3D scene_min, scene_max, scene_center;
    get_bounding_box(&scene_min,&scene_max);
    m_vCenter.x = (scene_min.x + scene_max.x) / 2.0f;
    m_vCenter.y = (scene_min.y + scene_max.y) / 2.0f;
    m_vCenter.z = (scene_min.z + scene_max.z) / 2.0f;

    m_fSize = scene_max.x-scene_min.x;
    m_fSize = aisgl_max(scene_max.y - scene_min.y,m_fSize);
    m_fSize = aisgl_max(scene_max.z - scene_min.z,m_fSize);

    RecursiveMesh_CountTotalMesh(m_assimpScene,m_assimpScene->mRootNode);

    m_AdvmeshStruct = new AdvanceMesh[m_nbTotalMesh];

    unsigned int iMesh = 0;
    unsigned int BufferSizeInBytes = 0;
    unsigned int BaseVertex = 0;
    unsigned int VOffset = 0;
    unsigned int NOffset = 0;
    unsigned int TexOffSet = 0;
    unsigned int TgOffset = 0;
    unsigned int BiTgOffset = 0;

    get_buffer_size(BufferSizeInBytes,m_assimpScene,m_assimpScene->mRootNode,&iMesh, BaseVertex );
    CreateBuffers(BufferSizeInBytes);
    iMesh = 0;
    FillVBO(m_assimpScene,m_assimpScene->mRootNode,&iMesh, VOffset, NOffset, TexOffSet, TgOffset, BiTgOffset);
    SetVertexFormat();

    iMesh = 0;
    //RecursiveMesh_Loading(m_assimpScene,m_assimpScene->mRootNode,&iMesh);

    //load textures
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    m_textureOfEachMaterial = new MATERIAL_TEXTUREID[m_assimpScene->mNumMaterials];

    for (unsigned int m=0; m<m_assimpScene->mNumMaterials; m++)
    {
      int texIndex;

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_AMBIENT, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_EMISSIVE, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_NORMALS, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }

      texIndex = 0;
      while (true)
      {
        aiString path;	
        aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_HEIGHT, texIndex, &path);
        if ( texFound ==  AI_SUCCESS ) { m_textureIdMap[path.data] = NULL;  texIndex++; }
        else{break;}
      }
    }

	std::map<std::string, GLuint*>::iterator itr = m_textureIdMap.begin();

	m_textureIds.resize(m_textureIdMap.size());

	for(std::size_t i = 0; i < m_textureIds.size(); ++i)
	{
		std::string filename = (*itr).first;
		(*itr).second = &m_textureIds[i];

		m_textureIds[i] = gli::createTexture2D(std::string(TextureDirectory) + filename);

		itr++;
	}

    for (unsigned int m=0; m<m_assimpScene->mNumMaterials; m++)
    {

      aiString path;
      aiReturn texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
      if ( texFound ==  AI_SUCCESS ) 
      { 
        m_textureOfEachMaterial[m].idDiffuse = *m_textureIdMap[path.data] ;  
      }
      else{ m_textureOfEachMaterial[m].idDiffuse = 0;}


      texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_SPECULAR, 0, &path);
      if ( texFound ==  AI_SUCCESS ) 
      { 
        m_textureOfEachMaterial[m].idSpecular = *m_textureIdMap[path.data] ;  
      }
      else{ m_textureOfEachMaterial[m].idSpecular = 0;}


      texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_AMBIENT, 0, &path);
      if ( texFound ==  AI_SUCCESS ) 
      { 
        m_textureOfEachMaterial[m].idAmbiant = *m_textureIdMap[path.data] ;  
      }
      else{ m_textureOfEachMaterial[m].idAmbiant = 0;}


      texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_EMISSIVE, 0, &path);
      if ( texFound ==  AI_SUCCESS ) 
      { 
        m_textureOfEachMaterial[m].idEmissive = *m_textureIdMap[path.data] ;  
      }
      else{ m_textureOfEachMaterial[m].idEmissive = 0;}


      texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_NORMALS, 0, &path);
      if ( texFound ==  AI_SUCCESS ) 
      { 
        m_textureOfEachMaterial[m].idNormal = *m_textureIdMap[path.data] ;  
      }
      else
      { 
        //height could be considered as normals
        texFound = m_assimpScene->mMaterials[m]->GetTexture(aiTextureType_HEIGHT, 0, &path);
        if ( texFound ==  AI_SUCCESS ) 
        { 
          m_textureOfEachMaterial[m].idNormal = *m_textureIdMap[path.data] ;  
        }
        else
        { 
          m_textureOfEachMaterial[m].idNormal = 0;
        }
      }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }	
}

ModelAndTextureLoader_V2::~ModelAndTextureLoader_V2()
{
	glDeleteTextures(GLsizei(m_textureIdMap.size()), &m_textureIds[0]);

	if(m_textureOfEachMaterial)
	{
		delete[] m_textureOfEachMaterial;
		m_textureOfEachMaterial = NULL;
	}

	m_textureIdMap.clear();

	if(m_assimpScene)
		DeleteBuffers();

	if(m_myImporter)
	{
		delete m_myImporter; 
		m_myImporter = NULL; 
	}

	if(m_AdvmeshStruct)
	{
		delete [] m_AdvmeshStruct;
		m_AdvmeshStruct = NULL;
	}
}

void ModelAndTextureLoader_V2::RecursiveMesh_CountTotalMesh(const struct aiScene *sc, const struct aiNode* nd)
{
  m_nbTotalMesh += nd->mNumMeshes;

  for (unsigned int n = 0; n < nd->mNumChildren; ++n)
  {
    RecursiveMesh_CountTotalMesh(sc, nd->mChildren[n]);
  }
}

void ModelAndTextureLoader_V2::get_buffer_size(unsigned int& size, const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh, unsigned int& BaseVertex)
{
  unsigned int& OffsetPernode = BaseVertex;
  for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
  {
    const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];
    size+=mesh->mNumVertices*3*sizeof(float);
    glBuffer_VertexBufferPositionSize+=mesh->mNumVertices*3*sizeof(float);
    if(mesh->HasNormals()) 
    {
      size+=mesh->mNumVertices*3*sizeof(float);
      glBuffer_VertexBufferNormalSize+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mTextureCoords[0])
    {
      size+=mesh->mNumVertices*3*sizeof(float); //need to use 3 component : see assimp-3.0.1270-sdk\include\assimp\mesh.h
      glBuffer_VertexBufferTextureCoorSize+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mTangents)
    {
      size+=mesh->mNumVertices*3*sizeof(float); //need to use 3 component : see assimp-3.0.1270-sdk\include\assimp\mesh.h
      glBuffer_VertexBufferTangentSize+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mBitangents)
    {
      size+=mesh->mNumVertices*3*sizeof(float); //need to use 3 component : see assimp-3.0.1270-sdk\include\assimp\mesh.h
      glBuffer_VertexBufferBitangentSize+=mesh->mNumVertices*3*sizeof(float);
    }

    unsigned int _3pointFaceCounter = 0;
    m_AdvmeshStruct[*currentMesh].nb3PointsFaces = 0;
    for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
    {
      const struct aiFace* face = &mesh->mFaces[t];
      if ( face->mNumIndices == 3 ) 
        m_AdvmeshStruct[*currentMesh].nb3PointsFaces ++; 
    } 

    m_AdvmeshStruct[*currentMesh].p3PointsFaces = (unsigned int*)malloc( m_AdvmeshStruct[*currentMesh].nb3PointsFaces * 3 * sizeof(unsigned int) );


    
    for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
    {
      const struct aiFace* face = &mesh->mFaces[t];
      if ( face->mNumIndices == 3 )
      {
        m_AdvmeshStruct[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+0] = face->mIndices[0] ;
        m_AdvmeshStruct[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+1] = face->mIndices[1] ;
        m_AdvmeshStruct[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+2] = face->mIndices[2] ;
        _3pointFaceCounter++;
      }
    } 
    m_AdvmeshStruct[*currentMesh].IndicesOffset = OffsetPernode*3;
    OffsetPernode+=_3pointFaceCounter;
    m_AdvmeshStruct[*currentMesh].ModelMesh =nd->mTransformation;
    (*currentMesh) ++;
  }

  for (unsigned int n = 0; n < nd->mNumChildren; ++n)
  {
    get_buffer_size(size, sc, nd->mChildren[n],currentMesh, OffsetPernode);
  }
}

void ModelAndTextureLoader_V2::CreateBuffers(int size)
{
    glGenBuffers(1,&BufferID);
    glBindBuffer(GL_ARRAY_BUFFER, BufferID);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1,&IndicesBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndicesBufferID);
    
    unsigned int IndicesSize = 0;
    for(unsigned int i = 0; i < m_nbTotalMesh; ++i)
      IndicesSize+=m_AdvmeshStruct[i].nb3PointsFaces*3*sizeof(unsigned int);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndicesSize, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ModelAndTextureLoader_V2::DeleteBuffers()
{
  glDeleteBuffers(1,&BufferID);
  glDeleteBuffers(1,&IndicesBufferID);
  glDeleteVertexArrays(1,&VAO_ID);
}

void ModelAndTextureLoader_V2::FillVBO(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh, unsigned int& VertexOffset, 
                                       unsigned int& NormalOffset, unsigned int& TexCoordOffset, unsigned int& TangentOffset, unsigned int& BiTangentOffset)
{

  glBindBuffer(GL_ARRAY_BUFFER, BufferID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndicesBufferID);
  for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
  {
    const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];
    if(mesh->HasPositions())
    {
      glBufferSubData(GL_ARRAY_BUFFER, VertexOffset, mesh->mNumVertices*3*sizeof(float), mesh->mVertices);
      m_AdvmeshStruct[*currentMesh].MeshOffset=VertexOffset/(3*sizeof(float));

      VertexOffset+=mesh->mNumVertices*3*sizeof(float);

    }

    if(mesh->HasNormals())
    {
      glBufferSubData(GL_ARRAY_BUFFER, glBuffer_VertexBufferPositionSize+NormalOffset, mesh->mNumVertices*3*sizeof(float), mesh->mNormals);
      NormalOffset+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mTextureCoords[0])
    {
      glBufferSubData(GL_ARRAY_BUFFER, glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize+TexCoordOffset, mesh->mNumVertices*3*sizeof(float), mesh->mTextureCoords[0]);
      TexCoordOffset+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mTangents)
    {
      glBufferSubData(GL_ARRAY_BUFFER, glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize+glBuffer_VertexBufferTextureCoorSize+TangentOffset, mesh->mNumVertices*3*sizeof(float), mesh->mTangents);
      TangentOffset+=mesh->mNumVertices*3*sizeof(float);
    }
    if(mesh->mBitangents)
    {
      glBufferSubData(GL_ARRAY_BUFFER, glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize+glBuffer_VertexBufferTextureCoorSize+glBuffer_VertexBufferTangentSize+BiTangentOffset, mesh->mNumVertices*3*sizeof(float), mesh->mBitangents);
      BiTangentOffset+=mesh->mNumVertices*3*sizeof(float);
    }
    if(m_AdvmeshStruct[*currentMesh].p3PointsFaces)
    {
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_AdvmeshStruct[*currentMesh].IndicesOffset*sizeof(unsigned int), m_AdvmeshStruct[*currentMesh].nb3PointsFaces*3*sizeof(unsigned int), m_AdvmeshStruct[*currentMesh].p3PointsFaces);
      free(m_AdvmeshStruct[*currentMesh].p3PointsFaces);
      m_AdvmeshStruct[*currentMesh].p3PointsFaces = NULL;
    }
    (*currentMesh)++;

  }
  for (unsigned int n = 0; n < nd->mNumChildren; ++n)
  {
    FillVBO( sc, nd->mChildren[n],currentMesh, VertexOffset, NormalOffset, TexCoordOffset, TangentOffset, BiTangentOffset);
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ModelAndTextureLoader_V2::SetVertexFormat()
{
    glGenVertexArrays(1,&VAO_ID);
    glBindVertexArray(VAO_ID);

        
    glBindBuffer(GL_ARRAY_BUFFER, BufferID );
    glVertexAttribPointer(MESHSEMANTIC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(MESHSEMANTIC_POSITION);
   
    glVertexAttribPointer(MESHSEMANTIC_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)glBuffer_VertexBufferPositionSize);
    glEnableVertexAttribArray(MESHSEMANTIC_NORMAL);

    glVertexAttribPointer(MESHSEMANTIC_TEXCOORD, 3, GL_FLOAT, GL_FALSE, 0, (void*)(glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize));
    glEnableVertexAttribArray(MESHSEMANTIC_TEXCOORD);

    glVertexAttribPointer(MESHSEMANTIC_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)(glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize+glBuffer_VertexBufferTextureCoorSize));
    glEnableVertexAttribArray(MESHSEMANTIC_TANGENT);

    glVertexAttribPointer(MESHSEMANTIC_BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)(glBuffer_VertexBufferPositionSize+glBuffer_VertexBufferNormalSize+glBuffer_VertexBufferTextureCoorSize+glBuffer_VertexBufferTangentSize));
    glEnableVertexAttribArray(MESHSEMANTIC_BITANGENT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndicesBufferID);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0 );
}


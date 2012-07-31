#include "ModelAndTextureLoader.h"
#include <GL/glew.h>
//#include <assimp\config.h>
//#include <assimp\Importer.hpp>
#include "assimp.h"

#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////
void ModelAndTextureLoader::get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo)
{
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void ModelAndTextureLoader::get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(m_assimpScene->mRootNode,min,max,&trafo);
}

ModelAndTextureLoader::ModelAndTextureLoader(const char* TextureDirectory,const char* fullPathToModel)
{
	m_nbTotalMesh = 0;
	m_allMeshes = NULL;

	//force to recompute normal tangent and bitangent
	//Assimp::Importer::SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
	//aiSetImportPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
	//aiSetImportPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_TANGENTS_AND_BITANGENTS);


	m_assimpScene = aiImportFile(fullPathToModel,  aiProcessPreset_TargetRealtime_MaxQuality|aiProcess_PreTransformVertices);
//		 | aiProcess_RemoveComponent | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals //force to recompute normal tangent and bitangent
//		 );


	if ( m_assimpScene )
	{
		struct aiVector3D scene_min, scene_max, scene_center;
		get_bounding_box(&scene_min,&scene_max);
		m_vCenter.x = (scene_min.x + scene_max.x) / 2.0f;
		m_vCenter.y = (scene_min.y + scene_max.y) / 2.0f;
		m_vCenter.z = (scene_min.z + scene_max.z) / 2.0f;

		m_fSize = scene_max.x-scene_min.x;
		m_fSize = aisgl_max(scene_max.y - scene_min.y,m_fSize);
		m_fSize = aisgl_max(scene_max.z - scene_min.z,m_fSize);

		RecursiveMesh_CountTotalMesh(m_assimpScene,m_assimpScene->mRootNode);

		m_allMeshes = (MESH*)malloc(sizeof(MESH) * m_nbTotalMesh);

		unsigned int iMesh = 0;
		RecursiveMesh_Loading(m_assimpScene,m_assimpScene->mRootNode,&iMesh);
	



		//load textures

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

		int numTextures = m_textureIdMap.size();

		std::map<std::string, GLuint*>::iterator itr = m_textureIdMap.begin();

		m_textureIds = new GLuint[numTextures];
		glGenTextures(numTextures, m_textureIds);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (int i=0; i<numTextures; i++)
		{
			std::string filename = (*itr).first;
			(*itr).second = &m_textureIds[i];

			gli::texture2D textureFileLoaded = gli::load(std::string(TextureDirectory) + filename);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gli::texture2D::format_type formatImage = textureFileLoaded.format();
			if ( formatImage == gli::RGB8U )
			{
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,textureFileLoaded[0].dimensions().x,textureFileLoaded[0].dimensions().y,0,GL_RGB,GL_UNSIGNED_BYTE,textureFileLoaded[0].data());
			}
			else if ( formatImage == gli::RGBA8U )
			{
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,textureFileLoaded[0].dimensions().x,textureFileLoaded[0].dimensions().y,0,GL_RGBA,GL_UNSIGNED_BYTE,textureFileLoaded[0].data());
			}
			glBindTexture(GL_TEXTURE_2D, 0);


			itr++;
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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

	}	
}

ModelAndTextureLoader::~ModelAndTextureLoader()
{
	glDeleteTextures(m_textureIdMap.size(),m_textureIds);
	if (m_textureIds) { delete[] m_textureIds; m_textureIds = NULL; }
	if ( m_textureOfEachMaterial ) { delete[] m_textureOfEachMaterial; m_textureOfEachMaterial = NULL; }

	m_textureIdMap.clear();

	if ( m_assimpScene )
	{
		unsigned int iMesh = 0;
		RecursiveMesh_Delete(m_assimpScene,m_assimpScene->mRootNode,&iMesh);
		aiReleaseImport(m_assimpScene);
	}

	if ( m_allMeshes ) { free(m_allMeshes); m_allMeshes = NULL; }
}


void ModelAndTextureLoader::RecursiveMesh_CountTotalMesh(const struct aiScene *sc, const struct aiNode* nd)
{
	m_nbTotalMesh += nd->mNumMeshes;

	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		RecursiveMesh_CountTotalMesh(sc, nd->mChildren[n]);
	}
}

void ModelAndTextureLoader::RecursiveMesh_Loading(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh)
{
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];

		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferPosition));
		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferNormal));
		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferTextureCoor));
		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferTangent));
		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferBitangent));
		glGenBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_IndexBuffer));

		glGenVertexArrays(1,&(m_allMeshes[*currentMesh].gl_vertexArrayModel));


		glBindBuffer(GL_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_VertexBufferPosition);
		glBufferData(GL_ARRAY_BUFFER,mesh->mNumVertices*3*sizeof(float), mesh->mVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_VertexBufferNormal);
		glBufferData(GL_ARRAY_BUFFER,mesh->mNumVertices*3*sizeof(float), mesh->mNormals, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_VertexBufferTextureCoor);
		glBufferData(GL_ARRAY_BUFFER,mesh->mNumVertices*3*sizeof(float), mesh->mTextureCoords[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_VertexBufferTangent);
		glBufferData(GL_ARRAY_BUFFER,mesh->mNumVertices*3*sizeof(float), mesh->mTangents, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_VertexBufferBitangent);
		glBufferData(GL_ARRAY_BUFFER,mesh->mNumVertices*3*sizeof(float), mesh->mBitangents, GL_STATIC_DRAW);

		
		m_allMeshes[*currentMesh].nb3PointsFaces = 0;
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
		{
			const struct aiFace* face = &mesh->mFaces[t];
			if ( face->mNumIndices == 3 ) { m_allMeshes[*currentMesh].nb3PointsFaces ++; }
		} 

		m_allMeshes[*currentMesh].p3PointsFaces = (unsigned int*)malloc( m_allMeshes[*currentMesh].nb3PointsFaces * 3 * sizeof(unsigned int) );

		
		unsigned int _3pointFaceCounter = 0;
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
		{
			const struct aiFace* face = &mesh->mFaces[t];
			if ( face->mNumIndices == 3 )
			{
				m_allMeshes[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+0] = face->mIndices[0] ;
				m_allMeshes[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+1] = face->mIndices[1] ;
				m_allMeshes[*currentMesh].p3PointsFaces[_3pointFaceCounter*3+2] = face->mIndices[2] ;
				_3pointFaceCounter++;
			}
		} 


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_allMeshes[*currentMesh].glBuffer_IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_allMeshes[*currentMesh].nb3PointsFaces*3*sizeof(unsigned int), m_allMeshes[*currentMesh].p3PointsFaces, GL_STATIC_DRAW);

		


		glBindVertexArray(m_allMeshes[*currentMesh].gl_vertexArrayModel);
			glBindBuffer(GL_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_VertexBufferPosition );
				glVertexAttribPointer(MESHSEMANTIC_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(MESHSEMANTIC_POSITION);

			glBindBuffer(GL_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_VertexBufferNormal);
				glVertexAttribPointer(MESHSEMANTIC_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(MESHSEMANTIC_NORMAL);

			glBindBuffer(GL_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_VertexBufferTextureCoor);
				glVertexAttribPointer(MESHSEMANTIC_TEXCOORD, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(MESHSEMANTIC_TEXCOORD);

			glBindBuffer(GL_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_VertexBufferTangent);
				glVertexAttribPointer(MESHSEMANTIC_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(MESHSEMANTIC_TANGENT);

			glBindBuffer(GL_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_VertexBufferBitangent);
				glVertexAttribPointer(MESHSEMANTIC_BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnableVertexAttribArray(MESHSEMANTIC_BITANGENT);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_allMeshes[*currentMesh].glBuffer_IndexBuffer);
		glBindVertexArray(0);



		(*currentMesh) ++;
	}

	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		RecursiveMesh_Loading(sc, nd->mChildren[n],currentMesh);
	}

	return;
}

void ModelAndTextureLoader::RecursiveMesh_Delete(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh)
{
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];

		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferPosition));
		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferNormal));
		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferTextureCoor));
		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferTangent));
		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_VertexBufferBitangent));
		glDeleteBuffers(1,&(m_allMeshes[*currentMesh].glBuffer_IndexBuffer));

		glDeleteVertexArrays(1, &(m_allMeshes[*currentMesh].gl_vertexArrayModel));

		free(m_allMeshes[*currentMesh].p3PointsFaces);
		m_allMeshes[*currentMesh].p3PointsFaces = NULL;

		(*currentMesh) ++;
	}

	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		RecursiveMesh_Delete(sc, nd->mChildren[n],currentMesh);
	}
}




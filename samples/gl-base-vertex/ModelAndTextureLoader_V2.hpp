#pragma once

#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>

#include <assimp.h>
#include <aiPostProcess.h>
#include <aiScene.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)


class ModelAndTextureLoader_V2
{
public:

	ModelAndTextureLoader_V2(const char* TextureDirectory,const char* fullPathToModel);
	~ModelAndTextureLoader_V2();

	struct MATERIAL_TEXTUREID
	{
		unsigned int idDiffuse;
		unsigned int idSpecular;
		unsigned int idAmbiant;
		unsigned int idEmissive;
		unsigned int idNormal;
	};


  unsigned int GetMeshVertexArray() 
  { 
    return VAO_ID; 
  }
	unsigned int GetNb3PointsFaceAdvMesh(unsigned int numMesh) 
  { 
    return m_AdvmeshStruct[numMesh].nb3PointsFaces; 
  }
  unsigned int GetBaseVertexMesh(unsigned int numMesh)
  { 
    return m_AdvmeshStruct[numMesh].MeshOffset;
  }
  unsigned int GetIndicesMesh(unsigned int numMesh)
  { 
    return m_AdvmeshStruct[numMesh].IndicesOffset;
  }

  void DeleteBuffers();

	const struct aiScene* GetAssimpScene() { return m_assimpScene; };
	MATERIAL_TEXTUREID GetGLtextureOfMaterial(unsigned int materialID) { return m_textureOfEachMaterial[materialID]; }

	float GetSize()	{ return m_fSize; }
	const glm::vec3 &GetCenter()	{ return m_vCenter;	}

private:

	enum MESH_SEMANTIC
	{
		MESHSEMANTIC_POSITION   = 0,
		MESHSEMANTIC_NORMAL     = 1,
		MESHSEMANTIC_TEXCOORD   = 2,
		MESHSEMANTIC_TANGENT    = 3,
		MESHSEMANTIC_BITANGENT  = 4,
	};

	void RecursiveMesh_CountTotalMesh(const struct aiScene *sc, const struct aiNode* nd);
	void RecursiveMesh_Loading(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh);

	void get_bounding_box_for_node (const  aiNode* nd,  aiVector3D* min,  aiVector3D* max,  aiMatrix4x4* trafo);
	void get_bounding_box ( aiVector3D* min,  aiVector3D* max);
  void get_buffer_size(unsigned int& size,const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh, unsigned int& BaseVertex);
  void CreateBuffers(int size);
  void FillVBO(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh, unsigned int& VertexOffset, 
               unsigned int& NormalOffset, unsigned int& TexCoordOffset, unsigned int& TangentOffset, unsigned int& BiTangentOffset);

  void SetVertexFormat();
	unsigned int m_nbTotalMesh;


  struct AdvanceMesh
  {
    unsigned int nb3PointsFaces;//number of faces/ with 3 points (sometimes meshes have degenerated triangles with less than 3 points)
    unsigned int MeshOffset; //base vertex
    unsigned int* p3PointsFaces;//temp buffer for Indices. We will delete it as soon as we fill the BO
    unsigned int IndicesOffset;
    aiMatrix4x4 ModelMesh;  //fro the dragon all the meshes have the same matrix, but we need to be careful!!!! 
  };
  AdvanceMesh* m_AdvmeshStruct;

  unsigned int glBuffer_VertexBufferPositionSize;
  unsigned int glBuffer_VertexBufferNormalSize;
  unsigned int glBuffer_VertexBufferTextureCoorSize;
  unsigned int glBuffer_VertexBufferTangentSize;
  unsigned int glBuffer_VertexBufferBitangentSize;

  unsigned int BufferID;
  unsigned int IndicesBufferID;
  unsigned int VAO_ID;

  const struct aiScene* m_assimpScene;

	// images / texture
	std::map<std::string, unsigned int*> m_textureIdMap;	// map image filenames to textureIds
	unsigned int* m_textureIds;							// pointer to texture Array
	MATERIAL_TEXTUREID* m_textureOfEachMaterial;
	Assimp::Importer* m_myImporter;

	float m_fSize;
	glm::vec3 m_vCenter;
};


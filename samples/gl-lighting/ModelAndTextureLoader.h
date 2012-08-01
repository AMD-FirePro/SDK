#pragma once

#include <vector>
#include <map>
#include <string>

//
// assimp include files. These three are usually needed.
//#include "../../../external/Assimp-3.0/include/assimp/assimp.h"
//#include "../../../external/Assimp-3.0/include/assimp/aiPostProcess.h"
//#include "../../../external/Assimp-3.0/include/assimp/aiScene.h"
//#include <GL/gl.h>
//#pragma comment(lib, "../../../external/Assimp-3.0/assimp.lib")

//#include <assimp\

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp\Importer.hpp>



//#include "../../../external/Assimp-3.0/lib/



class ModelAndTextureLoader
{
public:

	ModelAndTextureLoader(const char* TextureDirectory,const char* fullPathToModel);
	~ModelAndTextureLoader();

	struct MATERIAL_TEXTUREID
	{
		unsigned int idDiffuse;
		unsigned int idSpecular;
		unsigned int idAmbiant;
		unsigned int idEmissive;
		unsigned int idNormal;
	};

	unsigned int GetMeshVertexArray(unsigned int numMesh) { return m_allMeshes[numMesh].gl_vertexArrayModel; }
	unsigned int GetNb3PointsFace(unsigned int numMesh) { return m_allMeshes[numMesh].nb3PointsFaces; }
	const struct aiScene* GetAssimpScene() { return m_assimpScene; };
	MATERIAL_TEXTUREID GetGLtextureOfMaterial(unsigned int materialID) { return m_textureOfEachMaterial[materialID]; }



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
	void RecursiveMesh_Delete(const struct aiScene *sc, const struct aiNode* nd, unsigned int* currentMesh);

	unsigned int m_nbTotalMesh;

	struct MESH
	{
		unsigned int glBuffer_VertexBufferPosition;
		unsigned int glBuffer_VertexBufferNormal;
		unsigned int glBuffer_VertexBufferTextureCoor;
		unsigned int glBuffer_VertexBufferTangent;
		unsigned int glBuffer_VertexBufferBitangent;
		unsigned int glBuffer_IndexBuffer;

		unsigned int nb3PointsFaces;//number of 3 points faces 
		unsigned int* p3PointsFaces;//index buffer to 3 points faces

		unsigned int gl_vertexArrayModel;
	};

	MESH* m_allMeshes;

	const struct aiScene* m_assimpScene;

	// images / texture
	std::map<std::string, unsigned int*> m_textureIdMap;	// map image filenames to textureIds
	unsigned int* m_textureIds;							// pointer to texture Array
	MATERIAL_TEXTUREID* m_textureOfEachMaterial;
	Assimp::Importer* m_myImporter;

};


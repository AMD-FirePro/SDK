#pragma once

// assimp include files. These three are usually needed.
#include <assimp.h>
#include <aiPostProcess.h>
#include <aiScene.h>
//#include <GL/gl.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

// the global Assimp scene object
extern const struct aiScene* scene;
//extern GLuint scene_list;
extern struct aiVector3D scene_min, scene_max, scene_center;


typedef struct sceneGeometry
{
	unsigned int VBOID;
	unsigned int VBOIDNormal;
	unsigned int VBOIDtextureCoor;
	unsigned int VBOIDtangent;
	unsigned int VBOIDbitangent;
	unsigned int IBOID;
	struct aiMatrix4x4 Matrix;  
	sceneGeometry** children;
	unsigned int NbFaces;
	unsigned int nbChildren;

	sceneGeometry()
	{
		VBOID = 0;  
		IBOID = 0;
		VBOIDNormal=0;
		VBOIDtextureCoor = 0;
		VBOIDtangent = 0;
		VBOIDbitangent = 0;
		//TexID = 0;
		nbChildren = 0;
		NbFaces = 0;
		children = NULL;
	}

} pSceneGeometry;

static pSceneGeometry* pCurrentScene = NULL;

int loadasset (const char* path);
void recursive_render (const struct aiScene *sc, const struct aiNode* nd);

void recursive_meshGenerator (const struct aiScene *sc, const struct aiNode* nd, pSceneGeometry*& LocalScene);

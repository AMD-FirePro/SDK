#include "ModelLoader2.h"
#include <GL/glew.h>
const struct aiScene* scene = NULL;
//GLuint scene_list = 0;
struct aiVector3D scene_min, scene_max, scene_center;
float alpha = 1.0f;

/////////////////////////////////////////////////////////////////////////////////////////////
void get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo
){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) 
	{
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) 
		{

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

	for (n = 0; n < nd->mNumChildren; ++n) 
	{
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

/////////////////////////////////////////////////////////////////////////////////////////////
int loadasset ( const char* path)
{

	// we are taking one of the postprocessing presets to avoid
	// writing 20 single postprocessing flags here.
	scene = aiImportFile(path,   aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene) 
	{
		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		return 0;
	}
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void color4_to_float4(const struct aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = alpha;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//void apply_material(const struct aiMaterial *mtl)
//{
//	float c[4];
//
//	GLenum fill_mode;
//	int ret1, ret2;
//	struct aiColor4D diffuse;
//	struct aiColor4D specular;
//	struct aiColor4D ambient;
//	struct aiColor4D emission;
//	float shininess, strength;
//	int two_sided;
//	int wireframe;
//	unsigned int max;
//
//	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
//		color4_to_float4(&diffuse, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);
//
//	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
//		color4_to_float4(&specular, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
//
//	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
//		color4_to_float4(&ambient, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);
//
//	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
//		color4_to_float4(&emission, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);
//
//	max = 1;
//	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
//	max = 1;
//	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
//	if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
//		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
//	//else {
//	//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
//	//	set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
//	//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
//	//}
//
//	//max = 1;
//	//if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
//	//	fill_mode = wireframe ? GL_LINE : GL_FILL;
//	//else
//	//	fill_mode = GL_FILL;
//	//glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
//
//	//max = 1;
//	//if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
//	//	glEnable(GL_CULL_FACE);
//	//else 
//	//	glDisable(GL_CULL_FACE);
//}

/////////////////////////////////////////////////////////////////////////////////////////////
// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
//void Color4f(const struct aiColor4D *color)
//{
//	glColor4f(color->r, color->g, color->b, color->a);
//}

/////////////////////////////////////////////////////////////////////////////////////////////
//void recursive_render (const struct aiScene *sc, const struct aiNode* nd)
//{
//	unsigned int n = 0;
//	struct aiMatrix4x4 m = nd->mTransformation;
//
//	// update transform
//	aiTransposeMatrix4(&m);
//	glPushMatrix();
//	glMultMatrixf((float*)&m);
//
//	// draw all meshes assigned to this node
//	for (; n < nd->mNumMeshes; ++n) {
//		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
//
//		apply_material(sc->mMaterials[mesh->mMaterialIndex]);
///*
//		if(mesh->mNormals == NULL) {
//			glDisable(GL_LIGHTING);
//		} else {
//			glEnable(GL_LIGHTING);
//		}
//
//		if(mesh->mColors[0] != NULL) {
//			glEnable(GL_COLOR_MATERIAL);
//		} else {
//			glDisable(GL_COLOR_MATERIAL);
//		}
//*/
//		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
//			const struct aiFace* face = &mesh->mFaces[t];
//			GLenum face_mode;
//
//			switch(face->mNumIndices) {
//				case 1: face_mode = GL_POINTS; break;
//				case 2: face_mode = GL_LINES; break;
//				case 3: face_mode = GL_TRIANGLES; break;
//				default: face_mode = GL_POLYGON; break;
//			}
//
//			glBegin(face_mode);
//
//			for(unsigned int i = 0; i < face->mNumIndices; i++) {
//				int index = face->mIndices[i];
//				if(mesh->mColors[0] != NULL)
//					Color4f(&mesh->mColors[0][index]);
//				if(mesh->mNormals != NULL) 
//					glNormal3fv(&mesh->mNormals[index].x);
//				glVertex3fv(&mesh->mVertices[index].x);
//			}
//
//			glEnd();
//		}
//
//	}
//
//	// draw all children
//	for (n = 0; n < nd->mNumChildren; ++n) {
//		recursive_render(sc, nd->mChildren[n]);
//	}
//
//	glPopMatrix();
//}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void recursive_meshGenerator (const struct aiScene *sc, const struct aiNode* nd, pSceneGeometry*& LocalScene)
{
	LocalScene = new pSceneGeometry;
	int NbVertices = 0;
	int NbFaces = 0;
	struct aiMatrix4x4 m = nd->mTransformation;

	LocalScene->Matrix = m;  //ModelmMatrix

	if(nd->mNumChildren)
	LocalScene->children= new pSceneGeometry*[nd->mNumChildren];
	else
	LocalScene->children =NULL;


	LocalScene->nbChildren=nd->mNumChildren;
	if(nd->mNumMeshes)
	{
		glGenBuffers(1,&(LocalScene->VBOID));
		glGenBuffers(1,&(LocalScene->VBOIDNormal));
		glGenBuffers(1,&(LocalScene->VBOIDtextureCoor));
		glGenBuffers(1,&(LocalScene->VBOIDtangent));
		glGenBuffers(1,&(LocalScene->VBOIDbitangent));
		glGenBuffers(1,&(LocalScene->IBOID));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,LocalScene->IBOID);

		for (unsigned int n = 0; n < nd->mNumMeshes; ++n) 
		{
			const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
			NbVertices+=mesh->mNumVertices;
			NbFaces+=mesh->mNumFaces;
		}

		LocalScene->NbFaces=NbFaces;
		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOID);
		glBufferData(GL_ARRAY_BUFFER,NbVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDNormal);
		glBufferData(GL_ARRAY_BUFFER,NbVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDtextureCoor);
		glBufferData(GL_ARRAY_BUFFER,NbVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDtangent);
		glBufferData(GL_ARRAY_BUFFER,NbVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDbitangent);
		glBufferData(GL_ARRAY_BUFFER,NbVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);


		unsigned short* Indices = new unsigned short[NbFaces*3];

		unsigned int trueNbOfFace = 0;
		unsigned int nbVerticesWhenSkipped = 0;
    
		int nbverticesLocal=0;
		int nbFacesLocal=0;
		for (unsigned int n = 0; n < nd->mNumMeshes; ++n) 
		{
			const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

			glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOID);
			glBufferSubData(GL_ARRAY_BUFFER,nbverticesLocal*3*sizeof(float), 3*sizeof(float)*mesh->mNumVertices, mesh->mVertices);

			glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDNormal);
			glBufferSubData(GL_ARRAY_BUFFER,nbverticesLocal*3*sizeof(float), 3*sizeof(float)*mesh->mNumVertices, mesh->mNormals);

			glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDtextureCoor);
			glBufferSubData(GL_ARRAY_BUFFER,nbverticesLocal*3*sizeof(float), 3*sizeof(float)*mesh->mNumVertices, mesh->mTextureCoords[0]);

			glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDtangent);
			glBufferSubData(GL_ARRAY_BUFFER,nbverticesLocal*3*sizeof(float), 3*sizeof(float)*mesh->mNumVertices, mesh->mTangents);

			glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOIDbitangent);
			glBufferSubData(GL_ARRAY_BUFFER,nbverticesLocal*3*sizeof(float), 3*sizeof(float)*mesh->mNumVertices, mesh->mBitangents);



			nbverticesLocal+=mesh->mNumVertices;
     

			for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
			{
				const struct aiFace* face = &mesh->mFaces[t];

				if ( face->mNumIndices == 3 )
				{
					Indices[trueNbOfFace*3+0] = face->mIndices[0] + nbVerticesWhenSkipped ;
					Indices[trueNbOfFace*3+1] = face->mIndices[1] + nbVerticesWhenSkipped ;
					Indices[trueNbOfFace*3+2] = face->mIndices[2] + nbVerticesWhenSkipped;
					trueNbOfFace++;
				}
				else
				{
					nbVerticesWhenSkipped = mesh->mNumVertices;
				}
			} 

		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(trueNbOfFace)*3*sizeof(unsigned short) , Indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER,LocalScene->VBOID);
		aiVector3D* bufferVertexRichard =  (aiVector3D*)glMapBuffer(GL_ARRAY_BUFFER,GL_READ_WRITE);

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	for (unsigned int n = 0; n < nd->mNumChildren; ++n) 
	{
		recursive_meshGenerator(sc, nd->mChildren[n], LocalScene->children[n]);
	}

}

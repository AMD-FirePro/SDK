#include "ModelAndTextureLoader_V2.hpp"
#include <GL/glew.h>
#include <assimp\config.h>
#include <assimp\vector3.h>
#include <assimp\matrix4x4.h>

#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>

#define        GL_VBO_FREE_MEMORY_ATI                     0x87FB

/////////////////////////////////////////////////////////////////////////////////////////////
void ModelAndTextureLoader_V2::get_bounding_box_for_node (const  aiNode* nd, 
  aiVector3D* min, 
  aiVector3D* max, 
  aiMatrix4x4* trafo)
{
  aiMatrix4x4 prev;
  unsigned int n = 0, t;

  prev = *trafo;
  aiMultiplyMatrix4(trafo,&nd->mTransformation);

  for (; n < nd->mNumMeshes; ++n) 
  {
    const struct aiMesh* mesh = m_assimpScene->mMeshes[nd->mMeshes[n]];
    for (t = 0; t < mesh->mNumVertices; ++t) 
    {

      aiVector3D tmp = mesh->mVertices[t];
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
void ModelAndTextureLoader_V2::get_bounding_box ( aiVector3D* min,  aiVector3D* max)
{
  aiMatrix4x4 trafo;
  aiIdentityMatrix4(&trafo);

  min->x = min->y = min->z =  1e10f;
  max->x = max->y = max->z = -1e10f;
  get_bounding_box_for_node(m_assimpScene->mRootNode,min,max,&trafo);
}

ModelAndTextureLoader_V2::ModelAndTextureLoader_V2(const char* TextureDirectory,const char* fullPathToModel): 
  glBuffer_VertexBufferPositionSize(0),
  glBuffer_VertexBufferNormalSize(0),
  glBuffer_VertexBufferTextureCoorSize(0),
  glBuffer_VertexBufferTangentSize(0),
  glBuffer_VertexBufferBitangentSize(0),
  m_AdvmeshStruct(NULL)
{
  m_nbTotalMesh = 0;
  



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


    for (int i=0; i<numTextures; i++)
    {
      std::string filename = (*itr).first;
      (*itr).second = &m_textureIds[i];

      gli::texture2D textureFileLoaded = gli::load(std::string(TextureDirectory) + filename);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gli::texture2D::format_type formatImage = textureFileLoaded.format();

      GLsizei dimX = textureFileLoaded[0].dimensions().x;
      GLsizei dimY = textureFileLoaded[0].dimensions().y;

      if ( formatImage == gli::RGB8U )
      {
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,dimX,dimY,0,GL_RGB,GL_UNSIGNED_BYTE,textureFileLoaded[0].data());
      }
      else if ( formatImage == gli::RGBA8U )
      {
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,dimX,dimY,0,GL_RGBA,GL_UNSIGNED_BYTE,textureFileLoaded[0].data());
      }
      glBindTexture(GL_TEXTURE_2D, 0);


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
  glDeleteTextures(m_textureIdMap.size(),m_textureIds);
  if (m_textureIds) { delete[] m_textureIds; m_textureIds = NULL; }
  if ( m_textureOfEachMaterial ) { delete[] m_textureOfEachMaterial; m_textureOfEachMaterial = NULL; }

  m_textureIdMap.clear();

  if ( m_assimpScene )
  {
    unsigned int iMesh = 0;
    DeleteBuffers();
  }


  if ( m_myImporter ) { delete m_myImporter; m_myImporter = NULL; }
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
    for(int i=0; i<m_nbTotalMesh; i++)
    {
      IndicesSize+=m_AdvmeshStruct[i].nb3PointsFaces*3*sizeof(unsigned int);
     
    }
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



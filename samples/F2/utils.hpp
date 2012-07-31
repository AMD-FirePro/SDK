#pragma once


#include <windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/half_float.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

// GLI
#define         GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define         GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define         GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define         GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>
#include <gli/gtx/gl_texture2d.hpp>

// STL
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

GLuint CreateTexture2D(const char * filename);
GLuint CreateTextureCube(const char * filename);
bool UpdateProperty(float & current, float goal, float speed, DWORD ms);
bool CheckOglError(bool assert = true);

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define INVALID_OGL_VALUE 0xFFFFFFFF
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; } 
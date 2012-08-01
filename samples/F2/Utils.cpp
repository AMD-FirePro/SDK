#include "glapp.hpp"
#include "utils.hpp"

//////////////////////////////////////////////////////////////////////
GLuint CreateTexture2D(const char * filename)
{
	return gli::createTexture2D(filename);
}

//////////////////////////////////////////////////////////////////////
GLuint CreateTextureCube(const char * filename)
{
	GLuint tex = 0;
	glActiveTexture(GL_TEXTURE0);

	gli::textureCube Texture = gli::loadTextureCubeDDS9(filename);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture.levels() > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

	GLint Alignment = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &Alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// this will get rid of the line artifact on mipmap filtering
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

	gli::gtx::gl_texture2d::detail::texture_desc TextureDesc = gli::gtx::gl_texture2d::detail::gli2ogl_cast(Texture.format());

	for(std::size_t Face = 0; Face < 6; ++Face)
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		glCompressedTexImage2D(
			GLenum(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face),
			GLint(Level),
			TextureDesc.InternalFormat,
			GLsizei(Texture[gli::textureCube::face_type(Face)][Level].dimensions().x), 
			GLsizei(Texture[gli::textureCube::face_type(Face)][Level].dimensions().y), 
			0, 
			GLsizei(Texture[gli::textureCube::face_type(Face)][Level].capacity()), 
			Texture[gli::textureCube::face_type(Face)][Level].data());
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, Alignment);

	return tex;
}

//////////////////////////////////////////////////////////////////////
bool UpdateProperty(float & current, float goal, float speed, DWORD ms)
{
	if (ms == 0)
		return false;

	if (current == goal)
		return false;

	if (current < goal)
	{
		current += speed*(float)ms;
		if (current > goal)
			current = goal;
	}
	else
	{
		current -= speed*(float)ms;
		if (current < goal)
			current = goal;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

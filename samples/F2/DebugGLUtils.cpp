#include <sstream>
#include <fstream>
#include <iostream>

#include "utils.hpp"

//////////////////////////////////////////////////////////////////////
bool CheckOglError(bool assert)
{
	// only assert noe time
	static bool alreadyAssertOnce = false;

	// report a maximum number of error to avoid use log file
	static int maxError = 10;
	if(maxError<=0)
	{
		if(maxError==0)
		{
#ifdef _DEBUG
			OutputDebugString("CheckOglError: too many error, no more check\n");
#endif // _DEBUG
			--maxError;
		}
		return glGetError()==GL_NO_ERROR;
	}

	// init loop
	int iCount = 0;
	GLenum error = -1;

	while ((error = glGetError()) != GL_NO_ERROR)
	{
		++iCount;
		// only output log on debug
#ifdef _DEBUG
		
		switch(error)
		{
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			OutputDebugString( "----OPENGL ERROR: GL_INVALID_ENUM----  : " );
			break;
		case GL_INVALID_OPERATION:
			OutputDebugString( "----OPENGL ERROR: GL_INVALID_OPERATION----  : " );
			break;
		case GL_STACK_OVERFLOW:
			OutputDebugString( "----OPENGL ERROR: GL_STACK_OVERFLOW----  : " );
			break;
		case GL_STACK_UNDERFLOW:
			OutputDebugString( "----OPENGL ERROR: GL_STACK_UNDERFLOW----  : " );
			break;
		case GL_INVALID_VALUE:
			OutputDebugString( "----OPENGL ERROR: GL_INVALID_VALUE----  : " );
			break;
		case GL_OUT_OF_MEMORY:
			OutputDebugString( "----OPENGL ERROR: GL_OUT_OF_MEMORY----  : " );
			break;
		default:
			{
				char tmp[256];
				sprintf_s(tmp, 255, "----OPENGL ERROR: UNKNOWN: %i  : ", error);
				OutputDebugString( tmp );
			}
		}
		
		OutputDebugString("\n");

		if(iCount>5)
		{
			OutputDebugString("CheckOglError: Too many OPENGL error, ignoring some for them\n");
			break;
		}
#else
		if (iCount>5)
			break;
#endif // _DEBUG
	}

	// only assert in debug
#ifdef _DEBUG
	if(iCount>0)
	{
		if(assert && !alreadyAssertOnce)
		{
			// Dont assert here since it would exit app
			// and most of the time, the error is recoverbale
			//assert(false);
			alreadyAssertOnce=true;
		}
	}
#endif // _DEBUG
	
	// return true if no error
	if(iCount>0) --maxError;
	return iCount==0;
}

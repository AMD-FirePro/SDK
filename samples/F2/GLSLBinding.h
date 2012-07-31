#ifndef _GLSLBINDING_H_
#define _GLSLBINDING_H_


#include "UniformRefs.h"
#include <string>
#include <map>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_UNIFORM_SIZE	64

class GpuParams
{
public:
	GpuParams();
	~GpuParams();

	struct ParamEntry
	{
		union {
			float	valf[MAX_UNIFORM_SIZE];
			GLint	vali[MAX_UNIFORM_SIZE];
			};
		bool isSet;
		int nbElt;
		ParamEntry() : isSet(false), nbElt(1) {}
	};

	ParamEntry * mParams;

	int GetNamedParamIndex(const std::string& name);

	void SetParam(UniformName::Semantic uniform, const int *val);
	void SetParam(UniformName::Semantic uniform, const float *val);
	void SetParam(UniformName::Semantic uniform, const int *val, int type, int nb);
	void SetParam(UniformName::Semantic uniform, const float *val, int type, int nb);
	void SetParam(UniformName::Semantic uniform, const int val);
	void SetParam(UniformName::Semantic uniform, const float val);

	bool IsDirty()				{ return m_bGPUParamsDirty;	}
	void SetDirty(bool b)		{ m_bGPUParamsDirty = b;	}

protected:
	void InitParamNameMap();

	/// Mapping from parameter names to indexes
	typedef std::map<std::string, int> ParamNameMap;
	ParamNameMap mParamNameMap;

	bool m_bGPUParamsDirty;
};



///////////////////////////////////////////////////////////////////////////////////////////////
// this is GLSL dependent
class GLSLBindings
{
public:
	
	// Create an empty constructor and have the deconstructor release our memory.
	GLSLBindings();
	~GLSLBindings();
	
	void BuildUniformRefs(GLhandleARB hProgram, GpuParams * params);
	void UpdateUniforms(const GpuParams & params);
	void Finalize(GLhandleARB hProgram, GpuParams * params);
	
private:
	/// structure used to keep track of named uniforms in the linked program object
	struct UniformRef
	{
		int			mIndex;			// index in the global data set
		GLenum		mType;			// OpenGL type (float, vec2, int, etc...)
		GLint		mLocation;		// Location in the GLSL program
		bool		isReal;			// int or float
		GLsizei		mElementCount;	// number of element (vec2 = 2, etc...) 
	};
	
	typedef std::vector<UniformRef> UniformRefList;
	typedef UniformRefList::iterator UniformRefIt;
	/// container of uniform references that are active in the program object
	UniformRefList mUniformRefs;
	
	bool m_bUniformRefsBuilt;
};

#endif
#ifndef GLF_COMMON_INCLUDED
#define GLF_COMMON_INCLUDED

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/half_float.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/euler_angles.hpp>

// GLI
#include <gli/gli.hpp>
#include <gli/gtx/loader.hpp>

// STL
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <cstring>

namespace glf
{
#	ifdef WIN32
		static std::string const DATA_DIRECTORY("../data/");
#	else
		static std::string const DATA_DIRECTORY("../../data/");
#	endif
}//namespace glf

#endif//GLF_COMMON_INCLUDED

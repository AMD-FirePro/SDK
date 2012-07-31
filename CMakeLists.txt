cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
cmake_policy(VERSION 2.6)

project(firepro-sdk)
enable_testing()

#CMAKE_BUILD_TOOL

################################
# Add common source 

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external")

################################
# Add package

macro(addExternalPackageGTC NAME DIRECTORY)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/external/${DIRECTORY}/util)
	set(${NAME}_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/${DIRECTORY})

	find_package(${NAME})
	if(NOT ${NAME}_FOUND)
		message("ERROR: ${NAME} not found")
	endif(NOT ${NAME}_FOUND)
endmacro(addExternalPackageGTC)

################################
# Add Assimp 

if(MSVC AND NOT CMAKE_CL_64)
	set(ASSIMP_LIBRARY_DIRECTORY "assimp_release-dll_win32")
	set(ASSIMP_BINARY_DIRECTORY "assimp_release-dll_win32/Assimp32.dll")
endif(MSVC AND NOT CMAKE_CL_64)

if(MSVC AND CMAKE_CL_64)
	set(ASSIMP_LIBRARY_DIRECTORY "assimp_release-dll_x64")
	set(ASSIMP_BINARY_DIRECTORY "assimp_release-dll_x64/Assimp64.dll")
endif(MSVC AND CMAKE_CL_64)

set(ASSIMP_BINARY ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp-2.0.863-sdk/bin/${ASSIMP_BINARY_DIRECTORY}) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/assimp-2.0.863-sdk/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/assimp-2.0.863-sdk/lib/${ASSIMP_LIBRARY_DIRECTORY})

set(ASSIMP_LIBRARY assimp)

################################
# Add GLEXT 

find_package(OpenGL)
if(NOT OPENGL_FOUND)
	message("ERROR: OpenGL not found")
endif(NOT OPENGL_FOUND)

if(WIN32)
	set(GL_LIBRARY OpenGL32)
elseif(UNIX)
	set(GL_LIBRARY GL)
endif(WIN32)

################################
# Add FreeGLUT

if(MSVC AND NOT CMAKE_CL_64)
	set(FREEGLUT_BINARY_DIRECTORY "win32-vc")
endif(MSVC AND NOT CMAKE_CL_64)

if(MSVC AND CMAKE_CL_64)
	set(FREEGLUT_BINARY_DIRECTORY "win64-vc")
endif(MSVC AND CMAKE_CL_64)

set(FREEGLUT_LIBRARY freeglut_static)

add_definitions(-DFREEGLUT_STATIC)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/freeglut-2.6.0)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/freeglut-2.6.0/lib/${FREEGLUT_BINARY_DIRECTORY})

if(WIN32)
	set(FREEGLUT_LIBRARY freeglut_static)
elseif(UNIX)
	set(FREEGLUT_LIBRARY freeglut)
endif(WIN32)

################################
# Add GLM 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glm-0.9.3.3)

################################
# Add GLI 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/gli-0.3.1.0)

################################
# Add GLEW

if(MSVC AND NOT CMAKE_CL_64)
	set(GLEW_BINARY_DIRECTORY "win32-vc")
endif(MSVC AND NOT CMAKE_CL_64)

if(MSVC AND CMAKE_CL_64)
	set(GLEW_BINARY_DIRECTORY "win64-vc")
endif(MSVC AND CMAKE_CL_64)

add_definitions(-DGLEW_STATIC)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.7.0/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.7.0/lib/${GLEW_BINARY_DIRECTORY})

if(WIN32)
	set(GLEW_LIBRARY glew32s)
elseif(UNIX)
	set(GLEW_LIBRARY GLEW)
endif(WIN32)

################################
# Add OpenCL

if(MSVC AND NOT CMAKE_CL_64)
	set(CL_BINARY_DIRECTORY "x86")
endif(MSVC AND NOT CMAKE_CL_64)

if(MSVC AND CMAKE_CL_64)
	set(CL_BINARY_DIRECTORY "x86_64")
endif(MSVC AND CMAKE_CL_64)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/opencl-1.2)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/opencl-1.2/lib/${CL_BINARY_DIRECTORY})

set(CL_LIBRARY OpenCL)

################################
# Add libraries to executables

if(APPLE)
set(BINARY_FILES 
	${OPENGL_LIBRARY})
else(APPLE)
set(BINARY_FILES 
	${OPENGL_LIBRARY} 
	${FREEGLUT_LIBRARY} 
	${GLEW_LIBRARY})
endif(APPLE)

################################
# Add output directory

if(MSVC)
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/)
else(MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/debug )
	else(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/release )
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(MSVC)

################################
# Add definitions

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

################################
# function to create a project for the sample in the solution

function(createSample NAME)
	file(GLOB PROJECT_SOURCE *.cpp)
	file(GLOB_RECURSE PROJECT_SHADER *.vert *.frag *.geom *.cont *.eval *.glsl)
	file(GLOB PROJECT_INLINE *.inl)
	file(GLOB PROJECT_HEADER *.hpp)
	file(GLOB PROJECT_TEXT readme.txt)

	source_group("Text Files" FILES ${PROJECT_TEXT})
	source_group("Inline Files" FILES ${PROJECT_INLINE})
	source_group("Shader Files" FILES ${PROJECT_SHADER})

	add_executable(${NAME} ${PROJECT_SOURCE} ${PROJECT_INLINE} ${PROJECT_HEADER} ${PROJECT_SHADER} ${PROJECT_TEXT})
	add_custom_command(
		TARGET ${NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)
	add_custom_command(
		TARGET ${NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_BINARY} ${CMAKE_CURRENT_BINARY_DIR})

	target_link_libraries(${NAME} ${GL_LIBRARY} ${GLEW_LIBRARY} ${FREEGLUT_LIBRARY} ${ASSIMP_LIBRARY})
endfunction(createSample)

function(createSampleWGL NAME)
	file(GLOB PROJECT_SOURCE *.cpp)
	file(GLOB_RECURSE PROJECT_SHADER *.vert *.frag *.geom *.cont *.eval *.glsl)
	file(GLOB PROJECT_INLINE *.inl)
	file(GLOB PROJECT_HEADER *.hpp)
	file(GLOB PROJECT_TEXT readme.txt)

	source_group("Text Files" FILES ${PROJECT_TEXT})
	source_group("Inline Files" FILES ${PROJECT_INLINE})
	source_group("Shader Files" FILES ${PROJECT_SHADER})

	add_executable(${NAME} WIN32 ${PROJECT_SOURCE} ${PROJECT_INLINE} ${PROJECT_HEADER} ${PROJECT_SHADER} ${PROJECT_TEXT})
	add_custom_command(
		TARGET ${NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)
	add_custom_command(
		TARGET ${NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_BINARY} ${CMAKE_CURRENT_BINARY_DIR})

	target_link_libraries(${NAME} ${GL_LIBRARY} ${GLEW_LIBRARY} ${FREEGLUT_LIBRARY} ${ASSIMP_LIBRARY})
endfunction(createSampleWGL)

################################
# Add individual samples

add_subdirectory(samples)

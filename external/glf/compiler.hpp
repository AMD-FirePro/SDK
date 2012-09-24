//**********************************
// Compiler
// 21/09/2012 - 21/09/2012
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#ifndef GLF_COMPILER_INCLUDED
#define GLF_COMPILER_INCLUDED

#include "common.hpp"

namespace glf
{
	std::string format(const char* msg, ...);
	std::string loadFile(std::string const & Filename);
	GLuint createShader(GLenum Type, std::string const & Source);
	GLuint createShader(GLenum Type, std::string const & Arguments, std::string const & Source);
	bool checkError(const char* Title);
	bool checkProgram(GLuint ProgramName);
	bool checkShader(GLuint ShaderName, std::string const & File);
	bool validateProgram(GLuint ProgramName);

	class compiler
	{
		typedef std::map<std::string, GLuint> names_map;
		typedef std::map<GLuint, std::string> files_map;

		class commandline
		{
			enum profile
			{
				CORE,
				COMPATIBILITY
			};

		public:
			commandline(std::string const & Arguments) :
				Profile("core"),
				Version(-1)
			{
				this->parseArguments(Arguments);
			}

			void parseArguments
			(
				std::string const & Arguments
			)
			{
				std::stringstream Stream(Arguments);
				std::string Param;

				while(!Stream.eof())
				{
					Stream >> Param;

					std::size_t Found = Param.find("-D");
					if(Found != std::string::npos)
						this->Defines.push_back(Param.substr(2, Param.size() - 2));
					else if(Param == "--define")
					{
						std::string Define;
						Stream >> Define;
						this->Defines.push_back(Define);
					}
					else if((Param == "--version") || (Param == "-v"))
						Stream >> Version;
					else if((Param == "--profile") || (Param == "-p"))
						Stream >> Profile;
					else if (Param == "--include" || Param == "-i")
					{
						std::string Include;
						Stream >> Include;
						this->Includes.push_back(Include);
					}
		/*
					else 
					{
						std::stringstream err;
						err << "unknown parameter type: \"" << Param << "\"";
						glDebugMessageInsertARB(
							GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 1, GL_DEBUG_SEVERITY_LOW_ARB, 
							-1, std::string(std::string("unknown parameter type: \"") << Param << std::string("\"")).c_str());
					}
					if(!Stream) 
					{
						glDebugMessageInsertARB(
							GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 1, GL_DEBUG_SEVERITY_LOW_ARB, 
							-1, std::string(std::string("error parsing parameter: \"") << Param << std::string("\"")).c_str());
					}
		*/
				}
			}

			int getVersion() const
			{
				return this->Version;
			}

			std::string getProfile() const
			{
				return this->Profile;
			}

			std::string getDefines() const
			{
				std::string Result;
				for(std::size_t i = 0; i < this->Defines.size(); ++i)
					Result += std::string("#define ") + this->Defines[i] + std::string("\n");
				return Result;
			}

		private:
			std::string Profile;
			int Version;
			std::vector<std::string> Defines;
			std::vector<std::string> Includes;
		};

		class parser
		{
		public:
			std::string operator() 
			(
				commandline const & CommandLine,
				std::string const & Source
			)
			{
				std::stringstream Stream;
				Stream << Source;
				std::string Line, Text;

				// Handle command line version and profile arguments
				if(CommandLine.getVersion() != -1)
					Text += glf::format("#version %d %s\n", CommandLine.getVersion(), CommandLine.getProfile().c_str());

				// Handle command line defines
				Text += CommandLine.getDefines();

				while(std::getline(Stream, Line))
				{
					std::size_t Offset = 0;

					// Version
					Offset = Line.find("#version");
					if(Offset != std::string::npos)
					{
						std::size_t CommentOffset = Line.find("//");
						if(CommentOffset != std::string::npos && CommentOffset < Offset)
							continue;

						// Reorder so that the #version line is always the first of a shader text
						if(CommandLine.getVersion() == -1)
							Text = Line + std::string("\n") + Text;
						// else skip is version is only mentionned
						continue;
					}

					// Include
					Offset = Line.find("#include");
					if(Offset != std::string::npos)
					{
						std::size_t CommentOffset = Line.find("//");
						if(CommentOffset != std::string::npos && CommentOffset < Offset)
							continue;

						Text += parseInclude(Line, Offset);
						continue;
					} 

					Text += Line + "\n";
				}

				return Text;
			}

		private:
			std::string parseInclude(std::string const & Line, std::size_t const & Offset)
			{
				std::string Result;

				std::string::size_type IncludeFirstQuote = Line.find("\"", Offset);
				std::string::size_type IncludeSecondQuote = Line.find("\"", IncludeFirstQuote + 1);
				std::string::size_type IncludeEndl = Line.find("\n", Offset);

				std::string IncludeName = Line.substr(IncludeFirstQuote + 1, IncludeSecondQuote - IncludeFirstQuote - 1);
				std::string PathName = glf::DATA_DIRECTORY + "gl-420/" + IncludeName;

				return glf::loadFile(PathName);;
			}
		};

	public:
		~compiler();

		GLuint create(GLenum Type, std::string const & Filename);
		GLuint create(GLenum Type, std::string const & Arguments, std::string const & Filename);
		bool destroy(GLuint const & Name);

		bool check();
		// TODO: Not defined
		bool check(GLuint const & Name);
		void clear();

	private:
		bool loadBinary(
			std::string const & Filename,
			GLenum & Format,
			std::vector<glm::byte> & Data,
			GLint & Size);
		std::string loadFile(
			std::string const & Filename);

		names_map ShaderNames;
		files_map ShaderFiles;
		names_map PendingChecks;
	};

}//namespace glf

#include "compiler.inl"

#endif//GLF_COMPILER_INCLUDED

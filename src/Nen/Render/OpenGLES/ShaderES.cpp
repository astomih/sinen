#include "ShaderES.h"
#include <Nen.hpp>
#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <GLES3/gl3.h>
namespace nen::es
{

	ShaderES::ShaderES()
		: mShaderProgram(0), mVertexShader(0), mFragShader(0)
	{
	}

	bool ShaderES::Load(const std::string &vertName, const std::string &fragName)
	{
		// Compile vertex and pixel shaders
		if (!CompileShader(fragName, GL_FRAGMENT_SHADER, mFragShader) ||
			!CompileShader(vertName, GL_VERTEX_SHADER, mVertexShader))
		{
			return false;
		}

		// Now create a shader program that links together the vertex/frag shaders
		mShaderProgram = glCreateProgram();
		glAttachShader(mShaderProgram, mVertexShader);
		glAttachShader(mShaderProgram, mFragShader);
		glLinkProgram(mShaderProgram);

		// Verify that the program linked successfully
		if (!IsValidProgram())
		{
			return false;
		}

		return true;
	}

	void ShaderES::Unload()
	{
		// Delete the program/shaders
		glDeleteProgram(mShaderProgram);
		glDeleteShader(mVertexShader);
		glDeleteShader(mFragShader);
	}

	void ShaderES::SetActive(const GLuint &blockIndex)
	{
		// Set this program as the active one
		glUseProgram(mShaderProgram);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, blockIndexBuffers[blockIndex]);
		glUniformBlockBinding(mShaderProgram, blockIndex, 1);
	}

	void ShaderES::SetDisable()
	{
		glDisable(mShaderProgram);
	}

	bool ShaderES::CompileShader(const std::string &fileName,
								 GLenum shaderType,
								 GLuint &outShader)
	{
#ifdef ANDROID
		SDL_RWops *file{SDL_RWFromFile(fileName.c_str(), "r")};
		size_t fileLength{static_cast<size_t>(SDL_RWsize(file))};
		void *data{SDL_LoadFile_RW(file, nullptr, 1)};
		std::string result(static_cast<char *>(data), fileLength);
		SDL_free(data);
		// Create a shader of the specified type
		outShader = glCreateShader(shaderType);
		const char *contentchar = result.c_str();
		// Set the source characters and try to compile
		glShaderSource(outShader, 1, &(contentchar), nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			std::cout << "Failed to compile shader " << fileName << std::endl;
			return false;
		}
#else

		// Open file
		std::ifstream shaderFile(fileName);
		if (!shaderFile.is_open())
		{
			std::cout << "Shader file not found: " << fileName << std::endl;
			Logger::Error("Shader file not found: %s", fileName);
			return false;
		}
		// Read all the text into a string
		std::stringstream sstream;
		sstream << shaderFile.rdbuf();
		std::string contents = sstream.str();
		const char *contentsChar = contents.c_str();

		// Create a shader of the specified type
		outShader = glCreateShader(shaderType);
		// Set the source characters and try to compile
		glShaderSource(outShader, 1, &(contentsChar), nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			std::cout << "Failed to compile shader " << fileName << std::endl;
			return false;
		}
#endif
		return true;
	}

	bool ShaderES::IsCompiled(GLuint shader)
	{
		GLint status;
		// Query the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

		if (status != GL_TRUE)
		{
			char buffer[512];
			memset(buffer, 0, 512);
			glGetShaderInfoLog(shader, 511, nullptr, buffer);
			std::cout << "GLSL Compile Failed: " << buffer << std::endl;
			return false;
		}

		return true;
	}

	bool ShaderES::IsValidProgram()
	{
		GLint status;
		// Query the link status
		glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
		if (status != GL_TRUE)
		{
			char buffer[512];
			memset(buffer, 0, 512);
			glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
			std::cout << "GLSL Link Status: " << buffer << std::endl;
			return false;
		}

		return true;
	}

	bool ShaderES::CreateUBO(const GLuint &blockIndex, const size_t &size, const void *data)
	{
		GLuint BIB; //blockIndexBuffer
		glGenBuffers(1, &BIB);
		glBindBuffer(GL_UNIFORM_BUFFER, BIB);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
		glUniformBlockBinding(mShaderProgram, blockIndex, 1);
		blockIndexBuffers.emplace(blockIndex, BIB);
		return true;
	}

	void ShaderES::UpdateUBO(const GLuint &blockIndex, const size_t &size, const void *data, const GLsizeiptr &offset)
	{
		auto BIB = blockIndexBuffers[blockIndex];
		glBindBuffer(GL_UNIFORM_BUFFER, BIB);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

} //namespace nen::es

#endif
#include <Engine.hpp>
#include "../../include/OpenGL/ShaderGL.h"
#include "../../include/OpenGL/GLRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_image.h>
#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/common.hpp>
#include <GL/glew.h>
namespace nen::gl
{

	ShaderGL::ShaderGL()
		: mShaderProgram(0), mVertexShader(0), mFragShader(0)
	{
	}

	bool ShaderGL::Load(const std::string &vertName, const std::string &fragName)
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

	void ShaderGL::Unload()
	{
		// Delete the program/shaders
		glDeleteProgram(mShaderProgram);
		glDeleteShader(mVertexShader);
		glDeleteShader(mFragShader);
	}

	void ShaderGL::SetActive()
	{
		// Set this program as the active one
		glUseProgram(mShaderProgram);
	}

	void ShaderGL::SetDisable()
	{
		glDisable(mShaderProgram);
	}

	void ShaderGL::SetMatrixUniform(const char *name, const glm::mat4 &matrix)
	{
		// Find the uniform by this name
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		// Send the matrix data to the uniform
		glUniformMatrix4fv(loc, 1, GL_TRUE, glm::value_ptr(matrix));
	}

	void ShaderGL::SetColorUniform(const char *name, const Color::Color &color)
	{
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		glUniform4f(loc, color.r, color.g, color.b, color.a);
	}

	void ShaderGL::SetVector2fUniform(const char *name, const Vector2f &vector)
	{
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		// Send the vector data
		glUniform2fv(loc, 1, vector.GetAsFloatPtr());
	}

	void ShaderGL::SetBoolUniform(const char *name, const bool boolean)
	{
		glUniform1i(glGetUniformLocation(mShaderProgram, name), boolean);
	}

	void ShaderGL::SetIntUniform(const char *name, const int integer)
	{
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		// Send the float data
		glUniform1i(loc, integer);
	}

	void ShaderGL::SetVectorUniform(const char *name, const glm::vec3 &vector)
	{
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		// Send the vector data
		glUniform3fv(loc, 1, (&vector[0]));
	}

	void ShaderGL::SetFloatUniform(const char *name, const float value)
	{
		GLuint loc = glGetUniformLocation(mShaderProgram, name);
		// Send the float data
		glUniform1f(loc, value);
	}

	bool ShaderGL::CompileShader(const std::string &fileName,
								 GLenum shaderType,
								 GLuint &outShader)
	{
		// Open file
		std::ifstream shaderFile(fileName);
		if (shaderFile.is_open())
		{
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
				SDL_Log("Failed to compile shader %s", fileName.c_str());
				return false;
			}
		}
		else
		{
			SDL_Log("Shader file not found: %s", fileName.c_str());
			return false;
		}

		return true;
	}

	bool ShaderGL::IsCompiled(GLuint shader)
	{
		GLint status;
		// Query the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

		if (status != GL_TRUE)
		{
			char buffer[512];
			memset(buffer, 0, 512);
			glGetShaderInfoLog(shader, 511, nullptr, buffer);
			SDL_Log("GLSL Compile Failed:\n%s", buffer);
			return false;
		}

		return true;
	}

	bool ShaderGL::IsValidProgram()
	{
		GLint status;
		// Query the link status
		glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &status);
		if (status != GL_TRUE)
		{
			char buffer[512];
			memset(buffer, 0, 512);
			glGetProgramInfoLog(mShaderProgram, 511, nullptr, buffer);
			SDL_Log("GLSL Link Status:\n%s", buffer);
			return false;
		}

		return true;
	}

} //namespace nen::gl
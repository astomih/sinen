#pragma once
#if defined(EMSCRIPTEN) || defined(MOBILE)
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengles2.h>
#include <Math/Math.hpp>
#include <Math/Vector2.hpp>
#include <Math/Vector3.hpp>
#include <Math/Matrix3.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Quaternion.hpp>
#include <Color/Color.hpp>

namespace nen::es
{
	class ShaderES
	{
	public:
		ShaderES();
		~ShaderES() {}
		// Load the vertex/fragment shaders with the given names
		bool Load(const std::string &vertName, const std::string &fragName);
		void Unload();
		// Set this as the active shader program
		void SetActive();
		void SetDisable();
		// Sets a Matrix uniform
		void SetMatrixUniform(const char *name, const Matrix4 &matrix);
		void SetColorUniform(const char *name, const Color &color);
		void SetVector2fUniform(const char *name, const Vector2 &vuv);
		void SetBoolUniform(const char *name, const bool boolean);
		void SetIntUniform(const char *name, const int integer);
		// Sets a Vector3 uniform
		void SetVectorUniform(const char *name, const Vector3 &vector);
		void SetFloatUniform(const char *name, const float value);

	private:
		// Tries to compile the specified shader
		bool CompileShader(const std::string &fileName,
						   GLenum shaderType,
						   GLuint &outShader);

		// Tests whether shader compiled successfully
		bool IsCompiled(GLuint shader);
		// Tests whether vertex/fragment programs link
		bool IsValidProgram();

	private:
		// Store the shader object IDs
		GLuint mVertexShader;
		GLuint mFragShader;
		GLuint mShaderProgram;
	};

}
#endif
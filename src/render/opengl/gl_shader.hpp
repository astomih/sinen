#ifndef SINEN_GL_SHADER_HPP
#define SINEN_GL_SHADER_HPP

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif
#include <string>
#include <unordered_map>

namespace sinen {
class gl_shader {
public:
  /**
   * @brief Constructor
   *
   */
  gl_shader();
  /**
   * @brief Default Destructor
   *
   */
  ~gl_shader() = default;

  /**
   * @brief Load shader from file
   *
   * @param vertName Vertex shader file name
   * @param fragName Fragment shader file name
   * @return true Success to load shader
   * @return false Failed to load shader
   */
  bool load(const std::string &vertName, const std::string &fragName);

  /**
   * @brief Unload the shader
   *
   */
  void unload();

  /**
   * @brief Active to shader
   *
   * @param blockIndex Valid block index
   */
  void active(const GLuint &blockIndex);

  /**
   * @brief Disable to shader
   *
   */
  void disable();

  /**
   * @brief Create UBO(Uniform Buffer Object)
   *
   * @param blockIndex Block Index
   * @param size Buffer size
   * @param data data
   * @return true Success to create UBO
   * @return false Failed to create UBO
   */
  bool create_ubo(const GLuint &blockIndex, const size_t &size,
                  const void *data);

  /**
   * @brief UBOを更新する
   *
   * @param blockIndex Block Index
   * @param size バッファのサイズ
   * @param data データ(size内に収めること)
   * @param offset 更新のオフセット
   */
  void update_ubo(const GLuint &blockIndex, const size_t &size,
                  const void *data, const GLsizeiptr &offset = 0);
  GLuint program() const { return mShaderProgram; }

private:
  bool CompileShader(const std::string &fileName, GLenum shaderType,
                     GLuint &outShader);

  bool IsCompiled(GLuint shader);
  bool IsValidProgram();

private:
  std::unordered_map<GLuint, GLuint> blockIndexBuffers;
  GLuint mVertexShader;
  GLuint mFragShader;
  GLuint mShaderProgram;
};

} // namespace sinen
#endif // !SINEN_GL_SHADER_HPP
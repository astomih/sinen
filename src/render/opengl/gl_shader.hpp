
#pragma once

#if defined(EMSCRIPTEN) || defined(MOBILE)
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif
#include <string>
#include <unordered_map>

namespace nen {
class gl_shader {
public:
  /**
   * @brief コンストラクタ
   *
   */
  gl_shader();
  /**
   * @brief デストラクタ
   *
   */
  ~gl_shader() = default;

  /**
   * @brief シェーダーをファイルから読み込む
   *
   * @param vertName 頂点シェーダのファイルパス
   * @param fragName フラグメントシェーダのファイルパス
   * @return true 読み込み成功
   * @return false 読み込み失敗(Loggerを参照)
   */
  bool load(const std::string &vertName, const std::string &fragName);

  /**
   * @brief 読み込んだシェーダを閉じる
   *
   */
  void unload();

  /**
   * @brief シェーダを有効にする
   *
   * @param blockIndex 有効なBlock Index
   */
  void active(const GLuint &blockIndex);

  /**
   * @brief シェーダを無効にする
   *
   */
  void disable();

  /**
   * @brief UBO(Uniform Buffer Object)を作成
   *
   * @param blockIndex Block Index
   * @param size バッファのサイズ
   * @param data データ（size内に収めること）
   * @return true UBOの作成に成功
   * @return false UBOの作成に失敗
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

} // namespace nen
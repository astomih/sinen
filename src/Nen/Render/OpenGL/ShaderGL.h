#pragma once
#if !defined(EMSCRIPTEN) && !defined(MOBILE)
#include <Color/Color.hpp>
#include <GL/glew.h>
#include <Math/Math.hpp>
#include <Math/Matrix3.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Quaternion.hpp>
#include <Math/Vector2.hpp>
#include <Math/Vector3.hpp>
#include <unordered_map>


namespace nen::gl {
class ShaderGL {
public:
  /**
   * @brief コンストラクタ
   *
   */
  ShaderGL();
  /**
   * @brief デストラクタ
   *
   */
  ~ShaderGL() = default;

  /**
   * @brief シェーダーをファイルから読み込む
   *
   * @param vertName 頂点シェーダのファイルパス
   * @param fragName フラグメントシェーダのファイルパス
   * @return true 読み込み成功
   * @return false 読み込み失敗(Loggerを参照)
   */
  bool Load(const std::string &vertName, const std::string &fragName);

  /**
   * @brief 読み込んだシェーダを閉じる
   *
   */
  void Unload();

  /**
   * @brief シェーダを有効にする
   *
   * @param blockIndex 有効なBlock Index
   */
  void SetActive(const GLuint &blockIndex);

  /**
   * @brief シェーダを無効にする
   *
   */
  void SetDisable();

  /**
   * @brief UBO(Uniform Buffer Object)を作成
   *
   * @param blockIndex Block Index
   * @param size バッファのサイズ
   * @param data データ（size内に収めること）
   * @return true UBOの作成に成功
   * @return false UBOの作成に失敗
   */
  bool CreateUBO(const GLuint &blockIndex, const size_t &size,
                 const void *data);

  /**
   * @brief UBOを更新する
   *
   * @param blockIndex Block Index
   * @param size バッファのサイズ
   * @param data データ(size内に収めること)
   * @param offset 更新のオフセット
   */
  void UpdateUBO(const GLuint &blockIndex, const size_t &size, const void *data,
                 const GLsizeiptr &offset = 0);

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

} // namespace nen::gl
#endif
#ifndef SINEN_DRAW_OBJECT_WRAPPER_HPP
#define SINEN_DRAW_OBJECT_WRAPPER_HPP
#include "../../asset/model/model.hpp"
#include "../../asset/texture/texture.hpp"
#include "drawable.hpp"

namespace sinen {
/**
 * @brief 2D draw object class
 *
 */
struct Draw2D {
public:
  /**
   * @brief Construct a new draw2d object
   *
   */
  Draw2D();
  /**
   * @brief Construct a new draw2d object
   * @param texture Texture to use
   *
   */
  Draw2D(Texture);
  /**
   * @brief Draw object
   *
   */
  void draw();
  /**
   * @brief Set user data
   *
   * @param index Index of user data 0-16
   * @param value Value of user data
   */
  void user_data_at(int index, float value);
  void add(const glm::vec2 &position, const float &rotation,
           const glm::vec2 &scale);
  void at(const int &index, const glm::vec2 &position, const float &rotation,
          const glm::vec2 &scale);
  void clear();
  struct world {
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
  };
  std::vector<world> worlds;
  glm::vec2 position;
  float rotation;
  glm::vec2 scale;
  Material material;
  Model model;
  std::shared_ptr<Drawable> obj;
};
/**
 * @brief 3D draw object class
 *
 */
class Draw3D {
public:
  /**
   * @brief Construct a new draw3d object
   *
   */
  Draw3D();
  /**
   * @brief Draw object
   *
   */
  void draw();
  /**
   * @brief Set user data
   *
   * @param index Index of user data 0-16
   * @param value Value of user data
   */
  void user_data_at(int index, float value);
  void add(const glm::vec3 &position, const glm::vec3 &rotation,
           const glm::vec3 &scale);
  void at(const int &index, const glm::vec3 &position,
          const glm::vec3 &rotation, const glm::vec3 &scale);
  void clear();
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  Material material;
  bool is_draw_depth = true;
  Model model;
  std::shared_ptr<Drawable> obj;
  struct world {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
  };
  std::vector<world> worlds;
};
} // namespace sinen
#endif // !SINEN_DRAW_OBEJCT_WRAPPER_HPP

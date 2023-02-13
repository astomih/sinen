#ifndef SINEN_DRAW_OBJECT_WRAPPER_HPP
#define SINEN_DRAW_OBJECT_WRAPPER_HPP
#include "../math/vector2.hpp"
#include "../math/vector3.hpp"
#include "../texture/texture.hpp"
#include "drawable.hpp"

namespace sinen {
/**
 * @brief 2D draw object class
 *
 */
struct draw2d {
public:
  /**
   * @brief Construct a new draw2d object
   *
   */
  draw2d();
  /**
   * @brief Construct a new draw2d object
   * @param texture Texture to use
   *
   */
  draw2d(texture);
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
  void add(const vector2 &position, const float &rotation,
           const vector2 &scale);
  void at(const int &index, const vector2 &position, const float &rotation,
          const vector2 &scale);
  void clear();
  struct world {
    vector2 position;
    float rotation;
    vector2 scale;
  };
  std::vector<world> worlds;
  vector2 position;
  float rotation;
  vector2 scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
  std::shared_ptr<drawable> obj;
};
/**
 * @brief UI draw object class
 *
 */
struct drawui {
public:
  /**
   * @brief Construct a new drawui object
   *
   */
  drawui();
  /**
   * @brief Construct a new drawui object
   * @param texture Texture to use
   *
   */
  drawui(texture);
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
  void add(const vector2 &position, const float &rotation,
           const vector2 &scale);
  void at(const int &index, const vector2 &position, const float &rotation,
          const vector2 &scale);
  void clear();
  struct world {
    vector2 position;
    float rotation;
    vector2 scale;
  };
  std::vector<world> worlds;
  vector2 position;
  float rotation;
  vector2 scale;
  texture texture_handle;
  std::string vertex_name = "SPRITE";
  std::shared_ptr<drawable> obj;
};
/**
 * @brief 3D draw object class
 *
 */
class draw3d {
public:
  /**
   * @brief Construct a new draw3d object
   *
   */
  draw3d();
  /**
   * @brief Construct a new draw3d object
   * @param texture Texture to use
   *
   */
  draw3d(texture);
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
  void add(const vector3 &position, const vector3 &rotation,
           const vector3 &scale);
  void at(const int &index, const vector3 &position, const vector3 &rotation,
          const vector3 &scale);
  void clear();
  vector3 position;
  vector3 rotation;
  vector3 scale;
  texture texture_handle;
  bool is_draw_depth = true;
  std::string vertex_name = "SPRITE";
  std::shared_ptr<drawable> obj;
  struct world {
    vector3 position;
    vector3 rotation;
    vector3 scale;
  };
  std::vector<world> worlds;
};
} // namespace sinen
#endif // !SINEN_DRAW_OBEJCT_WRAPPER_HPP

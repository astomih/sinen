#ifndef SINEN_CAMERA_HPP
#define SINEN_CAMERA_HPP
#include "../math/matrix4.hpp"
#include "../math/vector3.hpp"

namespace sinen {
/**
 * @brief Camera class
 *
 */
struct camera {
public:
  /**
   * @brief Construct a new camera object
   *
   */
  camera();
  /**
   * @brief Update the camera
   *
   */
  void update();
  matrix4 view;
  matrix4 projection;
  vector3 position;
  vector3 target;
  vector3 up;
  float fov;
  float aspect;
  float near;
  float far;
};
} // namespace sinen
#endif

#ifndef SINEN_MATRIX_HPP
#define SINEN_MATRIX_HPP

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

namespace sinen {
using Mat4 = glm::mat4x4;
} // namespace sinen

#endif
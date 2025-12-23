#ifndef SINEN_QUATERNION_HPP
#define SINEN_QUATERNION_HPP

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sinen {
using Quaternion = glm::quat;
} // namespace sinen

#endif

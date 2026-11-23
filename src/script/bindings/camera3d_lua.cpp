#include "luaapi.hpp"
#include <graphics/camera/camera3d.hpp>

namespace sinen {
void registerCamera(luau::table &sn) {
  engineClass<Camera3D>(sn, Camera3D::metaTableName())
      .constructor<>()
      .method<&Camera3D::lookat>("lookat")
      .method<&Camera3D::perspective>("perspective")
      .method<&Camera3D::orthographic>("orthographic")
      .method<&Camera3D::getPosition>("getPosition")
      .method<&Camera3D::getTarget>("getTarget")
      .method<&Camera3D::getUp>("getUp")
      .method<&Camera3D::getView>("getView")
      .method<&Camera3D::getProjection>("getProjection")
      .method<&Camera3D::isAABBInFrustum>("isAABBInFrustum")
      .method("getViewProjection",
              [](const Camera3D &camera) {
                return camera.getView() * camera.getProjection();
              })
      .method("worldToScreen",
              [](const Camera3D &camera, const Vec3 &position,
                 std::optional<Vec2> viewport) {
                return viewport ? camera.worldToScreen(position, *viewport)
                                : camera.worldToScreen(position);
              })
      .method("screenToWorldRay",
              [](const Camera3D &camera, const Vec2 &position) {
                return camera.screenToWorldRay(position);
              });
}
} // namespace sinen

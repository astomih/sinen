#include "luaapi.hpp"
#include <graphics/camera/camera2d.hpp>

namespace sinen {
void registerCamera2D(luau::table &sn) {
  engineClass<Camera2D>(sn, Camera2D::metaTableName())
      .constructor<>()
      .method<&Camera2D::size>("size")
      .method<&Camera2D::half>("half")
      .method<&Camera2D::resize>("resize")
      .method<&Camera2D::windowRatio>("windowRatio")
      .method<&Camera2D::invWindowRatio>("invWindowRatio")
      .method<&Camera2D::rect>("rect")
      .method<&Camera2D::topLeft>("topLeft")
      .method<&Camera2D::topCenter>("topCenter")
      .method<&Camera2D::topRight>("topRight")
      .method<&Camera2D::left>("left")
      .method<&Camera2D::center>("center")
      .method<&Camera2D::right>("right")
      .method<&Camera2D::bottomLeft>("bottomLeft")
      .method<&Camera2D::bottomCenter>("bottomCenter")
      .method<&Camera2D::bottomRight>("bottomRight");
}
} // namespace sinen

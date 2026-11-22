#include "script/bindings/luaapi.hpp"
#include <graphics/video/video.hpp>
#include <gtest/gtest.h>
#include <script/script.hpp>

namespace {
class EngineBindings : public testing::Test {
protected:
  void SetUp() override { ASSERT_TRUE(sinen::Script::initialize(false)); }
  void TearDown() override { sinen::Script::shutdown(); }

  testing::AssertionResult run(const char *source) {
    auto *L = sinen::getGlobalLua();
    const auto bytecode = Luau::compile(source);
    if (luau_load(L, "engine_binding_test", bytecode.data(), bytecode.size(),
                  0) != LUA_OK ||
        lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char *message = lua_tostring(L, -1);
      return testing::AssertionFailure() << (message ? message : "Luau error");
    }
    return testing::AssertionSuccess();
  }
};
} // namespace

TEST_F(EngineBindings, MathConstructorsFieldsOperatorsAndMethods) {
  ASSERT_TRUE(run(R"(
local sn = require("@sinen")
assert(sn.Vec2.new().x == 0)
assert(sn.Vec2.new(4).y == 4)
local v = sn.Vec3.new(1, 2, 3)
v.y = 5
local copy = v:copy()
copy.x = 10
assert(v.x == 1 and copy.x == 10)
local result = (v + sn.Vec3.new(2)) * sn.Vec3.new(2)
assert(result.x == 6 and result.y == 14 and result.z == 10)
assert(sn.Vec3.new(3, 0, 0):length() == 3)
assert(sn.Vec3.new(3, 0, 0):normalize().x == 1)
assert(sn.Vec3.new(1, 0, 0):dot(sn.Vec3.new(2, 0, 0)) == 2)
assert(v:lerp(v, 0.5).y == 5)
local color = sn.Color.new(0.5, 0.25)
assert(color.r == 0.5 and color.a == 0.25)
assert(sn.Color.new(1, 0, 0).a == 1)
color.g = 0.75
assert(color.g == 0.75 and #tostring(color) > 0)
local rect = sn.Rect.new(sn.Vec2.new(1, 2), sn.Vec2.new(4, 6))
rect.width = 8
assert(rect:center().x == 5 and rect:size().y == 6)
assert(sn.Mat4.identity():copy():transpose() ~= nil)
assert(not pcall(function() v.unknown = 1 end))
assert(not pcall(function() v.x = "bad" end))
assert(not pcall(function() v:dot(sn.Vec2.new()) end))
)"));
}

TEST_F(EngineBindings, TypedAndRawHandlersShareValuesAndBorrowedFields) {
  ASSERT_TRUE(run(R"(
local sn = require("@sinen")
local transform = sn.Transform.new()
transform.position = sn.Vec3.new(1, 2, 3)
transform.position.x = 9
assert(transform.position.x == 9)
assert(transform:getWorldMatrix():copy() ~= nil)
local bounds = sn.AABB.new()
bounds.min = sn.Vec3.new(-2)
bounds.max = sn.Vec3.new(2)
bounds.min.y = -4
assert(bounds.min.y == -4)
local ray = sn.Ray.new(sn.Vec3.new(1), sn.Vec3.new(0, 0, 1))
ray.origin.x = 7
assert(ray.origin.x == 7)
local camera = sn.Camera3D.new()
camera:lookat(sn.Vec3.new(0, 0, 5), sn.Vec3.new(), sn.Vec3.new(0, 1, 0))
assert(camera:getPosition().z == 5)
local buffer = sn.Buffer.new({sn.Vec3.new(1, 2, 3), sn.Vec2.new(4, 5)})
assert(buffer ~= nil)
assert(sn.Random.getIntRange(4, 4) == 4)
assert(not pcall(function() sn.Random.getIntRange(1.5, 3) end))
)"));
  lua_gc(sinen::getGlobalLua(), LUA_GCCOLLECT, 0);
}

TEST_F(EngineBindings, SharedResourcesAndRepeatedCollection) {
  ASSERT_TRUE(run(R"(
local sn = require("@sinen")
for i = 1, 50 do
  local writer = sn.VideoWriter.new()
  assert(writer:frameCount() == 0)
  writer:close()
  assert(type(writer:lastError()) == "string")
  local reader = sn.VideoReader.new()
  reader:close()
  assert(type(reader:lastError()) == "string")
  local timer = sn.Timer.new()
  timer:setTime(10)
  timer:start()
  assert(timer:isStarted())
  timer:stop()
  assert(not timer:isStarted())
end
)"));
  lua_gc(sinen::getGlobalLua(), LUA_GCCOLLECT, 0);
  lua_gc(sinen::getGlobalLua(), LUA_GCCOLLECT, 0);
}

TEST_F(EngineBindings, CollectionReleasesSharedResources) {
  ASSERT_TRUE(run("resource = require('@sinen').VideoWriter.new()"));
  auto *L = sinen::getGlobalLua();
  lua_getglobal(L, "resource");
  std::weak_ptr<sinen::VideoWriter> resource =
      sinen::luau::stack<sinen::Ptr<sinen::VideoWriter>>::get(L, -1);
  lua_pop(L, 1);
  ASSERT_FALSE(resource.expired());
  ASSERT_TRUE(run("resource = nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_TRUE(resource.expired());
}

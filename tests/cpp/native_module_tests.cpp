#include <lua.h>
#include <lualib.h>

#include <gtest/gtest.h>

#include "script/native_module.hpp"

#include <filesystem>
#include <string>

#ifndef SINEN_TEST_NATIVE_MODULE_PATH
#error SINEN_TEST_NATIVE_MODULE_PATH must name the test module
#endif
#ifndef SINEN_TEST_FAILING_NATIVE_MODULE_PATH
#error SINEN_TEST_FAILING_NATIVE_MODULE_PATH must name the failing test module
#endif

namespace {
class ScopedCurrentDirectory {
public:
  explicit ScopedCurrentDirectory(const std::filesystem::path &path)
      : original(std::filesystem::current_path()) {
    std::filesystem::current_path(path);
  }

  ~ScopedCurrentDirectory() {
    std::error_code ignored;
    std::filesystem::current_path(original, ignored);
  }

private:
  std::filesystem::path original;
};
} // namespace

TEST(NativeModuleTests, LoadsModuleAndKeepsItAliveUntilStateCloses) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);
  luaL_openlibs(state);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  ASSERT_TRUE(modules.load(state, SINEN_TEST_NATIVE_MODULE_PATH))
      << modules.lastError();

  lua_getglobal(state, "native_module_fixture");
  ASSERT_TRUE(lua_istable(state, -1));
  lua_getfield(state, -1, "answer");
  ASSERT_TRUE(lua_isfunction(state, -1));
  ASSERT_EQ(lua_pcall(state, 0, 1, 0), LUA_OK)
      << (lua_tostring(state, -1) ? lua_tostring(state, -1)
                                  : "unknown native module error");
  EXPECT_EQ(lua_tointeger(state, -1), 42);
  lua_settop(state, 0);

  modules.close(state);
  lua_getglobal(state, "native_module_fixture_closed");
  EXPECT_TRUE(lua_toboolean(state, -1));

  lua_close(state);
  modules.unload();
}

TEST(NativeModuleTests, ReportsLoaderErrors) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  EXPECT_FALSE(modules.load(state, "module_that_does_not_exist.dll"));
  EXPECT_FALSE(modules.lastError().empty());

  lua_close(state);
  modules.unload();
}

TEST(NativeModuleTests, PluginLookupIsLimitedToCurrentDirectory) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  const std::filesystem::path modulePath = SINEN_TEST_NATIVE_MODULE_PATH;
  const ScopedCurrentDirectory currentDirectory(modulePath.parent_path());

  EXPECT_FALSE(modules.loadPlugin(state, modulePath.string()));
  EXPECT_NE(modules.lastError().find("directory separator"), std::string::npos);
  EXPECT_FALSE(modules.loadPlugin(state, "../outside"));
  EXPECT_NE(modules.lastError().find("directory separator"), std::string::npos);

  const auto encodedName = modulePath.filename().u8string();
  const std::string moduleName(
      reinterpret_cast<const char *>(encodedName.data()), encodedName.size());
  EXPECT_TRUE(modules.loadPlugin(state, moduleName)) << modules.lastError();

  modules.close(state);
  lua_close(state);
  modules.unload();
}

#ifdef SINEN_TEST_NATIVEFS_PATH
TEST(NativeModuleTests, NativeFsPluginReadsAbsolutePaths) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);
  luaL_openlibs(state);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  const std::filesystem::path pluginPath = SINEN_TEST_NATIVEFS_PATH;
  const ScopedCurrentDirectory currentDirectory(pluginPath.parent_path());

  ASSERT_TRUE(modules.loadPlugin(state, "nativefs")) << modules.lastError();
  lua_getglobal(state, "nativefs");
  ASSERT_TRUE(lua_istable(state, -1));
  lua_getfield(state, -1, "readText");
  ASSERT_TRUE(lua_isfunction(state, -1));

  const std::filesystem::path sourceFile =
      std::filesystem::path(SINEN_TEST_SOURCE_DIR) / "CMakeLists.txt";
  const auto encodedSourceFile = sourceFile.u8string();
  lua_pushlstring(state,
                  reinterpret_cast<const char *>(encodedSourceFile.data()),
                  encodedSourceFile.size());
  ASSERT_EQ(lua_pcall(state, 1, 2, 0), LUA_OK)
      << (lua_tostring(state, -1) ? lua_tostring(state, -1)
                                  : "unknown nativefs error");
  ASSERT_TRUE(lua_isstring(state, -2));
  EXPECT_EQ(lua_type(state, -1), LUA_TNIL);
  size_t contentsSize = 0;
  const char *contents = lua_tolstring(state, -2, &contentsSize);
  EXPECT_NE(
      std::string_view(contents, contentsSize).find("SINEN_PLUGIN_NATIVEFS"),
      std::string_view::npos);

  lua_settop(state, 0);
  modules.close(state);
  lua_close(state);
  modules.unload();
}
#endif

TEST(NativeModuleTests, KeepsPartiallyRegisteredModuleAliveAfterOpenError) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  EXPECT_FALSE(modules.load(state, SINEN_TEST_FAILING_NATIVE_MODULE_PATH));
  EXPECT_NE(modules.lastError().find("intentional native module"),
            std::string::npos);

  lua_getglobal(state, "native_module_failure_callback");
  ASSERT_TRUE(lua_isfunction(state, -1));
  ASSERT_EQ(lua_pcall(state, 0, 1, 0), LUA_OK)
      << (lua_tostring(state, -1) ? lua_tostring(state, -1)
                                  : "unknown native module error");
  EXPECT_EQ(lua_tointeger(state, -1), 7);

  lua_close(state);
  modules.unload();
}

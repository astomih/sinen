#include <lua.h>
#include <lualib.h>

#include <gtest/gtest.h>

#include "script/native_module.hpp"

#include <string>

#ifndef SINEN_TEST_NATIVE_MODULE_PATH
#error SINEN_TEST_NATIVE_MODULE_PATH must name the test module
#endif
#ifndef SINEN_TEST_FAILING_NATIVE_MODULE_PATH
#error SINEN_TEST_FAILING_NATIVE_MODULE_PATH must name the failing test module
#endif

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

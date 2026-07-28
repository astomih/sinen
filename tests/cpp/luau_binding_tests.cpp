#include <lua.h>
#include <lualib.h>

#include <LuaBridge/LuaBridge.h>
#include <Luau/Compiler.h>

#include <gtest/gtest.h>

#include "script/bindings/binding.hpp"

#include <string>

namespace {
class Counter {
public:
  explicit Counter(int value) : value(value) {}

  int add(int amount) {
    value += amount;
    return value;
  }

  int value;
};

int sum(int lhs, int rhs) { return lhs + rhs; }

int multiply(lua_State *state) {
  const int lhs = static_cast<int>(luaL_checkinteger(state, 1));
  const int rhs = static_cast<int>(luaL_checkinteger(state, 2));
  lua_pushinteger(state, lhs * rhs);
  return 1;
}

struct LuaState {
  LuaState() : state(luaL_newstate()) {
    if (state) {
      luaL_openlibs(state);
      luabridge::registerMainThread(state);
    }
  }

  ~LuaState() {
    if (state) {
      lua_close(state);
    }
  }

  lua_State *state;
};

int loadSource(lua_State *state, const char *source) {
  std::string bytecode = Luau::compile(source);
  return luau_load(state, "luau_binding_tests", bytecode.data(),
                   bytecode.size(), 0);
}
} // namespace

TEST(LuauBindingTests, BindsFunctionsClassesAndProperties) {
  LuaState lua;
  ASSERT_NE(lua.state, nullptr);

  luabridge::getGlobalNamespace(lua.state)
      .beginNamespace("test")
      .addFunction("sum", &sum)
      .beginClass<Counter>("Counter")
      .addConstructor<void(int)>()
      .addProperty("value", &Counter::value, &Counter::value)
      .addFunction("add", &Counter::add)
      .endClass()
      .endNamespace();

  constexpr const char *source = R"(
local counter = test.Counter(5)
counter.value = 8
return test.sum(2, 3), counter:add(4), counter.value
)";

  ASSERT_EQ(loadSource(lua.state, source), LUA_OK)
      << (lua_tostring(lua.state, -1) ? lua_tostring(lua.state, -1)
                                      : "unknown Luau load error");
  ASSERT_EQ(lua_pcall(lua.state, 0, 3, 0), LUA_OK)
      << (lua_tostring(lua.state, -1) ? lua_tostring(lua.state, -1)
                                      : "unknown Luau runtime error");

  EXPECT_EQ(lua_tointeger(lua.state, -3), 5);
  EXPECT_EQ(lua_tointeger(lua.state, -2), 12);
  EXPECT_EQ(lua_tointeger(lua.state, -1), 12);
}

TEST(LuauBindingTests, SinenBindingRegistersFunctionsAndValues) {
  LuaState lua;
  ASSERT_NE(lua.state, nullptr);

  lua_newtable(lua.state);
  sinen::Binding::registerInteger(lua.state, "answer", 42);
  sinen::Binding::registerFunction(lua.state, "multiply", multiply);
  lua_setglobal(lua.state, "bound");

  constexpr const char *source = "return bound.multiply(bound.answer, 2)";
  ASSERT_EQ(loadSource(lua.state, source), LUA_OK)
      << (lua_tostring(lua.state, -1) ? lua_tostring(lua.state, -1)
                                      : "unknown Luau load error");
  ASSERT_EQ(lua_pcall(lua.state, 0, 1, 0), LUA_OK)
      << (lua_tostring(lua.state, -1) ? lua_tostring(lua.state, -1)
                                      : "unknown Luau runtime error");

  EXPECT_EQ(lua_tointeger(lua.state, -1), 84);
}

#include <lua.h>
#include <lualib.h>

#include <Luau/Compiler.h>

#include <gtest/gtest.h>

#include "script/bindings/binding.hpp"

#include <new>
#include <string>
#include <string_view>

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

constexpr const char *counterMetatableName = "test.Counter";

Counter &checkCounter(lua_State *state, int index) {
  return *static_cast<Counter *>(
      luaL_checkudata(state, index, counterMetatableName));
}

int counterNew(lua_State *state) {
  const int value = static_cast<int>(luaL_checkinteger(state, 1));
  void *storage = lua_newuserdata(state, sizeof(Counter));
  new (storage) Counter(value);
  luaL_getmetatable(state, counterMetatableName);
  lua_setmetatable(state, -2);
  return 1;
}

int counterIndex(lua_State *state) {
  Counter &counter = checkCounter(state, 1);
  const char *key = luaL_checkstring(state, 2);
  if (std::string_view(key) == "value") {
    lua_pushinteger(state, counter.value);
    return 1;
  }

  luaL_getmetatable(state, counterMetatableName);
  lua_getfield(state, -1, key);
  return 1;
}

int counterNewIndex(lua_State *state) {
  Counter &counter = checkCounter(state, 1);
  const char *key = luaL_checkstring(state, 2);
  if (std::string_view(key) != "value") {
    luaL_error(state, "unknown Counter property '%s'", key);
    return 0;
  }
  counter.value = static_cast<int>(luaL_checkinteger(state, 3));
  return 0;
}

int counterAdd(lua_State *state) {
  Counter &counter = checkCounter(state, 1);
  const int amount = static_cast<int>(luaL_checkinteger(state, 2));
  lua_pushinteger(state, counter.add(amount));
  return 1;
}

int lSum(lua_State *state) {
  const int lhs = static_cast<int>(luaL_checkinteger(state, 1));
  const int rhs = static_cast<int>(luaL_checkinteger(state, 2));
  lua_pushinteger(state, sum(lhs, rhs));
  return 1;
}

void registerTestBindings(lua_State *state) {
  luaL_newmetatable(state, counterMetatableName);
  lua_pushcfunction(state, counterIndex, "test.Counter.__index");
  lua_setfield(state, -2, "__index");
  lua_pushcfunction(state, counterNewIndex, "test.Counter.__newindex");
  lua_setfield(state, -2, "__newindex");
  lua_pushcfunction(state, counterAdd, "test.Counter.add");
  lua_setfield(state, -2, "add");
  lua_pop(state, 1);

  lua_newtable(state);
  lua_pushcfunction(state, lSum, "test.sum");
  lua_setfield(state, -2, "sum");

  lua_pushcfunction(state, counterNew, "test.Counter.new");
  lua_setfield(state, -2, "Counter");
  lua_setglobal(state, "test");
}

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

  registerTestBindings(lua.state);

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

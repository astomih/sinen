#include <Luau/Compiler.h>
#include <lua.h>
#include <lualib.h>

#include <gtest/gtest.h>

#include <gpu/shader/shader_compiler_plugin.h>
#include <gpu/shader/shader_format.hpp>
#include <gpu/shader/shader_stage.hpp>
#include <script/sinen_lua_module.h>

#include "script/native_module.hpp"
#include "script/require_alias.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
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

  // Exercise the header-only binder across an actual DLL boundary, through
  // the same plugin registry/require path used by application scripts.
  lua_pushcfunction(
      state,
      +[](lua_State *L) -> int {
        luaL_error(L, "unexpected fallback require");
      },
      "fallback require");
  lua_setglobal(state, "require");
  sinen::installSinenRequireAlias(state);
  const auto bytecode = Luau::compile(R"(
    local native = require('@sinen/plugin/native_module_fixture')
    local c = native.Counter(5)
    c.value = 8
    assert(c:add(4) == 12 and native.answer() == 42)
    assert(not pcall(function() c.value = 'invalid' end))
    -- Keep an instance alive until lua_close, after module close is called.
    native_instance = c
  )");
  ASSERT_EQ(
      luau_load(state, "native_binding", bytecode.data(), bytecode.size(), 0),
      LUA_OK);
  ASSERT_EQ(lua_pcall(state, 0, 0, 0), LUA_OK)
      << (lua_tostring(state, -1) ? lua_tostring(state, -1)
                                  : "unknown binding error");

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

#ifdef SINEN_TEST_LUAU_TO_CPP_DIR
TEST(NativeModuleTests, LuauToCppPluginConvertsSourceAndReportsErrors) {
  auto &modules = sinen::nativeModuleManager();
  const auto closeState = [&](lua_State *state) {
    modules.close(state);
    lua_close(state);
    modules.unload();
  };
  const std::unique_ptr<lua_State, decltype(closeState)> owner(luaL_newstate(),
                                                               closeState);
  lua_State *state = owner.get();
  ASSERT_NE(state, nullptr);
  luaL_openlibs(state);

  const ScopedCurrentDirectory currentDirectory(SINEN_TEST_LUAU_TO_CPP_DIR);
  ASSERT_TRUE(modules.loadPlugin(state, "luau_to_cpp")) << modules.lastError();
  lua_pushcfunction(
      state,
      +[](lua_State *L) -> int {
        luaL_error(L, "unexpected fallback require");
      },
      "fallback require");
  lua_setglobal(state, "require");
  sinen::installSinenRequireAlias(state);

  const auto script = Luau::compile(R"luau(
    local converter = require("@sinen/plugin/luau_to_cpp")
    local cpp, err = converter.convert("local function f(x: number): number return x end return f")
    assert(cpp and not err)
    assert(string.find(cpp, "inline double run(double v0)", 1, true))

    local bad, diagnostic = converter.convert("local function f(x: number): number\n  return print(x)\nend\nreturn f", "bad.luau")
    assert(not bad and string.find(diagnostic, "bad.luau:2:10:", 1, true))

    local invalid, syntaxError = converter.convert("local function", "syntax.luau")
    assert(not invalid and string.find(syntaxError, "syntax.luau:1:", 1, true))
  )luau");
  ASSERT_EQ(luau_load(state, "convert", script.data(), script.size(), 0), LUA_OK);
  ASSERT_EQ(lua_pcall(state, 0, 0, 0), LUA_OK) << lua_tostring(state, -1);
}
#endif

#ifdef SINEN_TEST_FPS_DIR
TEST(NativeModuleTests, FpsCollisionPluginMatchesLuau) {
  auto &modules = sinen::nativeModuleManager();
  const auto closeState = [&](lua_State *state) {
    modules.close(state);
    lua_close(state);
    modules.unload();
  };
  const std::unique_ptr<lua_State, decltype(closeState)> owner(luaL_newstate(),
                                                               closeState);
  lua_State *state = owner.get();
  ASSERT_NE(state, nullptr);
  luaL_openlibs(state);

  const ScopedCurrentDirectory currentDirectory(SINEN_TEST_FPS_DIR);
  ASSERT_TRUE(modules.loadPlugin(state, "fps_collision"))
      << modules.lastError();
  lua_pushcfunction(
      state,
      +[](lua_State *L) -> int {
        luaL_error(L, "unexpected fallback require");
      },
      "fallback require");
  lua_setglobal(state, "require");
  sinen::installSinenRequireAlias(state);

  std::ifstream input("slab.luau", std::ios::binary);
  ASSERT_TRUE(input);
  const std::string source{std::istreambuf_iterator<char>(input),
                           std::istreambuf_iterator<char>()};
  const auto fallback = Luau::compile(source);
  ASSERT_EQ(luau_load(state, "slab.luau", fallback.data(), fallback.size(), 0),
            LUA_OK);
  ASSERT_EQ(lua_pcall(state, 0, 1, 0), LUA_OK);
  lua_setglobal(state, "luauSlab");

  const auto compare = Luau::compile(R"(
    local nativeSlab = require("@sinen/plugin/fps_collision").slab
    local function same(a, b)
      return (a == b and (a ~= 0 or 1 / a == 1 / b)) or (a ~= a and b ~= b)
    end
    for _, origin in {-2, -1, -0.0, 0, 1, 2, math.huge, 0 / 0} do
      for _, direction in {-2, -1e-7, -1e-8, -0.0, 0, 1e-8, 1e-7, 2, math.huge, 0 / 0} do
        for _, bound in {0, 1} do
          local a, b = luauSlab(origin, direction, -bound, bound)
          local c, d = nativeSlab(origin, direction, -bound, bound)
          assert(same(a, c) and same(b, d), "slab result differs")
        end
      end
    end
  )");
  ASSERT_EQ(luau_load(state, "compare slab", compare.data(), compare.size(), 0),
            LUA_OK);
  ASSERT_EQ(lua_pcall(state, 0, 0, 0), LUA_OK) << lua_tostring(state, -1);
}
#endif

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
  lua_getfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "nativefs");
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

  lua_getglobal(state, "nativefs");
  EXPECT_TRUE(lua_isnil(state, -1));

  lua_settop(state, 0);
  modules.close(state);
  lua_getfield(state, LUA_REGISTRYINDEX,
               SINEN_LUA_PLUGIN_REGISTRY_PREFIX "nativefs");
  EXPECT_TRUE(lua_isnil(state, -1));
  lua_close(state);
  modules.unload();
}
#endif

#ifdef SINEN_TEST_SHADER_COMPILER_PLUGIN_PATH
TEST(NativeModuleTests, ShaderCompilerPluginCompilesSlangSource) {
  lua_State *state = luaL_newstate();
  ASSERT_NE(state, nullptr);

  sinen::NativeModuleManager &modules = sinen::nativeModuleManager();
  const std::filesystem::path pluginPath =
      SINEN_TEST_SHADER_COMPILER_PLUGIN_PATH;
  const ScopedCurrentDirectory currentDirectory(pluginPath.parent_path());
  ASSERT_TRUE(modules.loadPlugin(state, "shader_compiler"))
      << modules.lastError();

  void *symbol =
      modules.lastPluginSymbol(SINEN_SHADER_COMPILER_PLUGIN_API_SYMBOL);
  ASSERT_NE(symbol, nullptr);
  sinen_shader_compiler_get_api_fn getApi = nullptr;
  static_assert(sizeof(getApi) == sizeof(symbol));
  std::memcpy(&getApi, &symbol, sizeof(getApi));
  ASSERT_NE(getApi, nullptr);
  const sinen_shader_compiler_api *api = getApi();
  ASSERT_NE(api, nullptr);
  ASSERT_EQ(api->api_version, SINEN_SHADER_COMPILER_PLUGIN_API_VERSION);

  constexpr std::string_view source = R"(
[shader("vertex")]
float4 VSMain(float3 position : POSITION) : SV_Position
{
    return float4(position, 1.0);
}
)";
  sinen_shader_compiler_result result{};
  result.struct_size = sizeof(result);
  ASSERT_TRUE(api->compile(
      "test_shader", std::strlen("test_shader"), "test_shader.slang",
      std::strlen("test_shader.slang"), source.data(), source.size(),
      static_cast<uint32_t>(sinen::ShaderStage::Vertex),
      static_cast<uint32_t>(sinen::ShaderFormat::SPIRV), &result));
  EXPECT_NE(result.code, nullptr);
  EXPECT_GT(result.code_size, 0u);
  ASSERT_NE(result.release, nullptr);
  result.release(&result);

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

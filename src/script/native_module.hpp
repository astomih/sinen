#ifndef SINEN_NATIVE_MODULE_HPP
#define SINEN_NATIVE_MODULE_HPP

#include <string>
#include <string_view>

struct lua_State;

namespace sinen {

class NativeModuleManager {
public:
  bool load(lua_State *state, std::string_view path);
  void close(lua_State *state);
  void unload();

  const std::string &lastError() const;

private:
  struct Impl;
  static Impl &impl();
};

NativeModuleManager &nativeModuleManager();

} // namespace sinen

#endif // SINEN_NATIVE_MODULE_HPP

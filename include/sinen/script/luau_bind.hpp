#ifndef SINEN_LUAU_BIND_HPP
#define SINEN_LUAU_BIND_HPP

// Original, header-only C++ bindings for the Luau VM. No Lua binding
// dependency.
#include <lua.h>
#include <lualib.h>

#include "sinen_lua_module.h"

#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if LUA_USE_LONGJMP
#error                                                                         \
    "sinen::luau requires Luau with C++ exception unwinding (LUA_USE_LONGJMP=0)"
#endif

namespace sinen::luau {

class binding_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

// Specialize stack<T> with matches/get/push to add a one-value conversion.
// matches must preserve the stack; push must add exactly one value.
template <class T, class Enable = void> struct stack;
template <class T> class class_builder;
class table_proxy;

namespace detail {
class stack_guard {
public:
  explicit stack_guard(lua_State *L) : L(L), top(lua_gettop(L)) {}
  ~stack_guard() { lua_settop(L, top); }
  stack_guard(const stack_guard &) = delete;
  stack_guard &operator=(const stack_guard &) = delete;

private:
  lua_State *L;
  int top;
};

inline void reserve(lua_State *L, int count = 4) {
  if (!lua_checkstack(L, count))
    throw binding_error("Luau stack capacity exceeded");
}

inline void create_table(lua_State *L, int array_size = 0) {
  // Native plugins currently link Luau statically. An empty hash part would
  // point at that DLL's luaH_dummynode, which another VM binary cannot free.
  // Use an allocated hash part even for empty tables and array-only results.
  lua_createtable(L, array_size, 1);
}

inline void require(bool condition, const char *message) {
  if (!condition)
    throw binding_error(message);
}

// Translate exceptions only after C++ temporaries and exception handlers
// unwind. This also preserves the message of errors raised by the Luau C API.
template <class F> int protect(lua_State *L, F &&function) {
  try {
    reserve(L);
    return function();
  } catch (const std::exception &error) {
    lua_pushstring(L, error.what());
  } catch (...) {
    lua_pushstring(L, "unknown C++ exception in Luau binding");
  }
  lua_error(L);
}

template <class T> inline char type_key;

template <class T> struct object {
  T *pointer = nullptr;
  bool owned = false;
  bool readonly = false;
};

// The pointer key is a binary-local cache. Shared registrations live under a
// stable string key in this VM and are explicitly imported by other binaries.
inline std::string shared_type_key(std::string_view name) {
  require(!name.empty() && name.find('\0') == std::string_view::npos,
          "shared C++ type name must not be empty or contain a null byte");
  return "sinen.luau.shared_type." + std::string(name);
}

inline void set_abi_field(lua_State *L, const char *name, size_t value) {
  lua_pushnumber(L, static_cast<double>(value));
  lua_rawsetfield(L, -2, name);
}

inline bool abi_field_matches(lua_State *L, const char *name, size_t value) {
  lua_rawgetfield(L, -1, name);
  const bool matches = lua_type(L, -1) == LUA_TNUMBER &&
                       lua_tonumber(L, -1) == static_cast<double>(value);
  lua_pop(L, 1);
  return matches;
}

template <class T>
void check_shared_abi(lua_State *L, std::string_view name, unsigned version) {
  const bool matches =
      abi_field_matches(L, "binding_abi", 1) &&
      abi_field_matches(L, "version", version) &&
      abi_field_matches(L, "size", sizeof(T)) &&
      abi_field_matches(L, "alignment", alignof(T)) &&
      abi_field_matches(L, "box_size", sizeof(object<T>)) &&
      abi_field_matches(L, "box_alignment", alignof(object<T>));
  if (!matches)
    throw binding_error("shared C++ type ABI mismatch: " + std::string(name));
}

template <class T> bool is_object(lua_State *L, int index) {
  stack_guard guard(L);
  reserve(L);
  if (lua_type(L, index) != LUA_TUSERDATA || !lua_getmetatable(L, index))
    return false;
  lua_rawgetp(L, LUA_REGISTRYINDEX, &type_key<T>);
  return lua_istable(L, -1) && lua_rawequal(L, -1, -2);
}

template <class T> object<T> &get_object(lua_State *L, int index) {
  require(is_object<T>(L, index), "expected registered C++ class");
  auto &result = *static_cast<object<T> *>(lua_touserdata(L, index));
  require(result.pointer != nullptr, "C++ object is not initialized");
  return result;
}

template <class T> void destroy_object(lua_State *, void *data) noexcept {
  auto &value = *static_cast<object<T> *>(data);
  if (value.owned)
    delete value.pointer;
  value.~object<T>();
}

template <class T, class... A> void push_owned(lua_State *L, A &&...args) {
  reserve(L);
  lua_rawgetp(L, LUA_REGISTRYINDEX, &type_key<T>);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    throw binding_error("register the C++ class before returning an object");
  }
  // Allocate the box first. A throwing constructor leaves a harmless empty box.
  auto *box = new (lua_newuserdatadtor(L, sizeof(object<T>), destroy_object<T>))
      object<T>();
  lua_pushvalue(L, -2);
  lua_setmetatable(L, -2);
  lua_remove(L, -2);
  box->pointer = new T(std::forward<A>(args)...);
  box->owned = true;
}

template <class T> void push_borrowed(lua_State *L, T &value) {
  using U = std::remove_const_t<T>;
  reserve(L);
  lua_rawgetp(L, LUA_REGISTRYINDEX, &type_key<U>);
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    throw binding_error("register the C++ class before borrowing an object");
  }
  new (lua_newuserdatadtor(L, sizeof(object<U>), destroy_object<U>))
      object<U>{const_cast<U *>(&value), false, std::is_const_v<T>};
  lua_pushvalue(L, -2);
  lua_setmetatable(L, -2);
  lua_remove(L, -2);
}
} // namespace detail

// All participants must use the same C++ definition, compiler ABI and runtime.
// Size/alignment/version checks catch common mistakes, not every ABI mismatch.
template <class T>
void export_type(lua_State *L, std::string_view name, unsigned version = 1) {
  static_assert(std::is_class_v<T>,
                "export_type requires a C++ class or struct");
  detail::stack_guard guard(L);
  detail::reserve(L, 6);
  const auto key = detail::shared_type_key(name);
  lua_rawgetp(L, LUA_REGISTRYINDEX, &detail::type_key<T>);
  detail::require(lua_istable(L, -1),
                  "register the C++ class before exporting it");
  const int metatable = lua_absindex(L, -1);
  lua_rawgetfield(L, LUA_REGISTRYINDEX, key.c_str());
  if (!lua_isnil(L, -1)) {
    detail::require(lua_istable(L, -1), "invalid shared C++ type descriptor");
    detail::check_shared_abi<T>(L, name, version);
    lua_rawgetfield(L, -1, "metatable");
    if (!lua_rawequal(L, metatable, -1))
      throw binding_error("shared C++ type name already exported: " +
                          std::string(name));
    return; // Re-exporting the same registration is harmless.
  }
  lua_pop(L, 1);
  lua_createtable(L, 0, 7);
  detail::set_abi_field(L, "binding_abi", 1);
  detail::set_abi_field(L, "version", version);
  detail::set_abi_field(L, "size", sizeof(T));
  detail::set_abi_field(L, "alignment", alignof(T));
  detail::set_abi_field(L, "box_size", sizeof(detail::object<T>));
  detail::set_abi_field(L, "box_alignment", alignof(detail::object<T>));
  lua_pushvalue(L, metatable);
  lua_rawsetfield(L, -2, "metatable");
  lua_setreadonly(L, -1, true);
  lua_rawsetfield(L, LUA_REGISTRYINDEX, key.c_str());
}

template <class T>
void import_type(lua_State *L, std::string_view name, unsigned version = 1) {
  static_assert(std::is_class_v<T>,
                "import_type requires a C++ class or struct");
  detail::stack_guard guard(L);
  detail::reserve(L, 6);
  const auto key = detail::shared_type_key(name);
  lua_rawgetfield(L, LUA_REGISTRYINDEX, key.c_str());
  if (lua_isnil(L, -1))
    throw binding_error(
        "shared C++ type is not exported; load its provider first: " +
        std::string(name));
  detail::require(lua_istable(L, -1), "invalid shared C++ type descriptor");
  detail::check_shared_abi<T>(L, name, version);
  lua_rawgetfield(L, -1, "metatable");
  detail::require(lua_istable(L, -1), "invalid shared C++ type metatable");
  lua_rawgetp(L, LUA_REGISTRYINDEX, &detail::type_key<T>);
  if (!lua_isnil(L, -1) && !lua_rawequal(L, -1, -2))
    throw binding_error("C++ type already bound to a different registration: " +
                        std::string(name));
  lua_pop(L, 1);
  lua_rawsetp(L, LUA_REGISTRYINDEX, &detail::type_key<T>);
}

// Class values are owned by Luau. Returning T&/const T& copies the value;
// use std::ref/std::cref explicitly for a borrowed object.
template <class T, class Enable> struct stack {
  static_assert(std::is_class_v<T>,
                "unsupported Luau conversion; specialize stack<T>");
  static bool matches(lua_State *L, int index) {
    return detail::is_object<T>(L, index);
  }
  static T &get(lua_State *L, int index) {
    return *detail::get_object<T>(L, index).pointer;
  }
  template <class U> static void push(lua_State *L, U &&value) {
    detail::push_owned<T>(L, std::forward<U>(value));
  }
};

template <> struct stack<bool> {
  static bool matches(lua_State *L, int index) {
    return lua_type(L, index) == LUA_TBOOLEAN;
  }
  static bool get(lua_State *L, int index) {
    detail::require(matches(L, index), "expected boolean");
    return lua_toboolean(L, index) != 0;
  }
  static void push(lua_State *L, bool value) { lua_pushboolean(L, value); }
};

template <class T>
  requires((std::integral<T> || std::floating_point<T>) &&
           !std::same_as<T, bool>)
struct stack<T> {
  static bool matches(lua_State *L, int index) {
    if (lua_type(L, index) != LUA_TNUMBER)
      return false;
    const double value = lua_tonumber(L, index);
    if constexpr (std::is_integral_v<T>) {
      // Luau numbers are doubles. Never silently truncate or accept unsafe
      // integers.
      return std::isfinite(value) && std::trunc(value) == value &&
             value >= -9007199254740991.0 && value <= 9007199254740991.0 &&
             static_cast<long double>(value) >=
                 std::numeric_limits<T>::lowest() &&
             static_cast<long double>(value) <= std::numeric_limits<T>::max();
    } else {
      return !std::isfinite(value) ||
             (value >= -std::numeric_limits<T>::max() &&
              value <= std::numeric_limits<T>::max());
    }
  }
  static T get(lua_State *L, int index) {
    detail::require(matches(L, index), "expected number in C++ type range");
    return static_cast<T>(lua_tonumber(L, index));
  }
  static void push(lua_State *L, T value) {
    if constexpr (std::is_integral_v<T>) {
      const long double number = static_cast<long double>(value);
      detail::require(number >= -9007199254740991.0L &&
                          number <= 9007199254740991.0L,
                      "integer cannot be represented exactly by Luau");
    }
    lua_pushnumber(L, static_cast<double>(value));
  }
};

template <class T>
  requires std::is_enum_v<T>
struct stack<T> {
  using base = stack<std::underlying_type_t<T>>;
  static bool matches(lua_State *L, int index) {
    return base::matches(L, index);
  }
  static T get(lua_State *L, int index) {
    return static_cast<T>(base::get(L, index));
  }
  static void push(lua_State *L, T value) {
    base::push(L, std::to_underlying(value));
  }
};

template <> struct stack<std::string_view> {
  static bool matches(lua_State *L, int index) {
    return lua_type(L, index) == LUA_TSTRING;
  }
  static std::string_view get(lua_State *L, int index) {
    detail::require(matches(L, index), "expected string");
    size_t length = 0;
    const char *value = lua_tolstring(L, index, &length);
    return {value, length};
  }
  static void push(lua_State *L, std::string_view value) {
    lua_pushlstring(L, value.empty() ? "" : value.data(), value.size());
  }
};

template <> struct stack<std::string> : stack<std::string_view> {
  static std::string get(lua_State *L, int index) {
    return std::string(stack<std::string_view>::get(L, index));
  }
};

template <> struct stack<const char *> : stack<std::string_view> {
  static const char *get(lua_State *L, int index) {
    detail::require(matches(L, index), "expected string");
    return lua_tostring(L, index);
  }
  static void push(lua_State *L, const char *value) {
    if (value)
      lua_pushstring(L, value);
    else
      lua_pushnil(L);
  }
};

template <> struct stack<std::nullptr_t> {
  static bool matches(lua_State *L, int index) {
    return lua_isnoneornil(L, index);
  }
  static std::nullptr_t get(lua_State *L, int index) {
    detail::require(matches(L, index), "expected nil");
    return nullptr;
  }
  static void push(lua_State *L, std::nullptr_t) { lua_pushnil(L); }
};

template <class T> struct stack<std::optional<T>> {
  static bool matches(lua_State *L, int index) {
    return lua_isnoneornil(L, index) || stack<T>::matches(L, index);
  }
  static std::optional<T> get(lua_State *L, int index) {
    if (lua_isnoneornil(L, index))
      return std::nullopt;
    return stack<T>::get(L, index);
  }
  static void push(lua_State *L, const std::optional<T> &value) {
    if (value)
      stack<T>::push(L, *value);
    else
      lua_pushnil(L);
  }
};

template <class T> struct stack<std::reference_wrapper<T>> {
  static bool matches(lua_State *L, int index) {
    using U = std::remove_const_t<T>;
    return detail::is_object<U>(L, index) &&
           (std::is_const_v<T> || !detail::get_object<U>(L, index).readonly);
  }
  static std::reference_wrapper<T> get(lua_State *L, int index) {
    detail::require(matches(L, index),
                    "expected C++ object with matching constness");
    return std::ref(
        *detail::get_object<std::remove_const_t<T>>(L, index).pointer);
  }
  static void push(lua_State *L, std::reference_wrapper<T> value) {
    detail::push_borrowed(L, value.get());
  }
};

template <class T> struct stack<std::vector<T>> {
  static bool matches(lua_State *L, int index) {
    detail::stack_guard guard(L);
    detail::reserve(L);
    if (!lua_istable(L, index))
      return false;
    index = lua_absindex(L, index);
    const int length = lua_objlen(L, index);
    int count = 0;
    lua_pushnil(L);
    while (lua_next(L, index)) {
      if (!stack<int>::matches(L, -2))
        return false;
      const int key = stack<int>::get(L, -2);
      if (key < 1 || key > length || !stack<T>::matches(L, -1))
        return false;
      ++count;
      lua_pop(L, 1);
    }
    return count == length;
  }
  static std::vector<T> get(lua_State *L, int index) {
    detail::require(matches(L, index),
                    "expected dense array with matching element types");
    detail::stack_guard guard(L);
    index = lua_absindex(L, index);
    const int length = lua_objlen(L, index);
    std::vector<T> result;
    result.reserve(length);
    for (int i = 1; i <= length; ++i) {
      lua_rawgeti(L, index, i);
      result.emplace_back(stack<T>::get(L, -1));
      lua_pop(L, 1);
    }
    return result;
  }
  static void push(lua_State *L, const std::vector<T> &values) {
    detail::require(values.size() <=
                        static_cast<size_t>(std::numeric_limits<int>::max()),
                    "array is too large for Luau");
    detail::reserve(L);
    detail::create_table(L, static_cast<int>(values.size()));
    for (size_t i = 0; i < values.size(); ++i) {
      stack<T>::push(L, values[i]);
      lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
  }
};

namespace detail {
template <class T> using plain = std::remove_cvref_t<T>;

template <class T> bool argument_matches(lua_State *L, int index) {
  if (!stack<plain<T>>::matches(L, index))
    return false;
  if constexpr (std::is_lvalue_reference_v<T> &&
                !std::is_const_v<std::remove_reference_t<T>>) {
    static_assert(std::is_class_v<plain<T>>,
                  "mutable scalar references cannot refer to Luau values");
    // Only class conversions whose get() actually returns a reference may be
    // mutable.
    static_assert(
        std::is_lvalue_reference_v<decltype(stack<plain<T>>::get(L, index))>,
        "mutable references require a userdata conversion");
    return !get_object<plain<T>>(L, index).readonly;
  }
  return true;
}

template <class T> decltype(auto) read_argument(lua_State *L, int index) {
  using U = plain<T>;
  require(argument_matches<T>(L, index), "argument type or constness mismatch");
  if constexpr (std::is_lvalue_reference_v<T> &&
                std::is_lvalue_reference_v<decltype(stack<U>::get(L, index))>) {
    return static_cast<T>(stack<U>::get(L, index));
  } else {
    return U(stack<U>::get(L, index));
  }
}

template <class R, class... A> struct signature_base {
  using result = R;
  using args = std::tuple<A...>;
};
template <class F, class = void> struct signature {};
template <class R, class... A>
struct signature<R (*)(A...)> : signature_base<R, A...> {};
template <class R, class... A>
struct signature<R (*)(A...) noexcept> : signature_base<R, A...> {};

#define SINEN_LUAU_MEMBER_SIGNATURE(QUALIFIERS, SELF)                          \
  template <class C, class R, class... A>                                      \
  struct signature<R (C::*)(A...) QUALIFIERS>                                  \
      : signature_base<R, SELF, A...> {};
SINEN_LUAU_MEMBER_SIGNATURE(, C &)
SINEN_LUAU_MEMBER_SIGNATURE(const, const C &)
SINEN_LUAU_MEMBER_SIGNATURE(noexcept, C &)
SINEN_LUAU_MEMBER_SIGNATURE(const noexcept, const C &)
SINEN_LUAU_MEMBER_SIGNATURE(&, C &)
SINEN_LUAU_MEMBER_SIGNATURE(const &, const C &)
SINEN_LUAU_MEMBER_SIGNATURE(& noexcept, C &)
SINEN_LUAU_MEMBER_SIGNATURE(const & noexcept, const C &)
#undef SINEN_LUAU_MEMBER_SIGNATURE

template <class Tuple> struct drop_self;
template <class C, class... A> struct drop_self<std::tuple<C, A...>> {
  using type = std::tuple<A...>;
};
template <class F>
  requires requires {
    typename signature<decltype(&F::operator())>::result;
    typename signature<decltype(&F::operator())>::args;
  }
struct signature<F, std::void_t<decltype(&F::operator())>> {
  using pointer = decltype(&F::operator());
  using call = signature<pointer>;
  using result = typename call::result;
  using args = typename std::conditional_t<
      std::is_member_function_pointer_v<pointer>,
      drop_self<typename call::args>,
      std::type_identity<typename call::args>>::type;
};

template <class F>
concept callable = requires {
  typename signature<std::decay_t<F>>::result;
  typename signature<std::decay_t<F>>::args;
};

template <class T> struct is_tuple : std::false_type {};
template <class... T> struct is_tuple<std::tuple<T...>> : std::true_type {};

template <class R> int push_result(lua_State *L, R &&result) {
  if constexpr (is_tuple<plain<R>>::value) {
    constexpr size_t count = std::tuple_size_v<plain<R>>;
    reserve(L, static_cast<int>(count) + 4);
    std::apply(
        [L](auto &&...values) {
          (stack<std::decay_t<decltype(values)>>::push(
               L, std::forward<decltype(values)>(values)),
           ...);
        },
        std::forward<R>(result));
    return static_cast<int>(count);
  } else {
    stack<std::decay_t<R>>::push(L, std::forward<R>(result));
    return 1;
  }
}

template <class Args, size_t... I>
bool matches_arguments(lua_State *L, int first, std::index_sequence<I...>) {
  // Missing arguments are accepted only by conversions such as optional<T>.
  return lua_gettop(L) < first + static_cast<int>(sizeof...(I)) &&
         (argument_matches<std::tuple_element_t<I, Args>>(
              L, first + static_cast<int>(I)) &&
          ...);
}

template <class F> bool matches_callable(lua_State *L, int first = 1) {
  using args = typename signature<F>::args;
  return matches_arguments<args>(
      L, first, std::make_index_sequence<std::tuple_size_v<args>>{});
}

template <class F, size_t... I>
int invoke_callable(lua_State *L, F &function, int first,
                    std::index_sequence<I...>) {
  using traits = signature<F>;
  using args = typename traits::args;
  // Braced initialization reads left-to-right and keeps converted temporaries
  // alive.
  std::tuple<decltype(read_argument<std::tuple_element_t<I, args>>(L,
                                                                   first))...>
      values{read_argument<std::tuple_element_t<I, args>>(
          L, first + static_cast<int>(I))...};
  auto invoke = [&](auto &...value) -> decltype(auto) {
    return std::invoke(function,
                       std::forward<std::tuple_element_t<I, args>>(value)...);
  };
  if constexpr (std::is_void_v<typename traits::result>) {
    std::apply(invoke, values);
    return 0;
  } else {
    return push_result(L, std::apply(invoke, values));
  }
}

template <class F>
int invoke_callable(lua_State *L, F &function, int first = 1) {
  require(matches_callable<F>(L, first),
          "no matching binding signature (argument count, type or range)");
  return invoke_callable(L, function, first,
                         std::make_index_sequence<
                             std::tuple_size_v<typename signature<F>::args>>{});
}

template <class T> struct capture {
  T *pointer = nullptr;
};
template <class T> void destroy_capture(lua_State *, void *data) noexcept {
  auto *value = static_cast<capture<T> *>(data);
  delete value->pointer;
  value->~capture<T>();
}

template <class F, lua_CFunction Trampoline>
void push_closure(lua_State *L, F &&function, const char *name) {
  using U = std::decay_t<F>;
  reserve(L);
  auto *value =
      new (lua_newuserdatadtor(L, sizeof(capture<U>), destroy_capture<U>))
          capture<U>();
  value->pointer = new U(std::forward<F>(function));
  lua_pushcclosure(L, Trampoline, name, 1);
}

template <class F> int function_call(lua_State *L) {
  return protect(L, [&] {
    auto *value =
        static_cast<capture<F> *>(lua_touserdata(L, lua_upvalueindex(1)));
    return invoke_callable(L, *value->pointer);
  });
}

inline int raw_function_call(lua_State *L) {
  return protect(L, [&] {
    auto *value = static_cast<capture<lua_CFunction> *>(
        lua_touserdata(L, lua_upvalueindex(1)));
    return (*value->pointer)(L);
  });
}

// Encode the function in the trampoline type, without capture userdata.
template <auto Function> int static_function_call(lua_State *L) {
  return protect(L, [&] {
    auto function = Function;
    return invoke_callable(L, function);
  });
}

template <lua_CFunction Function> int static_raw_function_call(lua_State *L) {
  return protect(L, [&] { return Function(L); });
}

template <class F>
void push_function(lua_State *L, F &&function, const char *name) {
  push_closure<F, function_call<std::decay_t<F>>>(L, std::forward<F>(function),
                                                  name);
}
} // namespace detail

template <class... F> struct overload_set {
  std::tuple<F...> functions;
};
template <detail::callable... F>
  requires(sizeof...(F) > 0)
auto overload(F &&...functions) {
  return overload_set<std::decay_t<F>...>{{std::forward<F>(functions)...}};
}

namespace detail {
template <class T> inline constexpr bool is_overload = false;
template <class... F>
inline constexpr bool is_overload<overload_set<F...>> = true;
template <class F>
concept binding_function = callable<F> || is_overload<std::remove_cvref_t<F>>;

template <size_t I = 0, class... F>
int invoke_overload(lua_State *L, overload_set<F...> &set) {
  if constexpr (I == sizeof...(F)) {
    throw binding_error("no matching overload (argument count, type or range)");
  } else {
    using function = std::tuple_element_t<I, std::tuple<F...>>;
    if (matches_callable<function>(L))
      return invoke_callable(L, std::get<I>(set.functions));
    return invoke_overload<I + 1>(L, set);
  }
}
template <class... F> int overload_call(lua_State *L) {
  return protect(L, [&] {
    using set = overload_set<F...>;
    auto *value =
        static_cast<capture<set> *>(lua_touserdata(L, lua_upvalueindex(1)));
    return invoke_overload(L, *value->pointer);
  });
}
template <class... F>
void push_function(lua_State *L, overload_set<F...> set, const char *name) {
  push_closure<overload_set<F...>, overload_call<F...>>(L, std::move(set),
                                                        name);
}
} // namespace detail

// A registry reference, independent of stack positions. Must die before
// lua_close.
class table {
public:
  table(lua_State *L, int index) : L(L) {
    detail::require(lua_istable(L, index), "expected table");
    // Keep a hash entry while lua_ref grows the registry's array part, even
    // before open_libraries. Otherwise Luau can replace the hash with this
    // DLL's static empty-node sentinel during rehashing.
    detail::reserve(L);
    lua_pushboolean(L, true);
    lua_rawsetfield(L, LUA_REGISTRYINDEX, "sinen.luau.bind");
    reference = lua_ref(L, index);
  }
  table(const table &other) : L(other.L) {
    detail::stack_guard guard(L);
    other.push();
    reference = lua_ref(L, -1);
  }
  table(table &&other) noexcept
      : L(other.L), reference(std::exchange(other.reference, LUA_NOREF)) {}
  table &operator=(table other) noexcept {
    std::swap(L, other.L);
    std::swap(reference, other.reference);
    return *this;
  }
  ~table() {
    if (reference != LUA_NOREF)
      lua_unref(L, reference);
  }
  static table create(lua_State *L) {
    detail::stack_guard guard(L);
    detail::reserve(L);
    detail::create_table(L);
    return table(L, -1);
  }
  void push() const {
    detail::reserve(L);
    detail::require(reference != LUA_NOREF, "use of moved-from binding table");
    lua_getref(L, reference);
  }
  lua_State *state() const { return L; }

  template <class T>
  table &import_type(std::string_view name, unsigned version = 1) {
    luau::import_type<T>(L, name, version);
    return *this;
  }

  table create_table() const { return table::create(L); }
  table get_table(const char *name) const {
    detail::stack_guard guard(L);
    push();
    lua_getfield(L, -1, name);
    return table(L, -1);
  }
  table get_or_create_table(const char *name) {
    detail::stack_guard guard(L);
    push();
    lua_getfield(L, -1, name);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      detail::create_table(L);
      lua_pushvalue(L, -1);
      lua_setfield(L, -3, name);
    }
    return table(L, -1);
  }
  template <class T> T get(const char *name) const {
    detail::stack_guard guard(L);
    push();
    lua_getfield(L, -1, name);
    if constexpr (std::is_same_v<T, table>)
      return table(L, -1);
    else
      return detail::read_argument<T>(L, -1);
  }
  table_proxy operator[](const char *name) const;

  template <detail::binding_function F>
  table &function(const char *name, F &&function) {
    detail::stack_guard guard(L);
    push();
    detail::push_function(L, std::forward<F>(function), name);
    lua_setfield(L, -2, name);
    return *this;
  }
  template <auto Function>
    requires detail::callable<decltype(Function)>
  table &function(const char *name) {
    detail::stack_guard guard(L);
    push();
    lua_pushcfunction(L, detail::static_function_call<Function>, name);
    lua_setfield(L, -2, name);
    return *this;
  }
  // Escape hatch for int(lua_State*) functions that manage their own stack.
  // As with typed functions, C++ exceptions become Luau errors.
  table &raw_function(const char *name, lua_CFunction function) {
    detail::stack_guard guard(L);
    push();
    detail::push_closure<lua_CFunction, detail::raw_function_call>(
        L, std::move(function), name);
    lua_setfield(L, -2, name);
    return *this;
  }
  template <lua_CFunction Function> table &raw_function(const char *name) {
    static_assert(Function != nullptr, "raw callback must not be null");
    detail::stack_guard guard(L);
    push();
    lua_pushcfunction(L, detail::static_raw_function_call<Function>, name);
    lua_setfield(L, -2, name);
    return *this;
  }
  template <class T> table &value(const char *name, T &&value) {
    detail::stack_guard guard(L);
    push();
    stack<std::decay_t<T>>::push(L, std::forward<T>(value));
    lua_setfield(L, -2, name);
    return *this;
  }
  table &set_table(const char *name, const table &value) {
    detail::require(L == value.L,
                    "binding tables belong to different lua_State handles");
    detail::stack_guard guard(L);
    push();
    value.push();
    lua_setfield(L, -2, name);
    return *this;
  }
  table &publish(const char *global) {
    detail::stack_guard guard(L);
    push();
    lua_setglobal(L, global);
    return *this;
  }
  table &publish_plugin(std::string_view name) {
    detail::require(!name.empty() && name.find('\0') == std::string_view::npos,
                    "invalid plugin registry name");
    detail::require(
        name.find('/') == std::string_view::npos &&
            name.find('\\') == std::string_view::npos,
        "plugin registry name must not contain directory separators");
    const std::string key =
        std::string(SINEN_LUA_PLUGIN_REGISTRY_PREFIX) + std::string(name);
    detail::stack_guard guard(L);
    push();
    lua_setfield(L, LUA_REGISTRYINDEX, key.c_str());
    return *this;
  }
  template <class T> class_builder<T> class_(const char *name);

private:
  lua_State *L;
  int reference = LUA_NOREF;
};

// Holds a table reference, so a proxy obtained from a temporary table is safe.
class table_proxy {
public:
  table_proxy(table parent, const char *name)
      : parent(std::move(parent)), name(name) {}
  table_proxy(const table_proxy &) = default;
  template <class T> table_proxy &operator=(T &&value) {
    using U = std::decay_t<T>;
    if constexpr (std::is_same_v<U, table_proxy>) {
      return assign(value);
    } else if constexpr (std::is_same_v<U, table>) {
      parent.set_table(name.c_str(), value);
    } else if constexpr (detail::binding_function<U>) {
      parent.function(name.c_str(), std::forward<T>(value));
    } else {
      parent.value(name.c_str(), std::forward<T>(value));
    }
    return *this;
  }
  table_proxy &operator=(const table_proxy &other) { return assign(other); }
  template <class T> T get() const { return parent.get<T>(name.c_str()); }
  template <class T> operator T() const { return get<T>(); }

private:
  table_proxy &assign(const table_proxy &other) {
    auto *L = parent.state();
    detail::require(L == other.parent.state(),
                    "binding tables belong to different lua_State handles");
    detail::stack_guard guard(L);
    parent.push();
    other.parent.push();
    lua_getfield(L, -1, other.name.c_str());
    lua_remove(L, -2);
    lua_setfield(L, -2, name.c_str());
    return *this;
  }
  table parent;
  std::string name;
};

inline table_proxy table::operator[](const char *name) const {
  return table_proxy(*this, name);
}

// A non-owning VM facade. Raw handles are needed only at integration
// boundaries.
class state_view {
public:
  explicit state_view(lua_State *state) : L(state) {
    detail::require(L != nullptr, "cannot bind a null Luau state");
  }
  lua_State *lua_state() const { return L; }
  template <class T>
  void import_type(std::string_view name, unsigned version = 1) const {
    luau::import_type<T>(L, name, version);
  }
  void open_libraries() const { luaL_openlibs(L); }
  void collect_garbage() const { lua_gc(L, LUA_GCCOLLECT, 0); }
  table globals() const { return table(L, LUA_GLOBALSINDEX); }
  table create_table() const { return table::create(L); }
  table get_table(const char *name) const { return globals().get_table(name); }
  table get_or_create_table(const char *name) const {
    return globals().get_or_create_table(name);
  }
  table_proxy operator[](const char *name) const { return globals()[name]; }
  template <class T> class_builder<T> class_(const char *name) const {
    return globals().class_<T>(name);
  }
  template <class T> T get(const char *name) const {
    return globals().get<T>(name);
  }
  template <detail::binding_function F>
  state_view &function(const char *name, F &&function) {
    globals().function(name, std::forward<F>(function));
    return *this;
  }
  template <auto Function>
    requires detail::callable<decltype(Function)>
  state_view &function(const char *name) {
    globals().function<Function>(name);
    return *this;
  }
  // Accept compiled bytecode to keep the binding header independent of the
  // compiler library. For example: vm.execute(Luau::compile(source)).
  void execute(std::string_view bytecode, const char *chunk = "chunk") const {
    detail::stack_guard guard(L);
    if (luau_load(L, chunk, bytecode.data(), bytecode.size(), 0) != LUA_OK ||
        lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char *message = lua_tostring(L, -1);
      throw binding_error(message ? message : "Luau execution failed");
    }
  }

protected:
  lua_State *L;
};

// Owns a VM. Declare it before tables/builders/proxies so they die first.
class state : public state_view {
public:
  state() : state_view(luaL_newstate()) {}
  ~state() { lua_close(L); }
  state(const state &) = delete;
  state &operator=(const state &) = delete;
};

namespace detail {
inline int class_index(lua_State *L) {
  return protect(L, [&] {
    require(lua_gettop(L) == 2, "invalid property read");
    stack<std::string_view>::get(L, 2);
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1)); // getters
    if (!lua_isnil(L, -1)) {
      lua_pushvalue(L, 1);
      lua_call(L, 1, 1);
      return 1;
    }
    lua_pop(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(2)); // methods
    return 1;
  });
}
inline int class_newindex(lua_State *L) {
  return protect(L, [&] {
    require(lua_gettop(L) == 3, "invalid property write");
    stack<std::string_view>::get(L, 2);
    lua_pushvalue(L, 2);
    lua_rawget(L, lua_upvalueindex(1)); // setters
    require(!lua_isnil(L, -1), "unknown or read-only C++ property");
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 3);
    lua_call(L, 2, 0);
    return 0;
  });
}

template <class T, class... A> struct constructor {
  void operator()(
      A...) const; // signature only; construction happens in the trampoline
};
template <class T, class... A, size_t... I>
int construct(lua_State *L, int first, std::index_sequence<I...>) {
  std::tuple<decltype(read_argument<A>(L, first))...> values{
      read_argument<A>(L, first + static_cast<int>(I))...};
  std::apply(
      [&](auto &...value) { push_owned<T>(L, std::forward<A>(value)...); },
      values);
  return 1;
}
template <bool Called, class T, class... A> int constructor_call(lua_State *L) {
  return protect(L, [&] {
    constexpr int first = Called ? 2 : 1;
    require(matches_callable<constructor<T, A...>>(L, first),
            "no matching constructor (argument count, type or range)");
    return construct<T, A...>(L, first, std::index_sequence_for<A...>{});
  });
}
} // namespace detail

template <class T> class class_builder {
  static_assert(std::is_class_v<T>, "class_ requires a C++ class or struct");

public:
  class_builder(const table &parent, const char *name)
      : L(parent.state()), metaTable(table::create(L)), methods(metaTable),
        getters(table::create(L)), setters(table::create(L)),
        constructors(table::create(L)) {
    detail::stack_guard guard(L);
    lua_rawgetp(L, LUA_REGISTRYINDEX, &detail::type_key<T>);
    detail::require(lua_isnil(L, -1),
                    "C++ class already registered in this VM/module");
    lua_pop(L, 1);
    metaTable.push();
    getters.push();
    methods.push();
    lua_pushcclosure(L, detail::class_index, "C++.__index", 2);
    lua_setfield(L, -2, "__index");
    setters.push();
    lua_pushcclosure(L, detail::class_newindex, "C++.__newindex", 1);
    lua_setfield(L, -2, "__newindex");
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__type");
    lua_pushstring(L, "protected C++ metatable");
    lua_setfield(L, -2, "__metatable");
    lua_rawsetp(L, LUA_REGISTRYINDEX, &detail::type_key<T>);
    parent_copy(parent, name);
  }
  // Low-level access for custom metamethods and existing C API bindings.
  // Prefer field/property/method for ordinary class members.
  table metatable() const { return metaTable; }
  class_builder &metatable_name(const char *name) {
    detail::stack_guard guard(L);
    metaTable.push();
    lua_setfield(L, LUA_REGISTRYINDEX, name);
    return *this;
  }
  class_builder &export_type(std::string_view name, unsigned version = 1) {
    luau::export_type<T>(L, name, version);
    return *this;
  }
  template <detail::binding_function F>
  class_builder &metamethod(const char *name, F &&function) {
    metaTable.function(name, std::forward<F>(function));
    return *this;
  }
  template <auto Function>
    requires detail::callable<decltype(Function)>
  class_builder &metamethod(const char *name) {
    metaTable.function<Function>(name);
    return *this;
  }
  class_builder &raw_metamethod(const char *name, lua_CFunction function) {
    metaTable.raw_function(name, function);
    return *this;
  }
  template <lua_CFunction Function>
  class_builder &raw_metamethod(const char *name) {
    metaTable.raw_function<Function>(name);
    return *this;
  }
  class_builder &raw_method(const char *name, lua_CFunction function) {
    methods.raw_function(name, function);
    return *this;
  }
  template <lua_CFunction Function>
  class_builder &raw_method(const char *name) {
    methods.raw_function<Function>(name);
    return *this;
  }
  class_builder &raw_static_function(const char *name, lua_CFunction function) {
    constructors.raw_function(name, function);
    return *this;
  }
  template <lua_CFunction Function>
  class_builder &raw_static_function(const char *name) {
    constructors.raw_function<Function>(name);
    return *this;
  }
  template <class V> class_builder &static_value(const char *name, V &&value) {
    constructors.value(name, std::forward<V>(value));
    return *this;
  }
  template <class... A>
    requires std::constructible_from<T, A...>
  class_builder &constructor() {
    detail::stack_guard guard(L);
    constructors.push();
    lua_pushcfunction(L, (detail::constructor_call<false, T, A...>), "C++.new");
    lua_setfield(L, -2, "new");
    detail::create_table(L);
    lua_pushcfunction(L, (detail::constructor_call<true, T, A...>),
                      "C++.__call");
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return *this;
  }
  // Functions/lambdas may take T& or const T& as their first argument.
  template <detail::binding_function F>
  class_builder &method(const char *name, F &&function) {
    methods.function(name, std::forward<F>(function));
    return *this;
  }
  template <auto Function>
    requires detail::callable<decltype(Function)>
  class_builder &method(const char *name) {
    methods.function<Function>(name);
    return *this;
  }
  template <detail::binding_function F>
  class_builder &static_function(const char *name, F &&function) {
    constructors.function(name, std::forward<F>(function));
    return *this;
  }
  template <auto Function>
    requires detail::callable<decltype(Function)>
  class_builder &static_function(const char *name) {
    constructors.function<Function>(name);
    return *this;
  }
  template <auto Member>
    requires(std::is_member_object_pointer_v<decltype(Member)> &&
             requires(T &self) { self.*Member; })
  class_builder &field(const char *name) {
    using V = std::remove_reference_t<decltype(std::declval<T &>().*Member)>;
    getters.function<[](const T &self) -> const V & { return self.*Member; }>(
        name);
    if constexpr (!std::is_const_v<V>)
      setters
          .function<[](T &self, V value) { self.*Member = std::move(value); }>(
              name);
    else
      setters.value(name, nullptr);
    return *this;
  }
  template <auto Member>
    requires(std::is_member_object_pointer_v<decltype(Member)> &&
             requires(T &self) { self.*Member; })
  class_builder &readonly(const char *name) {
    using V = std::remove_reference_t<decltype(std::declval<T &>().*Member)>;
    getters.function<[](const T &self) -> const V & { return self.*Member; }>(
        name);
    setters.value(name, nullptr);
    return *this;
  }
  // Borrow a class-valued member, preserving nested writes such as
  // v.position.x.
  template <auto Member>
    requires(std::is_member_object_pointer_v<decltype(Member)> &&
             requires(T &self) { self.*Member; } &&
             std::is_class_v<
                 std::remove_cvref_t<decltype(std::declval<T &>().*Member)>>)
  class_builder &reference_field(const char *name) {
    using V = std::remove_reference_t<decltype(std::declval<T &>().*Member)>;
    getters.function<[](T &self) { return std::ref(self.*Member); }>(name);
    if constexpr (!std::is_const_v<V>)
      setters
          .function<[](T &self, V value) { self.*Member = std::move(value); }>(
              name);
    else
      setters.value(name, nullptr);
    return *this;
  }
  template <class V> class_builder &field(const char *name, V T::*member) {
    getters.function(
        name, [member](const T &self) -> const V & { return self.*member; });
    if constexpr (!std::is_const_v<V>) {
      setters.function(name, [member](T &self, V value) {
        self.*member = std::move(value);
      });
    } else {
      setters.value(name, nullptr);
    }
    return *this;
  }
  template <class V> class_builder &readonly(const char *name, V T::*member) {
    getters.function(
        name, [member](const T &self) -> const V & { return self.*member; });
    setters.value(name, nullptr);
    return *this;
  }
  template <detail::callable G>
  class_builder &property(const char *name, G &&getter) {
    validate_property<G, 1>();
    getters.function(name, std::forward<G>(getter));
    setters.value(name, nullptr);
    return *this;
  }
  template <auto Getter>
    requires detail::callable<decltype(Getter)>
  class_builder &property(const char *name) {
    validate_property<std::decay_t<decltype(Getter)>, 1>();
    getters.function<Getter>(name);
    setters.value(name, nullptr);
    return *this;
  }
  template <auto Getter, auto Setter>
    requires(detail::callable<decltype(Getter)> &&
             detail::callable<decltype(Setter)>)
  class_builder &property(const char *name) {
    validate_property<std::decay_t<decltype(Getter)>, 1>();
    validate_property<std::decay_t<decltype(Setter)>, 2>();
    static_assert(
        std::is_void_v<
            typename detail::signature<std::decay_t<decltype(Setter)>>::result>,
        "property setter must return void");
    getters.function<Getter>(name);
    setters.function<Setter>(name);
    return *this;
  }
  template <detail::callable G, detail::callable S>
  class_builder &property(const char *name, G &&getter, S &&setter) {
    validate_property<G, 1>();
    validate_property<S, 2>();
    static_assert(
        std::is_void_v<typename detail::signature<std::decay_t<S>>::result>,
        "property setter must return void");
    getters.function(name, std::forward<G>(getter));
    setters.function(name, std::forward<S>(setter));
    return *this;
  }

private:
  template <class F, size_t Count> static void validate_property() {
    using traits = detail::signature<std::decay_t<F>>;
    static_assert(std::tuple_size_v<typename traits::args> == Count,
                  "property must take self (and one value for a setter)");
    if constexpr (Count == 1) {
      static_assert(
          !std::is_void_v<typename traits::result> &&
              !detail::is_tuple<detail::plain<typename traits::result>>::value,
          "property getter must return one value");
    }
  }
  void parent_copy(const table &parent, const char *name) {
    table copy(parent);
    copy.set_table(name, constructors);
  }
  lua_State *L;
  table metaTable, methods, getters, setters, constructors;
};

template <class T> class_builder<T> table::class_(const char *name) {
  return class_builder<T>(*this, name);
}

// Use around plugin registration to report C++ exceptions as Luau errors.
template <class F> int open(lua_State *L, F &&register_bindings) {
  return detail::protect(L, [&] {
    detail::stack_guard guard(L);
    std::invoke(std::forward<F>(register_bindings));
    return 0;
  });
}

// Keep the native module ABI at the boundary; user registration receives only
// the module table and publication happens after successful registration.
template <class F>
int open_plugin(lua_State *L, std::string_view name, F &&register_bindings) {
  return open(L, [&] {
    auto module = state_view(L).create_table();
    std::invoke(std::forward<F>(register_bindings), module);
    module.publish_plugin(name);
  });
}

} // namespace sinen::luau
#endif // SINEN_LUAU_BIND_HPP

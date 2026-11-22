# C++ / Luau bindings

`script/luau_bind.hpp` is Sinen's original header-only C++23 binding library for the vendored Luau VM. 

## Register without raw VM handles

`state` owns a VM; `state_view` borrows an existing VM at the integration boundary. Normal registration functions receive a `table&` and do not use `lua_State*`, stack indices, or push/pop operations.

```cpp
#include <script/luau_bind.hpp>
#include <Luau/Compiler.h>

sinen::luau::state lua;
lua.open_libraries();
lua["sum"] = [](int a, int b) { return a + b; };
auto config = lua.get_or_create_table("config");
config["scale"] = 2.0f;
lua.execute(Luau::compile("result = sum(2, 3)"));
int result = lua["result"]; // 5
```

`execute` accepts compiled bytecode, so ordinary binding users and native plugins need no compiler dependency. Executables using `Luau::compile` also link `Luau.Compiler`. `get_table` reads an existing table; `get_or_create_table` creates a missing table and rejects an existing non-table value. `table["key"]` supports values, functions, tables, and copying another proxy; use `.get<T>()` for an explicit read.

## Compile-time registration with C++20/23

When a function/member is known at compile time, pass it as a template argument:

```cpp
module.function<&sum>("sum");
module.class_<Counter>("Counter")
    .constructor<int>()
    .method<&Counter::add>("add")
    .field<&Counter::value>("value")
    .property<&Counter::get, &Counter::set>("amount");
module.function<[](int n) static { return n * 2; }>("double");
```

This path stores no callback capture userdata and does not allocate a C++ callable object on the heap. It retains the same argument validation and exception translation. Runtime function pointers, capturing lambdas, and `std::function` still use the existing value-taking overloads. Use those overloads when the callback has state that must persist between calls.

`reference_field<&Transform::position>("position")` replaces a borrowing getter plus assigning setter and preserves nested writes such as `transform.position.x = 1`. Ordinary `field` still returns a copy; borrowed members must outlive their referring userdata. A const member is read-only. Concepts constrain callable signatures and constructors; enum conversions use C++23 `std::to_underlying`.

## Native plugins

```cpp
#include <script/luau_bind.hpp>

struct Counter {
  explicit Counter(int value) : value(value) {}
  int add(int amount) { return value += amount; }
  int value;
};

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

void registerCounter(sinen::luau::table &module) {
  module.function("sum", [](int a, int b) { return a + b; });
  module.class_<Counter>("Counter")
      .constructor<int>()
      .method<&Counter::add>("add")
      .field<&Counter::value>("value")
      .readonly<&Counter::value>("readOnly");
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *L) {
  return sinen::luau::open_plugin(L, "counter", registerCounter);
}
```

Build inside Sinen with `sinen_add_lua_module(counter counter.cpp)`. This helper supplies the header paths through the `sinen::luau_bind` interface target and preserves the existing native/Wasm linking rules. Non-plugin executables can link `sinen::luau_bind` for headers and `Luau.VM` for the runtime. The header also supports `<sinen/script/luau_bind.hpp>` when the repository's `include` directory is on the include path.

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("counter")
assert(ok, err)
local native = require("@sinen/plugin/counter")
local c = native.Counter.new(5) -- also native.Counter(5)
c.value = 10
print(c:add(3), native.sum(2, 3)) -- 13, 5
```

## Registration

| API | Purpose |
| --- | --- |
| `state` / `state_view(existingState)` | Owning / borrowed VM handle |
| `vm["name"] = valueOrFunction` | Global registration and lookup |
| `.get_table(name)` / `.get_or_create_table(name)` | Read / create nested tables |
| `.get<T>(name)` / `[name].get<T>()` | Typed value read |
| `open_plugin(L, name, registerFunction)` | Native ABI adapter passing `table&` |
| `table::create(L)` / `table(L, index)` | New table / reference to an existing table |
| `.function<Function>(name)` / `.method<Function>(name)` | Compile-time callback without capture storage |
| `.field<Member>(name)` / `.readonly<Member>(name)` | Compile-time member binding |
| `.reference_field<Member>(name)` | Borrowed class-valued field with nested writes |
| `.property<Getter, Setter>(name)` | Compile-time accessors |
| `.function(name, callable)` | Free function, capturing lambda, or `std::function` |
| `.raw_function(name, lua_CFunction)` | Existing `int(lua_State*)` callback |
| `.value(name, value)` / `.set_table(name, table)` | Value / nested table |
| `.publish(name)` / `.publish_plugin(name)` | Global / plugin registry entry |
| `.class_<T>(name)` | Register a C++ type |
| `.constructor<Args...>()` | `Class.new(...)` and `Class(...)` |
| `.method(name, callable)` | Member pointer, or callable with `T&` / `const T&` first |
| `.static_function(name, callable)` | `Class.function(...)` |
| `.metamethod(name, callable)` | Typed operators such as `__add` and `__tostring` |
| `.raw_method` / `.raw_static_function` / `.raw_metamethod` | Explicit C API escape hatches |
| `.static_value(name, value)` | Class constants |
| `.metatable_name(name)` | Registry alias for legacy C API interoperation |
| `.metatable()` | Low-level table handle for custom C API metamethods |
| `.field(name, &T::member)` / `.readonly(name, &T::member)` | Writable / read-only field |
| `.property(name, getter[, setter])` | Accessor property; setter returns `void` |

Register each C++ type once per VM per binary. A class without a constructor can still be returned by factories. Repeating `.constructor` replaces the previous signature.

`sinen::luau::overload(f1, f2, ...)` binds the first matching signature in declaration order, for functions and methods. Disambiguate overloaded C++ pointers with `static_cast`. Generic lambdas need a wrapper with concrete argument types. `std::tuple` return values become multiple Luau results; `void` returns no values.

## Conversions and ownership

Supported conversions: booleans, integers, floating point, enums, `std::string`, `std::string_view`, `const char*`, `nullptr`, `std::optional<T>`, dense `std::vector<T>` arrays, and registered classes. Optional arguments accept `nil` or omission. Strings and string views preserve embedded NUL bytes; `const char*` follows C string conventions. Numeric strings, fractional integers, holes in arrays, and dictionary keys are rejected. Integer values must fit both the C++ type and the exact range `[-(2^53-1), 2^53-1]`.

Constructed objects and returned class values are owned and deleted by Luau userdata destructors. Class value returns support move-only types. Class reference returns and class-valued fields are copied. Use `std::ref(object)` explicitly to borrow; `std::cref(object)` rejects non-const methods and writes. Borrowed objects must outlive all referring userdata. Class reference arguments access the original userdata. Raw object pointers and implicit inheritance conversions are unsupported. String view / C string arguments must not be retained beyond the call.

Specialize `sinen::luau::stack<T>` with `matches(L, index)`, `get(L, index)`, and `push(L, value)` for custom one-value conversions. `matches` preserves the stack, `get` throws `binding_error` for invalid values, and `push` adds exactly one value.

## Lifetime and errors

`table`, `class_builder`, and index proxies hold registry references and must be destroyed before their owning `state` (or before `lua_close` for a borrowed VM). Declare the `state` before these handles. A `state_view` never closes the VM. Registered functions and classes remain valid after these C++ handles die. Plugin binaries must remain loaded until the VM closes; Sinen's existing loader guarantees this ordering. Registration preserves stack height; `.push()` explicitly adds one table.

C++ exceptions become Luau errors that scripts can catch with `pcall`. Wrap registration with `sinen::luau::open`. Raw C API callbacks manage their own stack and return counts; their C++ exceptions are also translated into Luau errors. Enable C++ exceptions in both Luau and plugins, with `LUA_USE_LONGJMP=0`. Destructors must not throw. Automatic inheritance, Luau-to-C++ function callbacks, and yielding across C++ calls are outside this API's scope.

See the [DLL fixture](https://github.com/astomih/sinen/blob/main/tests/cpp/native_module_fixture.cpp), [behavior tests](https://github.com/astomih/sinen/blob/main/tests/cpp/luau_bind_library_tests.cpp), and [native module loading guide](native_modules.md).

The engine uses this same binding API. Value classes and `Ptr<T>` resources share its userdata ownership and type checks. Complex callbacks and table parsers use `raw_function`; their userdata conversions use `stack<T>` as well. Existing `sn.Class.new(...)` entry points remain available. Typed entry points use the strict conversions described above.

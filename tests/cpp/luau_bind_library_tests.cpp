#include <Luau/Compiler.h>
#include <gtest/gtest.h>
#include <script/luau_bind.hpp>

#include <cstdint>
#include <numeric>

namespace {
namespace bind = sinen::luau;

struct Counter {
  explicit Counter(int value) : value(value) {}
  int add(int amount) { return value += amount; }
  int get() const noexcept { return value; }
  void set(int amount) { value = amount; }
  int value;
};

struct Other {
  int value = 0;
};
struct alignas(64) Tracked {
  static inline int alive = 0;
  explicit Tracked(int value) : value(value) {
    if (value < 0)
      throw std::runtime_error("constructor failed");
    ++alive;
  }
  Tracked(Tracked &&other) noexcept : value(other.value) { ++alive; }
  Tracked(const Tracked &) = delete;
  ~Tracked() { --alive; }
  int value;
};

enum class Mode : unsigned { Fast = 2 };

template <class F>
concept RegistersFunction = requires(bind::table &module, F &&function) {
  module.function("function", std::forward<F>(function));
};
static_assert(RegistersFunction<decltype([](int value) { return value; })>);
static_assert(!RegistersFunction<int>);
static_assert(!RegistersFunction<decltype([](auto value) { return value; })>);

struct RvalueOnly {
  int operator()(int) &&;
};
static_assert(!RegistersFunction<RvalueOnly>);

struct CounterHolder {
  Counter counter{1};
  const Counter constant{2};
  const int fixed = 3;
};

int staticRawPair(lua_State *L) {
  lua_pushinteger(L, 2);
  lua_pushinteger(L, 3);
  return 2;
}
int staticRawFailure(lua_State *) {
  throw std::runtime_error("static raw failure");
}

class LuauBindLibrary : public testing::Test {
protected:
  void SetUp() override {
    L = luaL_newstate();
    ASSERT_NE(L, nullptr);
    luaL_openlibs(L);
  }
  void TearDown() override {
    if (L)
      lua_close(L);
  }
  testing::AssertionResult run(const char *source) {
    const auto bytecode = Luau::compile(source);
    const int status = luau_load(L, "binding_library_test", bytecode.data(),
                                 bytecode.size(), 0);
    if (status != LUA_OK || lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char *message = lua_tostring(L, -1);
      return testing::AssertionFailure()
             << (message ? message : "unknown Luau error");
    }
    return testing::AssertionSuccess();
  }
  void register_counter(bind::table &module) {
    module.class_<Counter>("Counter")
        .constructor<int>()
        .method("add", &Counter::add)
        .method("get", &Counter::get)
        .method("extend",
                [](Counter &self, int amount) { return self.add(amount * 2); })
        .field("value", &Counter::value)
        .readonly("readOnly", &Counter::value)
        .field("rebound", &Counter::value)
        .readonly("rebound", &Counter::value)
        .property("amount", &Counter::get, &Counter::set)
        .property("doubled", [](const Counter &self) { return self.value * 2; })
        .static_function("zero", [] { return Counter(0); });
  }
  lua_State *L = nullptr;
};

TEST_F(LuauBindLibrary, FunctionsCapturingLambdasAndStackBalance) {
  lua_pushinteger(L, 123);
  {
    auto module = bind::table::create(L);
    module.function(
              "sum", +[](int a, int b) { return a + b; })
        .function("next", [value = 0]() mutable { return ++value; })
        .function("voidResult", [] {})
        .function("greet",
                  std::function<std::string(const std::string &)>(
                      [](const std::string &s) { return "hello " + s; }))
        .value("answer", 42)
        .value("name", "native")
        .publish("test");
    EXPECT_EQ(lua_gettop(L), 1);
    auto copy = module;
    auto moved = std::move(copy);
    auto child = bind::table::create(L);
    child.value("enabled", true);
    moved.set_table("child", child);
    EXPECT_EQ(lua_gettop(L), 1);
  }
  ASSERT_TRUE(run(R"(
    assert(test.sum(2, 3) == 5)
    assert(test.next() == 1 and test.next() == 2)
    assert(select('#', test.voidResult()) == 0)
    assert(test.greet('world') == 'hello world')
    assert(test.answer == 42 and test.name == 'native' and test.child.enabled)
  )"));
  EXPECT_EQ(lua_gettop(L), 1);
  EXPECT_EQ(lua_tointeger(L, 1), 123);
}

TEST_F(LuauBindLibrary, CompileTimeBindingsNeedNoCaptureUserdata) {
  auto module = bind::table::create(L);
  constexpr auto triple = [](int value) static { return value * 3; };
  module.function<triple>("triple")
      .raw_function<staticRawPair>("pair")
      .raw_function<staticRawFailure>("fail")
      .function("captured", [value = 10]() mutable { return ++value; })
      .publish("test");
  module.class_<Counter>("Counter")
      .constructor<int>()
      .method<&Counter::add>("add")
      .field<&Counter::value>("value")
      .readonly<&Counter::value>("readOnly")
      .property<&Counter::get, &Counter::set>("amount")
      .property<&Counter::get>("getOnly")
      .static_function<[] { return Counter(0); }>("zero")
      .metamethod<[](const Counter &value) {
        return std::to_string(value.value);
      }>("__tostring");
  module.class_<CounterHolder>("Holder")
      .constructor<>()
      .reference_field<&CounterHolder::counter>("counter")
      .reference_field<&CounterHolder::constant>("constant")
      .field<&CounterHolder::fixed>("fixed");
  module.push();
  for (const char *name : {"triple", "pair", "fail"}) {
    lua_getfield(L, -1, name);
    EXPECT_EQ(lua_getupvalue(L, -1, 1), nullptr) << name;
    lua_pop(L, 1);
  }
  lua_getfield(L, -1, "captured");
  EXPECT_NE(lua_getupvalue(L, -1, 1), nullptr);
  lua_pop(L, 3); // capture, function, module
  EXPECT_EQ(lua_gettop(L), 0);
  ASSERT_TRUE(run(R"(
assert(test.triple(4) == 12)
local a, b = test.pair()
assert(a == 2 and b == 3)
local ok, message = pcall(test.fail)
assert(not ok and string.find(message, "static raw failure", 1, true))
assert(test.captured() == 11 and test.captured() == 12)
assert(not pcall(function() test.triple('bad') end))
local c = test.Counter.new(1)
c.value = 3
assert(c:add(2) == 5 and c.readOnly == 5)
c.amount = 8
assert(c.getOnly == 8 and tostring(c) == '8')
assert(test.Counter.zero().value == 0)
assert(not pcall(function() c.getOnly = 4 end))
local holder = test.Holder.new()
holder.counter.value = 9
assert(holder.counter.value == 9)
holder.counter = c
assert(holder.counter.value == 8)
assert(holder.constant.value == 2 and holder.fixed == 3)
assert(not pcall(function() holder.constant:add(1) end))
assert(not pcall(function() holder.constant = c end))
assert(not pcall(function() holder.fixed = 4 end))
)"));
}

TEST_F(LuauBindLibrary, ConstructorsMethodsFieldsAndProperties) {
  {
    auto module = bind::table::create(L);
    register_counter(module);
    module.publish("test");
  }
  ASSERT_TRUE(run(R"(
    local c = test.Counter.new(3)
    assert(c:add(4) == 7 and c:get() == 7)
    c.value = 8
    c.amount = 9
    assert(c.value == 9 and c.readOnly == 9 and c.doubled == 18)
    assert(c:extend(2) == 13)
    assert(test.Counter(5).value == 5)
    assert(test.Counter.zero().value == 0)
    assert(c.missing == nil)
    assert(not pcall(function() c.readOnly = 1 end))
    assert(not pcall(function() c.rebound = 1 end))
    assert(not pcall(function() c.doubled = 1 end))
    assert(not pcall(function() c.missing = 1 end))
    assert(not pcall(function() c.value = 'bad' end))
    for i = 1, 50 do
      assert(not pcall(function() c.value = {} end))
    end
    assert(c:get() == 13)
    assert(not pcall(function() test.Counter('bad') end))
    assert(not pcall(function() test.Counter.new(1, 2) end))
    assert(type(getmetatable(c)) == 'string')
  )"));
}

TEST_F(LuauBindLibrary, OverloadsUseStrictTypesAndDeclaredOrder) {
  {
    auto module = bind::table::create(L);
    module.function("choose",
                    bind::overload([](int n) { return n * 2; },
                                   [](const std::string &s) { return s + s; },
                                   [](int a, int b) { return a + b; }));
    auto overloads = bind::overload([](int n) { return n; }, [] { return 0; });
    module.function("lvalue", overloads);
    const auto const_overloads = overloads;
    module.function("constLvalue", const_overloads);
    module.class_<Counter>("Counter").constructor<int>().method(
        "add", bind::overload(
                   [](Counter &self, int amount) { return self.add(amount); },
                   [](Counter &self, const std::string &s) {
                     return self.add(static_cast<int>(s.size()));
                   }));
    module.publish("test");
  }
  ASSERT_TRUE(run(R"(
    assert(test.choose(3) == 6 and test.choose('3') == '33')
    assert(test.choose(3, 4) == 7)
    assert(test.lvalue(3) == 3 and test.constLvalue() == 0)
    assert(not pcall(test.choose, true))
    assert(not pcall(test.choose, 1.5))
    local c = test.Counter(0)
    assert(c:add(3) == 3 and c:add('abc') == 6)
  )"));
}

TEST_F(LuauBindLibrary, OptionalArraysEnumsBinaryStringsAndMultipleResults) {
  {
    auto module = bind::table::create(L);
    module.function("optional", [](std::optional<int> n) { return n; })
        .function("array",
                  [](const std::vector<int> &values) { return values; })
        .function(
            "results",
            [] { return std::make_tuple(42, std::string("yes"), nullptr); })
        .function("mode", [](Mode m) { return m; })
        .function("binary", [](std::string_view s) { return std::string(s); })
        .function("boolean", [](bool value) { return !value; })
        .value("fast", Mode::Fast)
        .publish("test");
  }
  ASSERT_TRUE(run(R"(
    assert(test.optional() == nil and test.optional(nil) == nil)
    assert(test.optional(3) == 3)
    assert(not pcall(test.optional, '3'))
    local a = test.array({1, 2, 3})
    assert(#a == 3 and a[2] == 2 and #test.array({}) == 0)
    assert(not pcall(test.array, {1, '2'}))
    assert(not pcall(test.array, {[1] = 1, [3] = 3}))
    assert(not pcall(test.array, {x = 1}))
    local n, s, empty = test.results()
    assert(n == 42 and s == 'yes' and empty == nil)
    assert(select('#', test.results()) == 3)
    assert(test.mode(test.fast) == 2)
    assert(test.binary('a\0b') == 'a\0b')
    assert(test.boolean(false) and not pcall(test.boolean, 1))
  )"));
}

TEST_F(LuauBindLibrary, RejectsWrongUserdataAndScalarArguments) {
  {
    auto module = bind::table::create(L);
    register_counter(module);
    module.class_<Other>("Other").constructor<>();
    module.function("read", [](const Counter &c) { return c.value; })
        .function("byte", [](std::uint8_t value) { return value; })
        .function("integer", [](std::int64_t value) { return value; })
        .function("overflow",
                  [] { return std::numeric_limits<std::uint64_t>::max(); })
        .publish("test");
  }
  ASSERT_TRUE(run(R"(
    assert(not pcall(test.read, test.Other()))
    assert(not pcall(test.read, {}))
    assert(not pcall(test.read, newproxy(true)))
    assert(not pcall(test.read, nil))
    assert(not pcall(test.byte, -1) and not pcall(test.byte, 256))
    assert(not pcall(test.integer, 1.5))
    assert(not pcall(test.integer, math.huge))
    assert(not pcall(test.integer, 0/0))
    assert(not pcall(test.integer, 9007199254740992))
    assert(not pcall(test.integer, '42'))
    assert(not pcall(test.overflow))
    assert(test.integer(9007199254740991) == 9007199254740991)
    assert(test.byte(255) == 255)
    local c = test.Counter(5)
    assert(not pcall(c.add, test.Other(), 1))
    assert(not pcall(c.add, c))
    assert(not pcall(c.add, c, 1, 2))
    assert(test.read(c) == 5)
  )"));
}

TEST_F(LuauBindLibrary, CopiesReferencesAndSupportsExplicitConstBorrowing) {
  Counter external(8);
  {
    auto module = bind::table::create(L);
    register_counter(module);
    module.function("copy", [&]() -> const Counter & { return external; })
        .value("borrowed", std::ref(external))
        .value("constant", std::cref(external))
        .publish("test");
  }
  ASSERT_TRUE(run(R"(
    local copy = test.copy()
    copy.value = 100
    assert(test.borrowed.value == 8)
    assert(test.borrowed:add(2) == 10)
    assert(test.constant:get() == 10)
    assert(not pcall(function() test.constant.value = 1 end))
    assert(not pcall(function() test.constant:add(1) end))
    assert(not pcall(function() test.constant.amount = 1 end))
  )"));
  EXPECT_EQ(external.value, 10);
  // The external object outlives every borrowed userdata.
  lua_close(L);
  L = nullptr;
}

TEST_F(LuauBindLibrary, CollectsOwnedObjectsMoveOnlyValuesAndCaptures) {
  ASSERT_EQ(Tracked::alive, 0);
  auto resource = std::make_shared<int>(42);
  std::weak_ptr<int> weak = resource;
  {
    auto module = bind::table::create(L);
    module.class_<Tracked>("Tracked").constructor<int>().readonly(
        "value", &Tracked::value);
    module.function("make", [] { return Tracked(7); })
        .function("resource",
                  [resource = std::move(resource)] { return *resource; })
        .function("moveCapture",
                  [value = std::make_unique<int>(3)] { return *value; })
        .publish("test");
  }
  ASSERT_TRUE(run(R"(
    do
      local a = test.Tracked(2)
      local b = test.make()
      assert(a.value == 2 and b.value == 7)
      assert(not pcall(test.Tracked.new, -1))
      assert(test.resource() == 42 and test.moveCapture() == 3)
    end
  )"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_EQ(Tracked::alive, 0);
  EXPECT_FALSE(weak.expired());
  ASSERT_TRUE(run("test = nil"));
  lua_gc(L, LUA_GCCOLLECT, 0);
  EXPECT_TRUE(weak.expired());
  bind::stack<Tracked>::push(L, Tracked(9));
  lua_setglobal(L, "aliveUntilClose");
  EXPECT_EQ(Tracked::alive, 1);
  lua_close(L);
  L = nullptr;
  EXPECT_EQ(Tracked::alive, 0);
}

TEST_F(LuauBindLibrary, UnwindsConvertedArgumentsAndReportsCppExceptions) {
  {
    auto module = bind::table::create(L);
    module
        .function("fail",
                  [](std::string s, std::vector<int> values) -> int {
                    throw std::runtime_error(s + std::to_string(values.size()));
                  })
        .function("unknown", [] { throw 7; })
        .publish("test");
  }
  ASSERT_TRUE(run(R"(
    for i = 1, 50 do
      local ok, err = pcall(test.fail, 'failure:', {1, 2, 3})
      assert(not ok and string.find(err, 'failure:3', 1, true))
    end
    local ok, err = pcall(test.unknown)
    assert(not ok and string.find(err, 'unknown C++ exception', 1, true))
  )"));
  EXPECT_EQ(lua_gettop(L), 0);
}

TEST_F(LuauBindLibrary, RegistrationErrorsAndRawFunctionsPreserveStack) {
  auto module = bind::table::create(L);
  register_counter(module);
  EXPECT_THROW(module.class_<Counter>("Duplicate"), bind::binding_error);
  EXPECT_EQ(lua_gettop(L), 0);
  module
      .raw_function(
          "raw",
          +[](lua_State *state) {
            lua_pushinteger(state, lua_gettop(state));
            return 1;
          })
      .publish("test");
  EXPECT_THROW(module.value("unregistered", Other{}), bind::binding_error);
  EXPECT_THROW(module.publish_plugin("invalid/name"), bind::binding_error);
  EXPECT_EQ(lua_gettop(L), 0);
  ASSERT_TRUE(run("assert(test.raw(1, 2, 3) == 3)"));
  lua_pushcfunction(
      L,
      +[](lua_State *state) {
        return bind::open(
            state, [] { throw std::runtime_error("registration failed"); });
      },
      "registration");
  EXPECT_EQ(lua_pcall(L, 0, 0, 0), LUA_ERRRUN);
  EXPECT_NE(std::string_view(lua_tostring(L, -1)).find("registration failed"),
            std::string_view::npos);
}

TEST_F(LuauBindLibrary, ClassRegistrationsAreIndependentAcrossStates) {
  auto module = bind::table::create(L);
  register_counter(module);
  lua_State *second = luaL_newstate();
  ASSERT_NE(second, nullptr);
  {
    auto second_module = bind::table::create(second);
    register_counter(second_module);
    EXPECT_THROW(module.set_table("foreign", second_module),
                 bind::binding_error);
    second_module.publish("test");
  }
  const auto code = Luau::compile("assert(test.Counter(2):get() == 2)");
  ASSERT_EQ(luau_load(second, "second", code.data(), code.size(), 0), LUA_OK);
  // This state needs the assert builtin, too.
  luaL_openlibs(second);
  EXPECT_EQ(lua_pcall(second, 0, 0, 0), LUA_OK);
  lua_close(second);
}

TEST_F(LuauBindLibrary, RawCallbacksTranslateExceptionsAndPreserveResults) {
  auto module = bind::table::create(L);
  module
      .raw_function(
          "fail",
          +[](lua_State *) -> int {
            throw std::runtime_error("raw callback failed");
          })
      .raw_function(
          "pair",
          +[](lua_State *state) {
            lua_pushinteger(state, 2);
            lua_pushinteger(state, 3);
            return 2;
          })
      .publish("test");
  ASSERT_TRUE(run(R"(
local ok, message = pcall(test.fail)
assert(not ok and string.find(message, "raw callback failed", 1, true))
local a, b = test.pair()
assert(a == 2 and b == 3)
)"));
  EXPECT_EQ(lua_gettop(L), 0);
}

TEST(LuauState, RegistersAndExecutesWithoutRawHandles) {
  bind::state vm;
  vm.open_libraries();
  vm["answer"] = 42;
  vm["sum"] = [](int a, int b) { return a + b; };
  vm["convert"] = bind::overload([](int n) { return n * 2; },
                                 [](std::string s) { return s + s; });
  auto module = vm.get_or_create_table("test");
  module.class_<Counter>("Counter")
      .constructor<int>()
      .method("add", &Counter::add)
      .field("value", &Counter::value)
      .static_value("initial", 5);
  module["label"] = "counter";
  auto config = module.get_or_create_table("config");
  config["enabled"] = true;
  vm["config"] = config;
  vm["copied"] = vm["answer"];
  auto proxy = vm.get_table("test")["label"];
  proxy = "updated";
  vm.execute(Luau::compile(R"(
assert(sum(answer, copied) == 84)
assert(convert(3) == 6 and convert("x") == "xx")
assert(test.label == "updated" and config.enabled)
local counter = test.Counter.new(test.Counter.initial)
counter.value = counter:add(3)
result = counter.value
)"));
  int result = vm["result"];
  EXPECT_EQ(result, 8);
  EXPECT_EQ(module.get<std::string>("label"), "updated");
  EXPECT_TRUE(vm.get_table("config").get<bool>("enabled"));
  vm.collect_garbage();
}

TEST(LuauState, ClosingOwnedVmDestroysObjects) {
  EXPECT_EQ(Tracked::alive, 0);
  {
    bind::state vm;
    vm.class_<Tracked>("Tracked").constructor<int>();
    vm.execute(Luau::compile("object = Tracked.new(8)"));
    EXPECT_EQ(Tracked::alive, 1);
  }
  EXPECT_EQ(Tracked::alive, 0);
}

TEST_F(LuauBindLibrary, StateViewsPreserveStackAndLeaveOwnershipWithCaller) {
  lua_pushinteger(L, 99);
  {
    bind::state_view vm(L);
    EXPECT_THROW(bind::state_view(nullptr), bind::binding_error);
    auto module = vm.get_or_create_table("test");
    module["value"] = 7;
    auto same = vm.get_or_create_table("test");
    EXPECT_EQ(same["value"].get<int>(), 7);
    module["child"] = module.create_table();
    EXPECT_NO_THROW(module.get_table("child"));
    EXPECT_THROW(module.get_table("value"), bind::binding_error);
    EXPECT_THROW(module.get_or_create_table("value"), bind::binding_error);
    EXPECT_THROW(module.get<int>("missing"), bind::binding_error);
    EXPECT_THROW(vm.execute(Luau::compile("error('execution failed')")),
                 bind::binding_error);
    EXPECT_THROW(vm.execute("invalid bytecode"), bind::binding_error);
    vm.execute(Luau::compile("assert(test.value == 7)"));
    EXPECT_EQ(lua_gettop(L), 1);
    EXPECT_EQ(lua_tointeger(L, 1), 99);
  }
  ASSERT_TRUE(run("assert(test.value == 7)"));
  EXPECT_EQ(lua_gettop(L), 1);
}

TEST_F(LuauBindLibrary,
       ProxyAssignmentRejectsForeignStatesWithoutChangingValues) {
  bind::state_view vm(L);
  bind::state foreign;
  vm["value"] = 7;
  foreign["value"] = 11;
  EXPECT_THROW(vm["value"] = foreign["value"], bind::binding_error);
  EXPECT_THROW(vm["value"] = foreign.create_table(), bind::binding_error);
  EXPECT_EQ(vm["value"].get<int>(), 7);
  EXPECT_EQ(foreign["value"].get<int>(), 11);
  EXPECT_EQ(lua_gettop(L), 0);
}
} // namespace

# C++ / Luau バインディング

`script/luau_bind.hpp` は Sinen 独自のヘッダーオンリーライブラリです。C++17 以降と、このリポジトリの Luau を使用します。sol2 / luaaa の宣言的な登録APIを参考に、型変換、呼び出し、userdata管理を独自に実装しています。両ライブラリへの依存やソースコードの取り込みはありません。

## プラグインを書く

```cpp
#include <script/luau_bind.hpp>

struct Counter {
  explicit Counter(int value) : value(value) {}
  int add(int amount) { return value += amount; }
  int get() const { return value; }
  void set(int next) { value = next; }
  int value;
};

SINEN_LUA_MODULE_EXPORT uint32_t sinen_lua_module_get_api_version() {
  return SINEN_LUA_MODULE_API_VERSION;
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *L) {
  return sinen::luau::open(L, [&] {
    auto module = sinen::luau::table::create(L);
    module.function("sum", [](int a, int b) { return a + b; });
    module.class_<Counter>("Counter")
        .constructor<int>()
        .method("add", &Counter::add)
        .field("value", &Counter::value)
        .property("amount", &Counter::get, &Counter::set)
        .readonly("readOnly", &Counter::value);
    module.publish_plugin("counter");
  });
}
```

Sinen と同じ CMake ビルド内で登録します。ヘッダーのインクルードパスはヘルパーが設定します。

```cmake
sinen_add_lua_module(counter counter.cpp)
```

プラグインを実行時のカレントディレクトリに配置して読み込みます。プラットフォームごとの配置規則は [ネイティブモジュール](native_modules.md) を参照してください。

```luau
local sn = require("@sinen")
local ok, err = sn.Script.loadPlugin("counter")
assert(ok, err)
local native = require("@sinen/plugin/counter")
local c = native.Counter.new(5) -- native.Counter(5) も使用可能
c.amount = 10
print(c:add(3), c.value, native.sum(2, 3)) -- 13, 13, 5
```

## 登録API

| API | 用途 |
| --- | --- |
| `table::create(L)` / `table(L, index)` | 新規テーブル / 既存テーブルの参照 |
| `.function(name, callable)` | 自由関数、ラムダ、`std::function` |
| `.raw_function(name, lua_CFunction)` | C API を直接使用する関数 |
| `.value(name, value)` / `.set_table(name, table)` | 値 / サブテーブル |
| `.publish(name)` / `.publish_plugin(name)` | グローバル / プラグインとして公開 |
| `.class_<T>(name)` | C++ クラスを登録 |
| `.constructor<Args...>()` | `Class.new(...)` と `Class(...)` |
| `.method(name, callable)` | メンバー関数、または第1引数に `T&` / `const T&` を取る関数 |
| `.static_function(name, callable)` | `Class.function(...)` |
| `.metamethod(name, callable)` | `__add` や `__tostring` などの型付き演算子 |
| `.metatable()` | 独自の C API メタメソッド用のテーブル参照 |
| `.field(name, &T::member)` / `.readonly(name, &T::member)` | 読み書き / 読み取り専用フィールド |
| `.property(name, getter[, setter])` | アクセサによるプロパティ |

クラスは C++ 型ごとに同じ VM・同じバイナリ内で1回登録します。`.constructor` を省略すると、ファクトリから返すオブジェクトだけを公開できます。複数回 `.constructor` を呼ぶ場合は最後のシグネチャが有効になります。

## オーバーロードと返り値

```cpp
namespace bind = sinen::luau;
module.function("convert", bind::overload(
    [](int n) { return n * 2; },
    [](std::string s) { return s + s; }));
module.function("read", [](std::optional<int> limit) {
  return std::make_tuple(limit.value_or(10), std::string("ok"));
});
```

`overload` は登録順で最初に型と引数数が一致する関数を選びます。数値文字列の数値化や、小数の整数への切り捨ては行いません。`std::optional<T>` は `nil` と省略した引数を受け取れます。戻り値の `std::tuple` は複数の Luau 戻り値になります。`void` は戻り値なしです。

C++ 側でオーバーロードされている関数は、`static_cast<int (*)(int)>(&function)` などで型を明示してください。ジェネリックラムダは、引数型を固定したラムダで包んでください。

## 型と所有権

対応する型は `bool`、整数、浮動小数、列挙型、`std::string`、`std::string_view`、`const char*`、`nullptr`、`std::optional<T>`、`std::vector<T>`、登録したクラスです。`vector` は1始まりの密な配列と対応し、穴のあるテーブルや辞書を拒否します。文字列と `string_view` は埋め込み NUL を保持します。`const char*` の利用時は C 文字列の規則に従ってください。

- コンストラクタやクラス値の戻り値は Luau が所有し、Luau の userdata destructor で破棄します。値の戻り値は move-only クラスにも対応します。
- `T&` / `const T&` の戻り値とクラス型フィールドの読み出しはコピーです。変更を元のオブジェクトへ反映する場合は、明示的に `std::ref(object)` を返すか登録します。
- `std::cref(object)` は const の借用です。非constメソッド、フィールド代入、setter は拒否します。借用先は、それを参照する全userdataより長く生存させてください。
- `T&` / `const T&` の引数とメンバー関数の `self` は登録済みuserdataを直接参照します。生ポインタの自動変換や継承関係の自動変換はありません。
- `string_view` と `const char*` の引数は呼び出し中のみ有効です。後で使用するデータはC++側でコピーしてください。
- 整数は C++ 型の範囲と Luau で正確に扱える範囲 `[-(2^53-1), 2^53-1]` を確認します。

独自の変換は `sinen::luau::stack<T>` を特殊化し、`matches(L, index)`、`get(L, index)`、`push(L, value)` を実装します。`matches` はスタックを変更せず、`push` は1値だけ積みます。`get` は不正値に `binding_error` を投げてください。

## 寿命とエラー

`table` と `class_builder` はレジストリ参照を保持し、スタックの位置に依存しません。これらのC++ハンドルは `lua_close` より前に破棄してください。登録された関数やクラスはハンドル破棄後も有効です。プラグインのDLLはVM終了後にアンロードしてください。既存の Sinen ローダーはこの順序を保証します。

通常の登録操作はスタックの高さを維持します。`.push()` だけは明示的にテーブルを1つ積みます。C++例外は Luau エラーへ変換され、スクリプトの `pcall` で処理できます。登録時は `sinen::luau::open` を使用してください。`raw_function` の関数はスタックと戻り値の数を管理します。C++例外は型付き関数と同様に Luau エラーへ変換されます。

Luau とプラグインの両方でC++例外を有効にし、`LUA_USE_LONGJMP=0` を使用してください。デストラクタは例外を投げないことが前提です。自動継承、Luau関数からC++コールバックへの変換、yieldをまたぐC++呼び出しは対象外です。

登録の実例は [DLLテストプラグイン](https://github.com/astomih/sinen/blob/main/tests/cpp/native_module_fixture.cpp)、動作仕様は [テスト](https://github.com/astomih/sinen/blob/main/tests/cpp/luau_bind_library_tests.cpp) にあります。

API設計の参考: [sol2 usertype](https://sol2.readthedocs.io/en/latest/api/usertype.html)、[luaaa](https://github.com/gengyong/luaaa)。

エンジンも同じバインディングAPIを使用します。値型と `Ptr<T>` リソースのuserdata管理・型チェックを共有し、複雑なコールバックやテーブル解析には `raw_function` と `stack<T>` を使用します。既存の `sn.Class.new(...)` は引き続き利用できます。型付きAPIの引数には上記の厳密な型変換が適用されます。

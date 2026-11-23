# C++ / Luau バインディング

`script/luau_bind.hpp` は Sinen 独自のヘッダーオンリーライブラリです。C++23 以降と、このリポジトリの Luau を使用します。

## lua_State を使わずに登録する

`state` はVMを所有し、`state_view` は既存VMを接続する境界で使用します。通常の登録関数は `table&` を受け取り、`lua_State*`、スタック番号、push/pop を扱いません。

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

`execute` はコンパイル済みバイトコードを受け取ります。通常の登録処理やプラグインにコンパイラ依存は不要です。`Luau::compile` を使う実行ファイルでは `Luau.Compiler` もリンクしてください。`get_table` は既存テーブルを取得し、`get_or_create_table` は未作成の場合だけ作成します。既存値がテーブル以外ならエラーになります。`table["key"]` は値・関数・テーブルの代入や別のプロキシからのコピーに対応し、明示的な読み出しには `.get<T>()` を使えます。

## C++20/23を使ったコンパイル時登録

登録する関数やメンバーがコンパイル時に決まっている場合は、テンプレート引数に渡せます。

```cpp
module.function<&sum>("sum");
module.class_<Counter>("Counter")
    .constructor<int>()
    .method<&Counter::add>("add")
    .field<&Counter::value>("value")
    .property<&Counter::get, &Counter::set>("amount");
module.function<[](int n) static { return n * 2; }>("double");
```

この形式では、コールバック保存用のuserdataやC++呼び出しオブジェクトのヒープ確保が不要です。引数検証と例外変換は従来と同じです。実行時の関数ポインタ、キャプチャ付きラムダ、`std::function` は従来の引数として渡す形式も使えます。呼び出し間でコールバックの状態を保持したい場合は、従来形式を使ってください。

`reference_field<&Transform::position>("position")` は借用getterと代入setterをまとめ、`transform.position.x = 1` のような変更を保ちます。通常の `field` は読み出し時にコピーします。借用するメンバーは参照userdataより長く生存させてください。constメンバーは読み取り専用です。Conceptsで関数シグネチャとコンストラクタを制約し、列挙値の変換にはC++23の `std::to_underlying` を使っています。

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

void registerCounter(sinen::luau::table &module) {
  module.function("sum", [](int a, int b) { return a + b; });
  module.class_<Counter>("Counter")
      .constructor<int>()
      .method<&Counter::add>("add")
      .field<&Counter::value>("value")
      .property<&Counter::get, &Counter::set>("amount")
      .readonly<&Counter::value>("readOnly");
}

SINEN_LUA_MODULE_EXPORT int sinen_lua_module_open(lua_State *L) {
  return sinen::luau::open_plugin(L, "counter", registerCounter);
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
| `state` / `state_view(existingState)` | VMの所有 / 借用 |
| `vm["name"] = valueOrFunction` | グローバルの登録・参照 |
| `.get_table(name)` / `.get_or_create_table(name)` | サブテーブルの取得・作成 |
| `.get<T>(name)` / `[name].get<T>()` | 型付きの読み出し |
| `open_plugin(L, name, registerFunction)` | 登録関数に `table&` を渡すABIアダプタ |
| `table::create(L)` / `table(L, index)` | 新規テーブル / 既存テーブルの参照 |
| `.function<Function>(name)` / `.method<Function>(name)` | キャプチャ保存が不要なコンパイル時登録 |
| `.field<Member>(name)` / `.readonly<Member>(name)` | コンパイル時のメンバー登録 |
| `.reference_field<Member>(name)` | ネストした変更を反映するクラス型フィールドの借用 |
| `.property<Getter, Setter>(name)` | コンパイル時のアクセサ登録 |
| `.function(name, callable)` | 自由関数、ラムダ、`std::function` |
| `.raw_function(name, lua_CFunction)` | C API を直接使用する関数 |
| `.value(name, value)` / `.set_table(name, table)` | 値 / サブテーブル |
| `.publish(name)` / `.publish_plugin(name)` | グローバル / プラグインとして公開 |
| `.class_<T>(name)` | C++ クラスを登録 |
| `.constructor<Args...>()` | `Class.new(...)` と `Class(...)` |
| `.method(name, callable)` | メンバー関数、または第1引数に `T&` / `const T&` を取る関数 |
| `.static_function(name, callable)` | `Class.function(...)` |
| `.metamethod(name, callable)` | `__add` や `__tostring` などの型付き演算子 |
| `.raw_method` / `.raw_static_function` / `.raw_metamethod` | C API を直接扱う場合の登録 |
| `.static_value(name, value)` | クラスの定数 |
| `.metatable_name(name)` | 既存 C API と連携するレジストリ名 |
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

## 本体・プラグイン間でクラス型を共有する

提供側が型を名前とABIバージョン付きで公開し、利用側が同じVMの登録を取り込みます。取り込んだ型を `.class_<T>()` で再登録する必要はありません。

```cpp
// 提供側プラグイン。SharedValue の定義は共通ヘッダーに置きます。
module.class_<SharedValue>("SharedValue")
    .constructor<int>()
    .field<&SharedValue::value>("value")
    .export_type("example.SharedValue", 1);

// 利用側プラグイン。提供側のロード後に実行します。
module.import_type<SharedValue>("example.SharedValue", 1)
    .function("read", [](const SharedValue &v) { return v.value; })
    .function("copy", [](const SharedValue &v) { return SharedValue(v.value); });
```

`sinen::luau::import_type<T>(L, name, version)` と `state_view::import_type<T>(name, version)` も利用できます。バージョンの省略値は `1` です。登録はVMごとに独立しており、提供側が未ロードなら取り込み時にエラーになります。提供側を先にロードしてください。同名の別登録、バージョン、型のサイズ・アラインメント、userdata表現のABIが一致しない場合も拒否します。

エンジンの `engineClass` で登録した型は `sn.Vec2` などのメタテーブル名で公開されます。`game/fps` では次のように本体の `Vec2` を取り込み、`slab` から返します。返り値は本体と同じメタテーブルを使用するので、フィールド、演算子、`length()`、`copy()` も使用できます。

```cpp
module.import_type<sinen::Vec2>(sinen::Vec2::metaTableName());
module.function("slab", &sinen::generated::run); // float × 4 -> Vec2
```

この共有方式は互換性のあるC++ ABIを前提とします。共通ヘッダー、コンパイラ、ランタイムとビルド設定を揃え、型のレイアウトや意味を変更したときはABIバージョンを更新してください。サイズ・アラインメントの検査だけで、すべてのABI不一致を検出できるわけではありません。エンジン内部で `Ptr<T>` として登録されるリソースは、利用側も `Ptr<T>` を取り込んでください。

所有するオブジェクトは、そのオブジェクトを生成したバイナリのuserdata destructorで破棄します。取り込みによって所有権は移りません。`std::ref` / `std::cref` の借用先は、それを参照する全userdataより長く生存させてください。提供側と利用側のプラグインは、VMを閉じるまでロード状態を維持する必要があります。

## 寿命とエラー

`table`、`class_builder`、添字プロキシはレジストリ参照を保持し、スタックの位置に依存しません。所有する `state` をこれらのハンドルより先に宣言し、ハンドルを先に破棄してください。`state_view` はVMを閉じません。借用VMでは `lua_close` より前にハンドルを破棄してください。登録された関数やクラスはハンドル破棄後も有効です。プラグインのDLLはVM終了後にアンロードしてください。既存の Sinen ローダーはこの順序を保証します。

通常の登録操作はスタックの高さを維持します。`.push()` だけは明示的にテーブルを1つ積みます。C++例外は Luau エラーへ変換され、スクリプトの `pcall` で処理できます。登録時は `sinen::luau::open` を使用してください。`raw_function` の関数はスタックと戻り値の数を管理します。C++例外は型付き関数と同様に Luau エラーへ変換されます。

Luau とプラグインの両方でC++例外を有効にし、`LUA_USE_LONGJMP=0` を使用してください。デストラクタは例外を投げないことが前提です。自動継承、Luau関数からC++コールバックへの変換、yieldをまたぐC++呼び出しは対象外です。

登録の実例は [DLLテストプラグイン](https://github.com/astomih/sinen/blob/main/tests/cpp/native_module_fixture.cpp)、動作仕様は [テスト](https://github.com/astomih/sinen/blob/main/tests/cpp/luau_bind_library_tests.cpp) にあります。

API設計の参考: [sol2 usertype](https://sol2.readthedocs.io/en/latest/api/usertype.html)、[luaaa](https://github.com/gengyong/luaaa)。

エンジンも同じバインディングAPIを使用します。値型と `Ptr<T>` リソースのuserdata管理・型チェックを共有し、複雑なコールバックやテーブル解析には `raw_function` と `stack<T>` を使用します。既存の `sn.Class.new(...)` は引き続き利用できます。型付きAPIの引数には上記の厳密な型変換が適用されます。

#include <Engine.hpp>
namespace nen
{
	void Script::Create()
	{
		instance = std::make_unique<Script>();
		lua.open_libraries(sol::lib::base, sol::lib::package);
		// ユーザー定義型の登録
		lua.new_usertype<Vector3f>(
			// 型名
			"vec3",
			//コンストラクタ
			sol::constructors<sol::types<>, sol::types<float, float, float>>(),
			// データメンバ
			"x", &Vector3f::x,
			"y", &Vector3f::y,
			"z", &Vector3f::z);
		lua.new_usertype<Quaternion>(
			// 型名
			"quat",
			//コンストラクタ
			sol::constructors<sol::types<>, sol::types<Vector3f, float>>(),
			// メンバ関数
			"concatenate", &Quaternion::Concatenate,
			// データメンバ
			"x", &Quaternion::x,
			"y", &Quaternion::y,
			"z", &Quaternion::z);
	}

	void Script::DoScript(std::string_view fileName)
	{
		lua.script_file(fileName.data());
	}

}

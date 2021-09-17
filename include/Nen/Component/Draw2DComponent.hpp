#pragma once
#include "../Math/Vector2.hpp"
#include "../Texture/Texture.hpp"
#include "../Color/Color.hpp"
#include "Component.hpp"
#include <utility>

namespace nen
{
	class Draw2DComponent : public Component
	{
	public:
		/**
		 * @brief コンストラクタ
		 * 
		 * @param owner 親となるアクター
		 * @param drawOrder 描画順序
		 * @param tex テクスチャ
		 */
		Draw2DComponent(class Actor &owner, int drawOrder = 100);
		~Draw2DComponent();

		/**
		 * @brief Actorから呼ばれる更新関数
		 * 
		 * @param deltaTime 変位時間
		 */
		virtual void Update(float deltaTime) override;

		/**
		 * @brief DrawObjectを使用する
		 * 
		 * @param texture 使用するテクスチャ
		 * @param scale スケール
		 * @param shape 形
		 */
		virtual void Create(std::shared_ptr<Texture> texture, float scale = 1.0f, std::string_view shape = "SPRITE");

		/**
		 * @brief 生成したDrawObjectを登録する
		 * 
		 */
		void Register();

		int GetDrawOrder() const { return mDrawOrder; }
		int GetTexHeight() const { return mTexHeight; }
		int GetTexWidth() const { return mTexWidth; }

		std::shared_ptr<class DrawObject> GetSprite() { return this->sprite; }

		void SetScale(const Vector2 &scale)
		{
			mTexWidth = static_cast<int>(mTexWidth * scale.x);
			mTexHeight = static_cast<int>(mTexHeight * scale.y);
		}

	private:
		std::shared_ptr<class DrawObject> sprite;
		int mDrawOrder = 0;
		int mTexWidth = 0;
		int mTexHeight = 0;
		std::shared_ptr<class Texture> mTexture;
	};
}

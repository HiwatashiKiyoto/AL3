#define NOMINMAX
#include "Player.h"
#include "WorldTransformConfig.h"
#include <numbers>
#include<algorithm>

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) 
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {

	// 移動入力
	//左右移動操作
	if (onGround_) {

		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) 
		{
			// 左右加速
			KamataEngine::Vector3 acceleration = {};
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT)) 
			{
				if (velocity_.x < 0.0f) 
				{
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) 
				{
					lrDirection_ = LRDirection::kLeft;

					turnFirstRotationY_ = worldTransform_.rotation_.y;

					turnTimer_ = kTimerTurn;
				}
			}
			else if (KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT)) 
			{
				if (velocity_.x > 0.0f) 
				{
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kRight) 
				{
					lrDirection_ = LRDirection::kRight;

					turnFirstRotationY_ = worldTransform_.rotation_.y;

					turnTimer_ = kTimerTurn;
				}
			}

			// 加速/減速
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;

			{
				// 左右の自キャラ角度テーブル
				float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
				// 状態に応じた角度を取得する
				float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

				// 自キャラの角度を設定する
				worldTransform_.rotation_.y = destinationRotationY;
			}

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} 
		else 
		{
			// 日入力時は移動減衰
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_UP))
		{
			// ジャンプ初速
			velocity_.y += kJumpAcceleration;
		}
	} 
	else
	{
		//落下速度
		velocity_.y -= kGravityAcceleration;

		//落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

	}
	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;

	if (turnTimer_ > 0.0f)
	{
		turnTimer_ -= 1.0f / 60.0f;
		if (turnTimer_ <= 0.0f)
		{
			turnTimer_ = 0.0f;
		}

		float destinationRotationYTable[] = 
		{
		    std::numbers::pi_v<float> / 2.0f,       // kRight
		    std::numbers::pi_v<float> * 3.0f / 2.0f // kLeft
		};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 割合 t の計算 (0.0f ～ 1.0f)
		float t = 1.0f - (turnTimer_ / kTimerTurn);

		// 角度を補間して代入
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * t;
	}

	//着地フラグ
	bool landing = false;

	//地面との当たり判定
	//下降中？
	if (velocity_.y < 0)
	{
		//Y座標が地面以下のなったら着地
		if (worldTransform_.translation_.y <= 1.0f)
		{
			landing = true;
		}
	}

	//設置判定
	if (onGround_)
	{
		//ジャンプ開始
		if (velocity_.y > 0.0f)
		{
			//空中状態に移行
			onGround_ = false;
		}
	} 
	else
	{
		//着地
		if (landing)
		{
			//めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			//摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			//下方向速度をリセット
			velocity_.y = 0.0f;
			//接地状態に移行
			onGround_ = true;
		}
	}
	
	// 行列更新

	WorldTransformConfig(worldTransform_);

}

void Player::Draw(const KamataEngine::Camera& camera) {
	KamataEngine::Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	KamataEngine::Model::PostDraw();
}


///20pまで終わり
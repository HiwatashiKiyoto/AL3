#define NOMINMAX
#include "Player.h"
#include "WorldTransformConfig.h"
#include <numbers>
#include<algorithm>
#include"MapChipField.h"

KamataEngine::Vector3 operator+(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) { return KamataEngine::Vector3{v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) 
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update()
{

	MoveInput();

	//衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	//移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	//マップ衝突チェック
	CheckMapCollision(collisionMapInfo);

	// 移動
	worldTransform_.translation_.x += collisionMapInfo.move.x;
	worldTransform_.translation_.y += collisionMapInfo.move.y;

	//状態フラグを更新
	onGround_ = collisionMapInfo.isGrounded;

	if (collisionMapInfo.isCeilingHit || collisionMapInfo.isGrounded)
	{
		velocity_.y = 0.0f;
	}
	if (collisionMapInfo.isWallHit)
	{
		velocity_.x = 0.0f;
	}



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

void Player::Draw(const KamataEngine::Camera& camera) 
{
	KamataEngine::Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	KamataEngine::Model::PostDraw();
}

void Player::CheckMapCollision(CollisionMapInfo& info)
{
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) 
{
	// 上に移動している場合のみ判定する
	if (info.move.y > 0.0f) {
		// 移動後の中心座標を計算（Y軸のみ移動させる）
		KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
		nextCenter.y += info.move.y;

		// 左上と右上の角の座標を取得
		KamataEngine::Vector3 leftTop = CornerPosition(nextCenter, kLeftTop);
		KamataEngine::Vector3 rightTop = CornerPosition(nextCenter, kRightTop);

		// 壁への引っ掛かり防止（前のコードにあった 0.1f の処理です）
		leftTop.x += 0.1f;
		rightTop.x -= 0.1f;

		// 座標をマップのインデックス(何マス目か)に変換
		uint32_t yIndex = static_cast<uint32_t>(leftTop.y / MapChipField::kBlockHeight);
		uint32_t xIndexLeft = static_cast<uint32_t>(leftTop.x / MapChipField::kBlockWidth);
		uint32_t xIndexRight = static_cast<uint32_t>(rightTop.x / MapChipField::kBlockWidth);

		// ブロックと重なっているかチェック
		if (mapChipField_->GetMapChipTypeByIndex(xIndexLeft, yIndex) == MapChipType::kBlock || mapChipField_->GetMapChipTypeByIndex(xIndexRight, yIndex) == MapChipType::kBlock) {
			info.isCeilingHit = true; // 天井に当たった！
			info.move.y = 0.0f;       // これ以上上に移動させない
		}
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) 
{
	// 下に移動している場合のみ判定する
	if (info.move.y < 0.0f) {
		// 移動後の中心座標を計算
		KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
		nextCenter.y += info.move.y;

		// 左下と右下の角の座標を取得
		KamataEngine::Vector3 leftBottom = CornerPosition(nextCenter, kLeftBottom);
		KamataEngine::Vector3 rightBottom = CornerPosition(nextCenter, kRightBottom);

		// 壁への引っ掛かり防止
		leftBottom.x += 0.1f;
		rightBottom.x -= 0.1f;

		// インデックスに変換
		uint32_t yIndex = static_cast<uint32_t>(leftBottom.y / MapChipField::kBlockHeight);
		uint32_t xIndexLeft = static_cast<uint32_t>(leftBottom.x / MapChipField::kBlockWidth);
		uint32_t xIndexRight = static_cast<uint32_t>(rightBottom.x / MapChipField::kBlockWidth);

		// ブロックと重なっているかチェック
		if (mapChipField_->GetMapChipTypeByIndex(xIndexLeft, yIndex) == MapChipType::kBlock || mapChipField_->GetMapChipTypeByIndex(xIndexRight, yIndex) == MapChipType::kBlock) {
			info.isGrounded = true; // 着地した！
			info.move.y = 0.0f;     // これ以上下に落ちないようにする
		}
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) 
{
	// 右に移動している場合のみ判定する
	if (info.move.x > 0.0f) {
		// 移動後の中心座標を計算（X軸のみ移動させる）
		KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
		nextCenter.x += info.move.x;

		// 右上と右下の角の座標を取得
		KamataEngine::Vector3 rightTop = CornerPosition(nextCenter, kRightTop);
		KamataEngine::Vector3 rightBottom = CornerPosition(nextCenter, kRightBottom);

		// 天井や床への引っ掛かり防止
		rightTop.y -= 0.1f;
		rightBottom.y += 0.1f;

		// インデックスに変換
		uint32_t xIndex = static_cast<uint32_t>(rightTop.x / MapChipField::kBlockWidth);
		uint32_t yIndexTop = static_cast<uint32_t>(rightTop.y / MapChipField::kBlockHeight);
		uint32_t yIndexBottom = static_cast<uint32_t>(rightBottom.y / MapChipField::kBlockHeight);

		// ブロックと重なっているかチェック
		if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndexTop) == MapChipType::kBlock || mapChipField_->GetMapChipTypeByIndex(xIndex, yIndexBottom) == MapChipType::kBlock) {
			info.isWallHit = true; // 壁に当たった！
			info.move.x = 0.0f;    // 右への移動を止める
		}
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) 
{
	// 左に移動している場合のみ判定する
	if (info.move.x < 0.0f) {
		// 移動後の中心座標を計算
		KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
		nextCenter.x += info.move.x;

		// 左上と左下の角の座標を取得
		KamataEngine::Vector3 leftTop = CornerPosition(nextCenter, kLeftTop);
		KamataEngine::Vector3 leftBottom = CornerPosition(nextCenter, kLeftBottom);

		// 天井や床への引っ掛かり防止
		leftTop.y -= 0.1f;
		leftBottom.y += 0.1f;

		// インデックスに変換
		uint32_t xIndex = static_cast<uint32_t>(leftTop.x / MapChipField::kBlockWidth);
		uint32_t yIndexTop = static_cast<uint32_t>(leftTop.y / MapChipField::kBlockHeight);
		uint32_t yIndexBottom = static_cast<uint32_t>(leftBottom.y / MapChipField::kBlockHeight);

		// ブロックと重なっているかチェック
		if (mapChipField_->GetMapChipTypeByIndex(xIndex, yIndexTop) == MapChipType::kBlock || mapChipField_->GetMapChipTypeByIndex(xIndex, yIndexBottom) == MapChipType::kBlock) {
			info.isWallHit = true; // 壁に当たった！
			info.move.x = 0.0f;    // 左への移動を止める
		}
	}
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) 
{
	if (corner == kRightBottom)
	{
		return center + KamataEngine::Vector3{+kWidth / 2.0f, -kHeight / 2.0f, 0.0f};
	} 
	else if (corner == kLeftBottom)
	{
		return center + KamataEngine::Vector3{-kWidth / 2.0f, -kHeight / 2.0f, 0.0f};
	} 
	else if (corner == kRightTop)
	{
		return center + KamataEngine::Vector3{+kWidth / 2.0f, +kHeight / 2.0f, 0.0f};
	}
	else 
	{
		return center + KamataEngine::Vector3{-kWidth / 2.0f, +kHeight / 2.0f, 0.0f};
	}

}

void Player::MoveInput() 
{
	// 移動入力
	// 左右移動操作
	if (onGround_) 
	{
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
		// 落下速度
		velocity_.y -= kGravityAcceleration;

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}



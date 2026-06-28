#define NOMINMAX
#include "Player.h"
#include "Enemy.h"
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
	if (isDead_)
	{
		return;
	}

	MoveInput();

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.move = velocity_;

	CheckMapCollision(collisionMapInfo);
	MoveByCollisionMapInfo(collisionMapInfo);
	UpdateOnCollision(collisionMapInfo);

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

		float t = 1.0f - (turnTimer_ / kTimerTurn);
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * t;
	}

	WorldTransformConfig(worldTransform_);

}

void Player::Draw(const KamataEngine::Camera& camera)
{
	KamataEngine::Model::PreDraw();
	model_->Draw(worldTransform_, camera);
	KamataEngine::Model::PostDraw();
}

KamataEngine::Vector3 Player::GetWorldPosition() const
{
	KamataEngine::Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Player::GetAABB() const
{
	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	return aabb;
}

void Player::OnCollision(const Enemy* enemy)
{
	(void)enemy;
	isDead_ = true;
}

void Player::CheckMapCollision(CollisionMapInfo& info)
{
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

void Player::MoveByCollisionMapInfo(const CollisionMapInfo& info)
{
	// 移動
	worldTransform_.translation_.x += info.move.x;
	worldTransform_.translation_.y += info.move.y;
	worldTransform_.translation_.z += info.move.z;
}

void Player::UpdateOnCollision(const CollisionMapInfo& info)
{
	if (info.isCeilingHit)
	{
		KamataEngine::DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0.0f;
	}

	if (info.isWallHit)
	{
		velocity_.x *= (1.0f - kAttenuationWall);
	}

	if (onGround_)
	{
		if (velocity_.y > 0.0f)
		{
			onGround_ = false;
		}
		else if (!info.isGrounded)
		{
			onGround_ = false;
		}
	}
	else
	{
		if (info.isGrounded)
		{
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info)
{
	if (info.move.y <= 0.0f)
	{
		return;
	}

	KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
	nextCenter.x += info.move.x;
	nextCenter.y += info.move.y;

	KamataEngine::Vector3 leftTop = CornerPosition(nextCenter, kLeftTop);
	KamataEngine::Vector3 rightTop = CornerPosition(nextCenter, kRightTop);
	leftTop.x += kCollisionMargin;
	rightTop.x -= kCollisionMargin;

	KamataEngine::Vector3 leftTopNow = CornerPosition(worldTransform_.translation_, kLeftTop);
	KamataEngine::Vector3 rightTopNow = CornerPosition(worldTransform_.translation_, kRightTop);
	leftTopNow.x += kCollisionMargin;
	rightTopNow.x -= kCollisionMargin;

	IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(leftTop);
	IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(rightTop);
	IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(leftTopNow);
	IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(rightTopNow);

	bool hit = false;
	IndexSet indexSet{};
	if (mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex) == MapChipType::kBlock &&
	    mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex + 1) != MapChipType::kBlock &&
	    indexSetLeftNow.yIndex != indexSetLeft.yIndex)
	{
		hit = true;
		indexSet = indexSetLeft;
	}
	else if (mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex) == MapChipType::kBlock &&
	         mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex + 1) != MapChipType::kBlock &&
	         indexSetRightNow.yIndex != indexSetRight.yIndex)
	{
		hit = true;
		indexSet = indexSetRight;
	}

	if (hit)
	{
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		float currentTop = CornerPosition(worldTransform_.translation_, kLeftTop).y;
		info.move.y = std::min(info.move.y, rect.bottom - currentTop - kBlank);
		info.isCeilingHit = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info)
{
	if (info.move.y > 0.0f)
	{
		return;
	}

	KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
	nextCenter.x += info.move.x;
	nextCenter.y += info.move.y;

	KamataEngine::Vector3 leftBottom = CornerPosition(nextCenter, kLeftBottom);
	KamataEngine::Vector3 rightBottom = CornerPosition(nextCenter, kRightBottom);
	leftBottom.x += kCollisionMargin;
	rightBottom.x -= kCollisionMargin;
	leftBottom.y -= kBlank * 2.0f;
	rightBottom.y -= kBlank * 2.0f;

	KamataEngine::Vector3 leftBottomNow = CornerPosition(worldTransform_.translation_, kLeftBottom);
	KamataEngine::Vector3 rightBottomNow = CornerPosition(worldTransform_.translation_, kRightBottom);
	leftBottomNow.x += kCollisionMargin;
	rightBottomNow.x -= kCollisionMargin;

	IndexSet indexSetLeft = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
	IndexSet indexSetRight = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
	IndexSet indexSetLeftNow = mapChipField_->GetMapChipIndexSetByPosition(leftBottomNow);
	IndexSet indexSetRightNow = mapChipField_->GetMapChipIndexSetByPosition(rightBottomNow);

	bool hit = false;
	IndexSet indexSet{};
	if (mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex) == MapChipType::kBlock &&
	    mapChipField_->GetMapChipTypeByIndex(indexSetLeft.xIndex, indexSetLeft.yIndex - 1) != MapChipType::kBlock &&
	    indexSetLeftNow.yIndex != indexSetLeft.yIndex)
	{
		hit = true;
		indexSet = indexSetLeft;
	}
	else if (mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex) == MapChipType::kBlock &&
	         mapChipField_->GetMapChipTypeByIndex(indexSetRight.xIndex, indexSetRight.yIndex - 1) != MapChipType::kBlock &&
	         indexSetRightNow.yIndex != indexSetRight.yIndex)
	{
		hit = true;
		indexSet = indexSetRight;
	}

	if (hit)
	{
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		float currentBottom = CornerPosition(worldTransform_.translation_, kLeftBottom).y;
		info.move.y = std::max(info.move.y, rect.top - currentBottom + kBlank);
		info.isGrounded = true;
	}
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info)
{
	if (info.move.x <= 0.0f)
	{
		return;
	}

	KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
	nextCenter.x += info.move.x;
	nextCenter.y += info.move.y;

	KamataEngine::Vector3 rightTop = CornerPosition(nextCenter, kRightTop);
	KamataEngine::Vector3 rightBottom = CornerPosition(nextCenter, kRightBottom);
	rightTop.y -= kCollisionMargin;
	rightBottom.y += kCollisionMargin;

	KamataEngine::Vector3 rightTopNow = CornerPosition(worldTransform_.translation_, kRightTop);
	KamataEngine::Vector3 rightBottomNow = CornerPosition(worldTransform_.translation_, kRightBottom);
	rightTopNow.y -= kCollisionMargin;
	rightBottomNow.y += kCollisionMargin;

	IndexSet indexSetTop = mapChipField_->GetMapChipIndexSetByPosition(rightTop);
	IndexSet indexSetBottom = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
	IndexSet indexSetTopNow = mapChipField_->GetMapChipIndexSetByPosition(rightTopNow);
	IndexSet indexSetBottomNow = mapChipField_->GetMapChipIndexSetByPosition(rightBottomNow);

	bool hit = false;
	IndexSet indexSet{};
	if (mapChipField_->GetMapChipTypeByIndex(indexSetTop.xIndex, indexSetTop.yIndex) == MapChipType::kBlock &&
	    mapChipField_->GetMapChipTypeByIndex(indexSetTop.xIndex - 1, indexSetTop.yIndex) != MapChipType::kBlock &&
	    indexSetTopNow.xIndex != indexSetTop.xIndex)
	{
		hit = true;
		indexSet = indexSetTop;
	}
	else if (mapChipField_->GetMapChipTypeByIndex(indexSetBottom.xIndex, indexSetBottom.yIndex) == MapChipType::kBlock &&
	         mapChipField_->GetMapChipTypeByIndex(indexSetBottom.xIndex - 1, indexSetBottom.yIndex) != MapChipType::kBlock &&
	         indexSetBottomNow.xIndex != indexSetBottom.xIndex)
	{
		hit = true;
		indexSet = indexSetBottom;
	}

	if (hit)
	{
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		float currentRight = CornerPosition(worldTransform_.translation_, kRightTop).x;
		info.move.x = std::min(info.move.x, rect.left - currentRight - kBlank);
		info.isWallHit = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info)
{
	if (info.move.x >= 0.0f)
	{
		return;
	}

	KamataEngine::Vector3 nextCenter = worldTransform_.translation_;
	nextCenter.x += info.move.x;
	nextCenter.y += info.move.y;

	KamataEngine::Vector3 leftTop = CornerPosition(nextCenter, kLeftTop);
	KamataEngine::Vector3 leftBottom = CornerPosition(nextCenter, kLeftBottom);
	leftTop.y -= kCollisionMargin;
	leftBottom.y += kCollisionMargin;

	KamataEngine::Vector3 leftTopNow = CornerPosition(worldTransform_.translation_, kLeftTop);
	KamataEngine::Vector3 leftBottomNow = CornerPosition(worldTransform_.translation_, kLeftBottom);
	leftTopNow.y -= kCollisionMargin;
	leftBottomNow.y += kCollisionMargin;

	IndexSet indexSetTop = mapChipField_->GetMapChipIndexSetByPosition(leftTop);
	IndexSet indexSetBottom = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
	IndexSet indexSetTopNow = mapChipField_->GetMapChipIndexSetByPosition(leftTopNow);
	IndexSet indexSetBottomNow = mapChipField_->GetMapChipIndexSetByPosition(leftBottomNow);

	bool hit = false;
	IndexSet indexSet{};
	if (mapChipField_->GetMapChipTypeByIndex(indexSetTop.xIndex, indexSetTop.yIndex) == MapChipType::kBlock &&
	    mapChipField_->GetMapChipTypeByIndex(indexSetTop.xIndex + 1, indexSetTop.yIndex) != MapChipType::kBlock &&
	    indexSetTopNow.xIndex != indexSetTop.xIndex)
	{
		hit = true;
		indexSet = indexSetTop;
	}
	else if (mapChipField_->GetMapChipTypeByIndex(indexSetBottom.xIndex, indexSetBottom.yIndex) == MapChipType::kBlock &&
	         mapChipField_->GetMapChipTypeByIndex(indexSetBottom.xIndex + 1, indexSetBottom.yIndex) != MapChipType::kBlock &&
	         indexSetBottomNow.xIndex != indexSetBottom.xIndex)
	{
		hit = true;
		indexSet = indexSetBottom;
	}

	if (hit)
	{
		Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		float currentLeft = CornerPosition(worldTransform_.translation_, kLeftTop).x;
		info.move.x = std::max(info.move.x, rect.right - currentLeft + kBlank);
		info.isWallHit = true;
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
	if (onGround_)
	{
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT) || KamataEngine::Input::GetInstance()->PushKey(DIK_LEFT))
		{
			KamataEngine::Vector3 acceleration = {};
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_RIGHT))
			{
				if (velocity_.x < 0.0f)
				{
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

			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;

			{
				float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
				float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
				worldTransform_.rotation_.y = destinationRotationY;
			}

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		}
		else
		{
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (KamataEngine::Input::GetInstance()->PushKey(DIK_UP))
		{
			velocity_.y += kJumpAcceleration;
		}
	}
	else
	{
		velocity_.y -= kGravityAcceleration;
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

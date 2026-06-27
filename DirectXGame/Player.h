#pragma once
/// <summary>
/// Player
/// </summary>

#include "KamataEngine.h"
#include "Collision.h"

class MapChipField;
class Enemy;

class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	KamataEngine::Vector3 GetWorldPosition() const;
	AABB GetAABB() const;
	void OnCollision(const Enemy* enemy);

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	//マップとの当たり判定情報
	struct CollisionMapInfo
	{
		bool isCeilingHit = false;
		bool isGrounded = false;
		bool isWallHit = false;
		KamataEngine::Vector3 move;
	};

	void CheckMapCollision(CollisionMapInfo& info);
	void MoveByCollisionMapInfo(const CollisionMapInfo& info);
	void UpdateOnCollision(const CollisionMapInfo& info);

	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	enum Corner
	{
		kRightBottom,	//右下
		kLeftBottom,	//左下
		kRightTop,		//右上
		kLeftTop,		//左上

		kNumCorner		//要素数
	};

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

private:

	KamataEngine::Camera* camera_ = nullptr;

	//左右
	enum class LRDirection
	{
		kRight,
		kLeft,
	};

	LRDirection lrDirection_ = LRDirection::kRight;

	//ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	//モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Vector3 velocity_ = {};

	static inline const float kAcceleration = 0.01f;

	static inline const float kAttenuation = 0.01f;
	static inline const float kAttenuationLanding = 0.01f;
	static inline const float kAttenuationWall = 0.01f;

	static inline const float kLimitRunSpeed = 2.0f;

	//旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	//旋回タイマー
	float turnTimer_ = 0.0f;

	//旋回時間<秒>
	static inline const float kTimerTurn = 0.3f;

	//設置状態フラグ
	bool onGround_ = true;

	//重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.04f;

	//最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.4f;

	//ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.5f;

	//マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	//キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.01f;
	static inline const float kCollisionMargin = 0.1f;

	void MoveInput();

};

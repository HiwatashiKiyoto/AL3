#pragma once
/// <summary>
/// Player
/// </summary>

#include "KamataEngine.h"

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

};

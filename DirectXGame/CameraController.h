#pragma once
#include "KamataEngine.h"

// 前方宣言
class Player;

/// <summary>
/// カメラコントロール
/// </summary>
class CameraController {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// 追従対象（プレイヤー）を設定する関数
	void SetTarget(Player* target) { target_ = target; }

	// カメラ本体を取得する関数
	KamataEngine::Camera* GetCamera() const { return camera_; }

	// 初期位置へのリセット関数
	void Reset();

	struct Rect 
	{
		float left = 0.0f;		//左端
		float right = 1.0f;		//右端
		float bottom = 0.0f;	//下端
		float top = 1.0f;		//上端
	};

	//カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 100};

	void SetMovableArea(Rect area) { movableArea_ = area; }

private:
	// カメラ本体
	KamataEngine::Camera* camera_ = nullptr;

	// 追従対象（プレイヤー）へのポインタ
	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差(オフセット)
	KamataEngine::Vector3 targetOffset_ = {0.0f, 0.0f, -15.0f};
};
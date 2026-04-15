#pragma once
#include "KamataEngine.h"
#include"Player.h"

using namespace KamataEngine;

// ゲームシーン
class GameScene {
public:

	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Updata();

	// 描画
	void Draw();


private:

	//テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;

	//デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	KamataEngine::Camera camera_;
	
	Player* player_ = nullptr;
};

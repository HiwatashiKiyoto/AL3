#pragma once
#include "KamataEngine.h"
#include <vector>

// ゲームシーン
class GameScene 
{
public:
	~GameScene();
	// 初期化
	void Initialize();

	// 更新
	void Updata();

	// 描画
	void Draw();

private:
	// 3Dモデル
	KamataEngine::Model* modelBlock_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera* camera_;

	//デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	//デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;


};

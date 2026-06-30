#pragma once

#include "KamataEngine.h"
#include "Player.h"

// Game scene
class GameScene
{
public:
	~GameScene();

	// Initialize
	void Initialize();

	// Update
	void Updata();

	// Draw
	void Draw();

private:
	// Texture handle
	uint32_t textureHandle_ = 0u;

	// 3D model data
	KamataEngine::Model* model_ = nullptr;

	// View projection
	KamataEngine::Camera camera_;

	// Player
	Player* player_ = nullptr;

#ifdef _DEBUG
	// Debug camera
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// Debug camera enabled
	bool isDebugCameraActive_ = false;
#endif
};

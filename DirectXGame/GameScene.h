#pragma once

#include "KamataEngine.h"
#include "Enemy.h"
#include "Player.h"
#include "Skydome.h"

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
	// Check all collisions
	void CheckAllCollisions();

private:
	// Texture handle
	uint32_t textureHandle_ = 0u;

	// 3D model data
	KamataEngine::Model* model_ = nullptr;

	// Player model data
	KamataEngine::Model* modelPlayer_ = nullptr;

	// Skydome model data
	KamataEngine::Model* modelSkydome_ = nullptr;

	// View projection
	KamataEngine::Camera camera_;

	// Skydome
	Skydome* skydome_ = nullptr;

	// Player
	Player* player_ = nullptr;

	// Enemy
	Enemy* enemy_ = nullptr;

#ifdef _DEBUG
	// Debug camera
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// Debug camera enabled
	bool isDebugCameraActive_ = false;
#endif
};

#pragma once

#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "Skydome.h"
#include <list>
#include <vector>

class GameScene
{
public:
	~GameScene();

	void Initialize();
	void Update();
	void Draw();

private:
	KamataEngine::Model* modelBlock_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;
	CameraController* cameraController_ = nullptr;

	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome* skydome_ = nullptr;

	KamataEngine::Model* modelPlayer_ = nullptr;
	Player* player_ = nullptr;

	KamataEngine::Model* modelEnemy_ = nullptr;
	std::list<Enemy*> enemies_;

	KamataEngine::Model* modelDeathParticles_ = nullptr;
	DeathParticles* deathParticles_ = nullptr;

	MapChipField* mapChipField_ = nullptr;

	void GenerateBlocks();
	void CheckAllCollisions();
};

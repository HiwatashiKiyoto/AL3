#pragma once

#include "KamataEngine.h"
#include "Enemy.h"
#include "RailCameraController.h"
#include "Skydome.h"

#include <list>
#include <sstream>

class LockOn;
class Player;

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

	// Add enemy bullet
	void AddEnemyBullet(EnemyBullet* enemyBullet);

private:
	// Check all collisions
	void CheckAllCollisions();

	// Spawn enemy
	void SpawnEnemy(const KamataEngine::Vector3& position);

	// Load enemy pop data
	void LoadEnemyPopData();

	// Update enemy pop commands
	void UpdateEnemyPopCommands();

private:
	// Reticle texture handle
	uint32_t textureReticle_ = 0u;

	// 3D model data
	KamataEngine::Model* model_ = nullptr;

	// Player model data
	KamataEngine::Model* modelPlayer_ = nullptr;

	// Skydome model data
	KamataEngine::Model* modelSkydome_ = nullptr;

	// View projection
	KamataEngine::Camera camera_;

	// Rail camera
	RailCameraController* railCamera_ = nullptr;

	// Skydome
	Skydome* skydome_ = nullptr;

	// Player
	Player* player_ = nullptr;

	// Lock-on system
	LockOn* lockOn_ = nullptr;

	// Enemies
	std::list<Enemy*> enemies_;

	// Enemy bullets
	std::list<EnemyBullet*> enemyBullets_;

	// Enemy pop commands
	std::stringstream enemyPopCommands_;

	// Waiting flag
	bool isWaitingEnemyPop_ = false;

	// Waiting timer
	int32_t enemyPopWaitTimer_ = 0;

#ifdef _DEBUG
	// Debug camera
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// Debug camera enabled
	bool isDebugCameraActive_ = false;
#endif
};

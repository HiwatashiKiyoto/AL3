#pragma once

#include "KamataEngine.h"
#include "Enemy.h"
#include "RailCameraController.h"
#include "Skydome.h"

#include <list>
#include <sstream>

class LockOn;
class Player;
class AnimatedModel;
class Boss;

// Game scene
class GameScene
{
public:
	~GameScene();

	// Initialize
	void Initialize(bool tutorialMode = false, AnimatedModel* preparedEnemyModel = nullptr);

	// Update
	void Updata();

	// Draw
	void Draw();

	// Whether the player's health reached zero
	bool IsGameOver() const;

	// Whether every scripted enemy has appeared and been defeated
	bool IsGameClear() const;
	bool IsDangerActive() const { return dangerSequenceActive_; }
	int32_t GetTutorialStep() const { return tutorialStep_; }
	bool IsTutorialComplete() const { return tutorialComplete_; }

	// Add enemy bullet
	void AddEnemyBullet(EnemyBullet* enemyBullet);

private:
	// Check all collisions
	void CheckAllCollisions();

	// Spawn enemy
	void SpawnEnemy(const KamataEngine::Vector3& position, Enemy::BehaviorPattern behaviorPattern);

	// Load enemy pop data
	void LoadEnemyPopData();

	// Update enemy pop commands
	void UpdateEnemyPopCommands();
	void UpdateTutorial();
	void SpawnTrainingDummy();
	void UpdateDangerSequence();
	void SpawnBoss();
	void UpdateBossSequence(bool advanceBattle);

private:
	// Reticle texture handle
	uint32_t textureReticle_ = 0u;
	uint32_t textureLockOnReticle_ = 0u;
	uint32_t textureHealthBack_ = 0u;
	uint32_t textureHealthFill_ = 0u;
	uint32_t textureDanger_ = 0u;
	uint32_t textureBossName_ = 0u;
	uint32_t textureObjectiveEnemies_ = 0u;
	uint32_t textureObjectiveBoss_ = 0u;
	KamataEngine::Sprite* spriteHealthBack_ = nullptr;
	KamataEngine::Sprite* spriteHealthFill_ = nullptr;
	KamataEngine::Sprite* spriteDanger_ = nullptr;
	KamataEngine::Sprite* spriteBossName_ = nullptr;
	KamataEngine::Sprite* spriteBossHpBack_ = nullptr;
	KamataEngine::Sprite* spriteBossHpFill_ = nullptr;
	KamataEngine::Sprite* spriteObjectiveEnemies_ = nullptr;
	KamataEngine::Sprite* spriteObjectiveBoss_ = nullptr;

	// 3D model data
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	AnimatedModel* modelEnemyAnimated_ = nullptr;
	KamataEngine::Model* modelBoss_ = nullptr;

	// Player model data
	KamataEngine::Model* modelPlayer_ = nullptr;

	// Player bullet model data
	KamataEngine::Model* modelPlayerBullet_ = nullptr;

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
	Boss* boss_ = nullptr;

	// Enemy pop commands
	std::stringstream enemyPopCommands_;

	// Waiting flag
	bool isWaitingEnemyPop_ = false;

	// Waiting timer
	int32_t enemyPopWaitTimer_ = 0;

	// Toggle for the exercise 03_12_ex1 rail visualization.
	bool showRailPath_ = false;

	// Debug camera
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// Debug camera enabled
	bool isDebugCameraActive_ = false;

	bool isTutorialMode_ = false;
	int32_t tutorialStep_ = 0;
	int32_t tutorialLockFrames_ = 0;
	bool tutorialComplete_ = false;
	KamataEngine::Vector3 tutorialPlayerStart_{};

	// Transition presentation between the regular enemies and the future boss.
	bool dangerSequenceActive_ = false;
	bool dangerSequenceFinished_ = false;
	int32_t dangerSequenceTimer_ = 0;

	enum class BossPhase
	{
		None,
		Roaring,
		Battle,
		Defeated,
	};
	BossPhase bossPhase_ = BossPhase::None;
	int32_t bossPhaseTimer_ = 0;
};

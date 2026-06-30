#pragma once

#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "GuardEffect.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "Player.h"
#include "ShieldEnemy.h"
#include "Skydome.h"
#include "StageManager.h"
#include <list>
#include <vector>

class GameScene
{
public:
	~GameScene();

	void Initialize(StageManager* stageManager);
	void Update();
	void Draw();
	bool IsFinished() const { return finished_; }
	bool IsReloadRequested() const { return reloadRequested_; }
	void CreateHitEffect(const KamataEngine::Vector3& position);
	void CreateGuardEffect(const KamataEngine::Vector3& position);

private:
	enum class Phase
	{
		kFadeIn,
		kPlay,
		kDeath,
		kFadeOut,
	};

	static inline const float kFadeDuration = 1.0f;

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
	KamataEngine::Model* modelAttack_ = nullptr;
	Player* player_ = nullptr;

	KamataEngine::Model* modelEnemy_ = nullptr;
	std::list<Enemy*> enemies_;
	KamataEngine::Model* modelShieldEnemy_ = nullptr;
	std::list<ShieldEnemy*> shieldEnemies_;

	KamataEngine::Model* modelHitEffect_ = nullptr;
	std::list<HitEffect*> hitEffects_;
	KamataEngine::Model* modelGuardEffect_ = nullptr;
	std::list<GuardEffect*> guardEffects_;

	KamataEngine::Model* modelDeathParticles_ = nullptr;
	DeathParticles* deathParticles_ = nullptr;
	Fade* fade_ = nullptr;

	MapChipField* mapChipField_ = nullptr;
	StageManager* stageManager_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	bool finished_ = false;
	bool reloadRequested_ = false;

	void GenerateFieldObjects();
	void GenerateBlock(uint32_t xIndex, uint32_t yIndex);
	void GeneratePlayer(uint32_t xIndex, uint32_t yIndex);
	void GenerateEnemy(uint32_t xIndex, uint32_t yIndex, uint8_t subID);
	void ClearFieldObjects();
	void CheckAllCollisions();
	void RemoveDeadEnemies();
	void RemoveDeadHitEffects();
	void RemoveDeadShieldEnemies();
	void RemoveDeadGuardEffects();
	void ChangePhase();
	void UpdateDeathPhase();
	void UpdateFadeInPhase();
	void UpdateFadeOutPhase();
};

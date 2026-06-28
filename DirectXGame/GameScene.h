#pragma once

#include "CameraController.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
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
	bool IsFinished() const { return finished_; }

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

	KamataEngine::Model* modelDeathParticles_ = nullptr;
	DeathParticles* deathParticles_ = nullptr;
	Fade* fade_ = nullptr;

	MapChipField* mapChipField_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	bool finished_ = false;

	void GenerateBlocks();
	void CheckAllCollisions();
	void RemoveDeadEnemies();
	void ChangePhase();
	void UpdateDeathPhase();
	void UpdateFadeInPhase();
	void UpdateFadeOutPhase();
};

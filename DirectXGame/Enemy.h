#pragma once

#include "EnemyBullet.h"
#include "KamataEngine.h"

// Player class forward declaration
class Player;
// GameScene class forward declaration
class GameScene;
class AnimatedModel;

/// <summary>
/// Enemy
/// </summary>
class Enemy
{
public:
	~Enemy();

	// Movement pattern selected by the enemy-pop script.
	enum class BehaviorPattern
	{
		Straight = 0,
		MoveLeft,
		MoveRight,
		Zigzag,
	};

	// Behavior phase
	enum class Phase
	{
		Approach,
		Leave,
	};

	/// <summary>
	/// Initialize
	/// </summary>
	/// <param name="model">Model</param>
	/// <param name="position">Initial position</param>
	void Initialize(
	    KamataEngine::Model* model, KamataEngine::Model* bulletModel, AnimatedModel* animatedModel,
	    uint32_t modelTextureHandle, uint32_t bulletTextureHandle, const KamataEngine::Vector3& position,
	    BehaviorPattern behaviorPattern = BehaviorPattern::Straight);

	/// <summary>
	/// Update
	/// </summary>
	void Update();

	/// <summary>
	/// Draw
	/// </summary>
	/// <param name="camera">Camera</param>
	void Draw(const KamataEngine::Camera& camera);

	/// <summary>
	/// Get position
	/// </summary>
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	/// <summary>
	/// Get phase name
	/// </summary>
	const char* GetPhaseName() const;
	const char* GetBehaviorPatternName() const;

	// Set player
	void SetPlayer(Player* player) { player_ = player; }

	// Set game scene
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	// Training dummies stay still and never fire, but can still be locked on to and destroyed.
	void SetTrainingDummy(bool isTrainingDummy) { isTrainingDummy_ = isTrainingDummy; }

	// Get world position
	KamataEngine::Vector3 GetWorldPosition() const;

	// Is dead
	bool IsDead() const { return isDead_; }

	// Callback function called when collision is detected
	void OnCollision();

private:
	// Fire bullet
	void Fire();

	// Initialize approach phase
	void ApproachPhaseInitialize();

	// Update approach phase
	void UpdateApproach();

	// Update leave phase
	void UpdateLeave();

	// World transform data
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform worldTransformModel_;

	// Borrowed model data
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bulletModel_ = nullptr;
	AnimatedModel* animatedModel_ = nullptr;

	// Borrowed player data
	Player* player_ = nullptr;

	// Borrowed game scene
	GameScene* gameScene_ = nullptr;

	// Texture handle
	uint32_t textureHandle_ = 0u;
	uint32_t bulletTextureHandle_ = 0u;

	// Phase
	Phase phase_ = Phase::Approach;

	// Movement pattern selected by the script
	BehaviorPattern behaviorPattern_ = BehaviorPattern::Straight;
	int32_t behaviorTimer_ = 0;

	// Fire interval
	static const int kFireInterval = 60;

	// Fire timer
	int32_t fireTimer_ = 0;

	// Death flag
	bool isDead_ = false;
	bool isTrainingDummy_ = false;
};

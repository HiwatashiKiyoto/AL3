#include "GameScene.h"

#include "AnimatedModel.h"
#include "Boss.h"
#include "GameAudio.h"
#include "LockOn.h"
#include "Player.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <string>

using namespace KamataEngine;

namespace
{
const float kCameraFarZ = 2000.0f;

bool IsCollision(const Vector3& positionA, const Vector3& positionB, float radiusA, float radiusB)
{
	const float dx = positionB.x - positionA.x;
	const float dy = positionB.y - positionA.y;
	const float dz = positionB.z - positionA.z;
	const float distanceSquared = dx * dx + dy * dy + dz * dz;
	const float radiusSum = radiusA + radiusB;

	return distanceSquared <= radiusSum * radiusSum;
}

void RestoreFullScreenViewport()
{
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	const D3D12_VIEWPORT viewport = {
	    0.0f,
	    0.0f,
	    static_cast<float>(dxCommon->GetBackBufferWidth()),
	    static_cast<float>(dxCommon->GetBackBufferHeight()),
	    0.0f,
	    1.0f,
	};
	const D3D12_RECT scissorRect = {0, 0, dxCommon->GetBackBufferWidth(), dxCommon->GetBackBufferHeight()};

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

bool HasCameraTutorialInput()
{
	Input* input = Input::GetInstance();
	if (input->PushKey(DIK_W) || input->PushKey(DIK_A) || input->PushKey(DIK_S) || input->PushKey(DIK_D))
	{
		return true;
	}
	XINPUT_STATE state{};
	if (!input->GetJoystickState(0, state))
	{
		return false;
	}
	return state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
	       state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
	       state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
	       state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
}
}

GameScene::~GameScene()
{
	// A pause-menu return can destroy the scene during the warning cinematic.
	// Never leave its looping alarm playing over the title screen.
	GameAudio::GetInstance()->StopSe(GameAudio::Se::Danger);
	// AxisIndicator is a global singleton and otherwise retains a pointer to this
	// scene's camera after returning to the title screen.
	AxisIndicator::GetInstance()->SetVisible(false);
	AxisIndicator::GetInstance()->SetTargetCamera(nullptr);
	delete spriteHealthFill_;
	delete spriteHealthBack_;
	delete spriteDanger_;
	delete spriteBossHpFill_;
	delete spriteBossHpBack_;
	delete spriteBossName_;
	delete spriteObjectiveEnemies_;
	delete spriteObjectiveBoss_;
	delete debugCamera_;
	for (EnemyBullet* enemyBullet : enemyBullets_)
	{
		delete enemyBullet;
	}
	for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}
	delete player_;
	delete boss_;
	delete lockOn_;
	delete railCamera_;
	delete skydome_;
	delete modelSkydome_;
	delete modelPlayerBullet_;
	delete modelPlayer_;
	delete modelEnemyAnimated_;
	delete modelEnemy_;
	delete modelBoss_;
	delete model_;

	if (textureReticle_ != 0u)
	{
		TextureManager::Unload(textureReticle_);
	}
	if (textureLockOnReticle_ != 0u)
	{
		TextureManager::Unload(textureLockOnReticle_);
	}
	if (textureHealthFill_ != 0u)
	{
		TextureManager::Unload(textureHealthFill_);
	}
	if (textureHealthBack_ != 0u)
	{
		TextureManager::Unload(textureHealthBack_);
	}
	if (textureDanger_ != 0u)
	{
		TextureManager::Unload(textureDanger_);
	}
	if (textureBossName_ != 0u)
	{
		TextureManager::Unload(textureBossName_);
	}
	if (textureObjectiveEnemies_ != 0u)
	{
		TextureManager::Unload(textureObjectiveEnemies_);
	}
	if (textureObjectiveBoss_ != 0u)
	{
		TextureManager::Unload(textureObjectiveBoss_);
	}
}

void GameScene::Initialize(bool tutorialMode, AnimatedModel* preparedEnemyModel)
{
	isTutorialMode_ = tutorialMode;
	tutorialStep_ = 0;
	tutorialLockFrames_ = 0;
	tutorialComplete_ = false;
	dangerSequenceActive_ = false;
	dangerSequenceFinished_ = false;
	dangerSequenceTimer_ = 0;
	bossPhase_ = BossPhase::None;
	bossPhaseTimer_ = 0;
	textureReticle_ = TextureManager::Load("reticle/Standard reticle.png");
	textureLockOnReticle_ = TextureManager::Load("reticle/Lock-on reticle.png");
	textureHealthBack_ = TextureManager::Load("white1x1.png");
	textureHealthFill_ = TextureManager::Load("red1x1.png");
	spriteHealthBack_ = Sprite::Create(textureHealthBack_, {24.0f, static_cast<float>(WinApp::kWindowHeight) - 48.0f});
	spriteHealthFill_ = Sprite::Create(textureHealthFill_, {28.0f, static_cast<float>(WinApp::kWindowHeight) - 44.0f});
	spriteHealthBack_->SetSize({308.0f, 28.0f});
	spriteHealthBack_->SetColor({0.05f, 0.05f, 0.08f, 0.85f});
	spriteHealthFill_->SetSize({300.0f, 20.0f});
	// Tutorial never enters the boss sequence. Keeping these resources out of the
	// training scene also prevents unused texture/sprite handles from accumulating
	// while the title/tutorial/game loop is repeated.
	if (!isTutorialMode_)
	{
		textureDanger_ = TextureManager::Load("Danger.png");
		textureBossName_ = TextureManager::Load("BossName.png");
		textureObjectiveEnemies_ = TextureManager::Load("ObjectiveEnemies.png");
		textureObjectiveBoss_ = TextureManager::Load("ObjectiveBoss.png");
		spriteDanger_ = Sprite::Create(
		    textureDanger_,
		    {static_cast<float>(WinApp::kWindowWidth) * 0.5f, static_cast<float>(WinApp::kWindowHeight) * 0.5f},
		    {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.5f});
		spriteDanger_->SetSize({900.0f, 180.0f});
		spriteBossName_ = Sprite::Create(textureBossName_, {static_cast<float>(WinApp::kWindowWidth) * 0.5f, 32.0f},
		                                {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f});
		spriteBossName_->SetSize({520.0f, 52.0f});
		spriteBossHpBack_ = Sprite::Create(textureHealthBack_, {334.0f, 88.0f});
		spriteBossHpBack_->SetColor({0.015f, 0.02f, 0.04f, 0.94f});
		spriteBossHpBack_->SetSize({612.0f, 36.0f});
		spriteBossHpFill_ = Sprite::Create(textureHealthFill_, {340.0f, 94.0f});
		spriteBossHpFill_->SetColor({0.95f, 0.13f, 0.035f, 1.0f});
		spriteBossHpFill_->SetSize({600.0f, 24.0f});
		spriteObjectiveEnemies_ = Sprite::Create(
		    textureObjectiveEnemies_, {static_cast<float>(WinApp::kWindowWidth) * 0.5f, 18.0f},
		    {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f});
		spriteObjectiveEnemies_->SetSize({720.0f, 80.0f});
		spriteObjectiveBoss_ = Sprite::Create(
		    textureObjectiveBoss_, {static_cast<float>(WinApp::kWindowWidth) * 0.5f, 138.0f},
		    {1.0f, 1.0f, 1.0f, 1.0f}, {0.5f, 0.0f});
		spriteObjectiveBoss_->SetSize({720.0f, 80.0f});
	}
	model_ = Model::CreateFromOBJ("cube", true);
	modelEnemyAnimated_ = preparedEnemyModel;
	const std::filesystem::path enemyFbxPath = "Resources/Enemy Bicycle/Enemy Bicycle.fbx";
	const std::filesystem::path fallbackEnemyFbxPath = "DirectXGame/Resources/Enemy Bicycle/Enemy Bicycle.fbx";
	const std::filesystem::path resolvedEnemyFbxPath =
	    std::filesystem::exists(enemyFbxPath) ? enemyFbxPath : fallbackEnemyFbxPath;
	if (!modelEnemyAnimated_ && std::filesystem::exists(resolvedEnemyFbxPath))
	{
		modelEnemyAnimated_ = new AnimatedModel();
		if (!modelEnemyAnimated_->Load(resolvedEnemyFbxPath.string()))
		{
			OutputDebugStringA(("FBX load failed: " + modelEnemyAnimated_->GetLastError() + "\n").c_str());
			delete modelEnemyAnimated_;
			modelEnemyAnimated_ = nullptr;
		}
		else
		{
			// Validate the first animation pose immediately instead of waiting for gameplay to start.
			modelEnemyAnimated_->Update(0.0f);
		}
	}
	if (!modelEnemyAnimated_)
	{
		modelEnemy_ = Model::CreateFromOBJ("Enemy Bicycle", true);
	}
	modelPlayer_ = Model::CreateFromOBJ("cat", true);
	modelPlayerBullet_ = Model::CreateFromOBJ("fishboone", true);
	if (!isTutorialMode_)
	{
		modelBoss_ = Model::CreateFromOBJ("bakemono", true);
	}
	modelSkydome_ = Model::CreateFromOBJ(isTutorialMode_ ? "TrainingRoom" : "cosmo", true);

	camera_.Initialize();
	camera_.farZ = kCameraFarZ;

	railCamera_ = new RailCameraController();
	railCamera_->Initialize({0.0f, 2.0f, -50.0f}, {0.0f, 0.0f, 0.0f}, kCameraFarZ);
	railCamera_->SetDebugUIEnabled(!isTutorialMode_);
	if (isTutorialMode_)
	{
		// Establish a valid camera matrix while leaving the rail at its start.
		railCamera_->Update(false);
	}

	lockOn_ = new LockOn();
	lockOn_->Initialize(textureLockOnReticle_);

	player_ = new Player();
	player_->Initialize(modelPlayer_, modelPlayerBullet_, textureReticle_, textureHealthBack_);
	player_->SetParent(&railCamera_->GetWorldTransform());
	player_->SetLockOn(lockOn_);
	player_->SetAttackEnabled(!isTutorialMode_);
	player_->SetDebugUIEnabled(!isTutorialMode_);
	tutorialPlayerStart_ = player_->GetPosition();

	skydome_ = new Skydome();
	// The authored training sphere has radius 1, while the normal skydome has
	// radius 1000. Match their world size so the dome stays behind gameplay objects.
	// TrainingRoom and cosmo are authored as unit spheres, so both need to be
	// expanded around the camera to work as sky domes.
	skydome_->Initialize(modelSkydome_, 0u, 1000.0f);

	if (!isTutorialMode_)
	{
		LoadEnemyPopData();
	}

	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	debugCamera_->SetFarZ(kCameraFarZ);

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
}

void GameScene::Updata()
{
	if (isTutorialMode_)
	{
		UpdateTutorial();
		skydome_->Update(camera_.translation_);
		return;
	}

	const bool bossCinematicActive = bossPhase_ == BossPhase::Roaring || bossPhase_ == BossPhase::Defeated;
	bool shouldAdvanceSimulation = !dangerSequenceActive_ && !bossCinematicActive;

	if (Input::GetInstance()->TriggerKey(DIK_F1))
	{
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
	if (Input::GetInstance()->TriggerKey(DIK_F3))
	{
		showRailPath_ = !showRailPath_;
	}

	if (isDebugCameraActive_)
	{
		// The debug camera keeps accepting input while the simulation is frozen.
		// F2 advances exactly one game frame for inspecting fast projectiles.
		shouldAdvanceSimulation = Input::GetInstance()->TriggerKey(DIK_F2);
		debugCamera_->Update();
		const Camera& debugCamera = debugCamera_->GetCamera();
		camera_.matView = debugCamera.matView;
		camera_.matProjection = debugCamera.matProjection;
		camera_.translation_ = debugCamera.translation_;
		camera_.TransferMatrix();
	}
	else
	{
		const bool advanceRail = !dangerSequenceActive_ && bossPhase_ == BossPhase::None;
		railCamera_->Update(advanceRail);
		const Camera& railCamera = railCamera_->GetCamera();
		camera_.matView = railCamera.matView;
		camera_.matProjection = railCamera.matProjection;
		camera_.translation_ = railCamera.translation_;
		camera_.TransferMatrix();
	}

	if (shouldAdvanceSimulation)
	{
		if (modelEnemyAnimated_)
		{
			modelEnemyAnimated_->Update();
		}
		player_->Update(camera_);

		UpdateEnemyPopCommands();

		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsDead())
			{
				delete enemy;
				return true;
			}

			return false;
		});

		for (Enemy* enemy : enemies_)
		{
			enemy->Update();
		}

		// Remove enemies that crossed the foreground boundary during this
		// update before lock-on can keep a dangling/unreachable target.
		enemies_.remove_if([](Enemy* enemy) {
			if (!enemy->IsDead())
			{
				return false;
			}
			delete enemy;
			return true;
		});

		lockOn_->Update(player_, enemies_, camera_);

		enemyBullets_.remove_if([](EnemyBullet* enemyBullet) {
			if (enemyBullet->IsDead())
			{
				delete enemyBullet;
				return true;
			}

			return false;
		});

		for (EnemyBullet* enemyBullet : enemyBullets_)
		{
			enemyBullet->Update();
		}

		CheckAllCollisions();
	}

	// Keep the background centered on whichever camera is currently active.
	skydome_->Update(camera_.translation_);
	UpdateDangerSequence();
	UpdateBossSequence(shouldAdvanceSimulation);

#ifdef USE_IMGUI
	const ImGuiWindowFlags debugInfoFlags =
	    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
	    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;
	ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 150.0f), ImGuiCond_Always);
	ImGui::Begin("DebugInfo", nullptr, debugInfoFlags);
	const Vector3& playerPosition = player_->GetPosition();
	const Vector3& playerRotation = player_->GetRotation();
	ImGui::Text("Player Pos:(%.6f,%.6f,%.6f)", playerPosition.x, playerPosition.y, playerPosition.z);
	ImGui::Text("Player Rot:%.6f", playerRotation.y);
	ImGui::Text("Enemy Count:%zu", enemies_.size());
	ImGui::Text("EnemyBullet Count:%zu", enemyBullets_.size());
	ImGui::Text("F3: Rail path %s", showRailPath_ ? "ON" : "OFF");
	if (isDebugCameraActive_)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.1f, 1.0f), "DEBUG CAMERA / PAUSED");
		ImGui::Text("F1: Resume  F2: Step one frame");
	}
	if (!enemies_.empty())
	{
		const Enemy* enemy = enemies_.front();
		const Vector3& enemyPosition = enemy->GetPosition();
		ImGui::Text("Enemy Pos:(%.6f,%.6f,%.6f)", enemyPosition.x, enemyPosition.y, enemyPosition.z);
		ImGui::Text("Phase: %s", enemy->GetPhaseName());
	}
	ImGui::End();
#endif

}

void GameScene::UpdateTutorial()
{
	if (modelEnemyAnimated_)
	{
		modelEnemyAnimated_->Update();
	}

	// Accept normal camera controls but keep the training room fixed on the rail.
	railCamera_->Update(false);
	const Camera& tutorialCamera = railCamera_->GetCamera();
	camera_.matView = tutorialCamera.matView;
	camera_.matProjection = tutorialCamera.matProjection;
	camera_.translation_ = tutorialCamera.translation_;
	camera_.TransferMatrix();

	player_->Update(camera_);
	for (Enemy* enemy : enemies_)
	{
		enemy->Update();
	}
	lockOn_->Update(player_, enemies_, camera_);
	CheckAllCollisions();

	enemies_.remove_if([](Enemy* enemy) {
		if (!enemy->IsDead())
		{
			return false;
		}
		delete enemy;
		return true;
	});

	switch (tutorialStep_)
	{
	case 0:
	{
		const Vector3 playerPosition = player_->GetPosition();
		const Vector3 displacement = {
		    playerPosition.x - tutorialPlayerStart_.x,
		    playerPosition.y - tutorialPlayerStart_.y,
		    playerPosition.z - tutorialPlayerStart_.z};
		if (std::abs(displacement.x) >= 0.5f || std::abs(displacement.y) >= 0.5f)
		{
			tutorialStep_ = 1;
		}
		break;
	}
	case 1:
		if (HasCameraTutorialInput())
		{
			tutorialStep_ = 2;
			SpawnTrainingDummy();
		}
		break;
	case 2:
		if (lockOn_->ExistTarget())
		{
			if (++tutorialLockFrames_ >= 45)
			{
				tutorialStep_ = 3;
				player_->SetAttackEnabled(true);
			}
		}
		else
		{
			tutorialLockFrames_ = 0;
		}
		break;
	case 3:
		if (enemies_.empty())
		{
			tutorialStep_ = 4;
			tutorialComplete_ = true;
		}
		break;
	default:
		break;
	}
}

void GameScene::SpawnTrainingDummy()
{
	Enemy* enemy = new Enemy();
	enemy->Initialize(
	    modelEnemy_, modelPlayerBullet_, modelEnemyAnimated_, textureHealthBack_, textureHealthFill_,
	    {0.0f, 0.0f, 35.0f}, Enemy::BehaviorPattern::Straight);
	enemy->SetPlayer(player_);
	enemy->SetGameScene(this);
	enemy->SetTrainingDummy(true);
	enemies_.push_back(enemy);
}

bool GameScene::IsGameOver() const
{
	return !isTutorialMode_ && player_ != nullptr && player_->IsDead();
}

bool GameScene::IsGameClear() const
{
	return !isTutorialMode_ && dangerSequenceFinished_;
}

void GameScene::UpdateDangerSequence()
{
	// Once the boss has been created, the regular-enemy completion condition
	// remains true forever. Do not let that condition restart DANGER every frame.
	if (isTutorialMode_ || dangerSequenceFinished_ || boss_ != nullptr || bossPhase_ != BossPhase::None)
	{
		return;
	}

	const bool allRegularEnemiesDefeated =
	    enemyPopCommands_.eof() && !isWaitingEnemyPop_ && enemies_.empty();
	if (!dangerSequenceActive_)
	{
		if (!allRegularEnemiesDefeated)
		{
			return;
		}
		dangerSequenceActive_ = true;
		dangerSequenceTimer_ = 0;
		GameAudio::GetInstance()->PlaySeLoop(GameAudio::Se::Danger);
		// Remove any bullets left by the final enemy so the warning behaves as a
		// short cinematic and cannot cause an unfair death behind the sign.
		for (EnemyBullet* enemyBullet : enemyBullets_)
		{
			delete enemyBullet;
		}
		enemyBullets_.clear();
	}

	constexpr int32_t kDangerDurationFrames = 180;
	constexpr int32_t kShakeDurationFrames = 105;
	++dangerSequenceTimer_;

	// Offset the already-built view matrix. The rail camera overwrites it on the
	// next update, so the shake cannot accumulate or permanently move the camera.
	if (dangerSequenceTimer_ <= kShakeDurationFrames)
	{
		const float fade = 1.0f - static_cast<float>(dangerSequenceTimer_) /
		                             static_cast<float>(kShakeDurationFrames);
		const float shakeX = std::sin(static_cast<float>(dangerSequenceTimer_) * 2.37f) * 0.22f * fade;
		const float shakeY = std::sin(static_cast<float>(dangerSequenceTimer_) * 3.91f) * 0.16f * fade;
		camera_.matView.m[3][0] += shakeX;
		camera_.matView.m[3][1] += shakeY;
		camera_.TransferMatrix();
	}

	// A small pulse makes the warning feel like a sign slamming onto the screen.
	const float pulse = 1.0f + std::sin(static_cast<float>(dangerSequenceTimer_) * 0.22f) * 0.035f;
	spriteDanger_->SetSize({900.0f * pulse, 180.0f * pulse});

	if (dangerSequenceTimer_ >= kDangerDurationFrames)
	{
		GameAudio::GetInstance()->StopSe(GameAudio::Se::Danger);
		dangerSequenceActive_ = false;
		SpawnBoss();
	}
}

void GameScene::SpawnBoss()
{
	if (boss_ != nullptr || modelBoss_ == nullptr)
	{
		return;
	}
	Vector3 forward = MathUtility::TransformNormal({0.0f, 0.0f, 1.0f}, railCamera_->GetWorldTransform().matWorld_);
	MathUtility::Normalize(forward);
	const Vector3 position = {
	    camera_.translation_.x + forward.x * 68.0f,
	    camera_.translation_.y + forward.y * 68.0f + 4.0f,
	    camera_.translation_.z + forward.z * 68.0f};
	boss_ = new Boss();
	boss_->Initialize(modelBoss_, modelPlayerBullet_, player_, this, textureHealthFill_, position);
	GameAudio::GetInstance()->PlayBgm(GameAudio::Bgm::Boss);
	GameAudio::GetInstance()->PlaySe(GameAudio::Se::BossRoar);
	player_->SetBossTarget(boss_);
	bossPhase_ = BossPhase::Roaring;
	bossPhaseTimer_ = 0;
}

void GameScene::UpdateBossSequence(bool advanceBattle)
{
	if (boss_ == nullptr)
	{
		return;
	}

	if (bossPhase_ == BossPhase::Roaring)
	{
		constexpr int32_t kRoarDurationFrames = 150;
		++bossPhaseTimer_;
		const float entrance = std::clamp(static_cast<float>(bossPhaseTimer_) / 48.0f, 0.0f, 1.0f);
		const float smoothEntrance = entrance * entrance * (3.0f - 2.0f * entrance);
		const float roarPulse = bossPhaseTimer_ > 45
		                            ? 1.0f + std::sin(static_cast<float>(bossPhaseTimer_) * 0.55f) * 0.045f
		                            : 1.0f;
		boss_->SetPresentationScale((0.18f + 0.82f * smoothEntrance) * roarPulse);
		boss_->Update(false);

		if (bossPhaseTimer_ >= 35 && bossPhaseTimer_ <= 118)
		{
			const float fade = 1.0f - static_cast<float>(bossPhaseTimer_ - 35) / 83.0f;
			camera_.matView.m[3][0] += std::sin(static_cast<float>(bossPhaseTimer_) * 3.7f) * 0.30f * fade;
			camera_.matView.m[3][1] += std::sin(static_cast<float>(bossPhaseTimer_) * 5.1f) * 0.24f * fade;
			camera_.TransferMatrix();
		}

		if (bossPhaseTimer_ >= kRoarDurationFrames)
		{
			boss_->SetPresentationScale(1.0f);
			bossPhase_ = BossPhase::Battle;
			bossPhaseTimer_ = 0;
		}
		return;
	}

	if (bossPhase_ == BossPhase::Battle)
	{
		if (advanceBattle)
		{
			boss_->Update(true);
		}
		if (boss_->IsDead())
		{
			player_->SetBossTarget(nullptr);
			bossPhase_ = BossPhase::Defeated;
			bossPhaseTimer_ = 0;
			for (EnemyBullet* enemyBullet : enemyBullets_)
			{
				delete enemyBullet;
			}
			enemyBullets_.clear();
		}
		return;
	}

	if (bossPhase_ == BossPhase::Defeated)
	{
		if (++bossPhaseTimer_ >= 90)
		{
			dangerSequenceFinished_ = true;
		}
	}
}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet)
{
	assert(enemyBullet);
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::SpawnEnemy(const Vector3& position, Enemy::BehaviorPattern behaviorPattern)
{
	// Keep every scripted spawn inside the camera/player aiming area. The clamp
	// also protects future CSV edits from placing enemies outside the playable view.
	Vector3 visiblePosition = position;
	visiblePosition.x = std::clamp(visiblePosition.x, -14.0f, 14.0f);
	visiblePosition.y = std::clamp(visiblePosition.y, -7.0f, 8.0f);
	Enemy* enemy = new Enemy();
	enemy->Initialize(
	    modelEnemy_, modelPlayerBullet_, modelEnemyAnimated_, textureHealthBack_, textureHealthFill_,
	    visiblePosition, behaviorPattern);
	enemy->SetPlayer(player_);
	enemy->SetGameScene(this);
	enemies_.push_back(enemy);
}

void GameScene::LoadEnemyPopData()
{
	std::ifstream file;
	file.open("Resources/enemyPop.csv");
	if (!file.is_open())
	{
		file.open("DirectXGame/Resources/enemyPop.csv");
	}
	assert(file.is_open());

	enemyPopCommands_ << file.rdbuf();

	file.close();
}

void GameScene::UpdateEnemyPopCommands()
{
	if (isWaitingEnemyPop_)
	{
		enemyPopWaitTimer_--;
		if (enemyPopWaitTimer_ <= 0)
		{
			isWaitingEnemyPop_ = false;
		}
		return;
	}

	std::string line;
	while (std::getline(enemyPopCommands_, line))
	{
		std::istringstream lineStream(line);

		std::string word;
		std::getline(lineStream, word, ',');

		if (word.find("//") == 0)
		{
			continue;
		}
		else if (word.find("POP") == 0)
		{
			std::getline(lineStream, word, ',');
			float x = std::stof(word);

			std::getline(lineStream, word, ',');
			float y = std::stof(word);

			std::getline(lineStream, word, ',');
			float z = std::stof(word);

			Enemy::BehaviorPattern behaviorPattern = Enemy::BehaviorPattern::Straight;
			if (std::getline(lineStream, word, ',') && !word.empty())
			{
				const int32_t patternNumber = std::stoi(word);
				switch (patternNumber)
				{
				case 1:
					behaviorPattern = Enemy::BehaviorPattern::MoveLeft;
					break;
				case 2:
					behaviorPattern = Enemy::BehaviorPattern::MoveRight;
					break;
				case 3:
					behaviorPattern = Enemy::BehaviorPattern::Zigzag;
					break;
				case 0:
				default:
					behaviorPattern = Enemy::BehaviorPattern::Straight;
					break;
				}
			}

			SpawnEnemy({x, y, z}, behaviorPattern);
		}
		else if (word.find("WAIT") == 0)
		{
			std::getline(lineStream, word, ',');
			enemyPopWaitTimer_ = std::stoi(word);
			isWaitingEnemyPop_ = true;
			break;
		}
	}
}

void GameScene::CheckAllCollisions()
{
	const float kPlayerRadius = 1.0f;
	// Slightly generous because the animated bicycle-kick silhouette spreads
	// farther than the compact body center used by the lock-on marker.
	const float kEnemyRadius = 1.35f;
	const float kPlayerBulletRadius = 1.0f;
	const float kEnemyBulletRadius = 1.0f;

	Vector3 posA;
	Vector3 posB;

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();

#pragma region Player and enemy bullet collision
	posA = player_->GetWorldPosition();

	for (EnemyBullet* bullet : enemyBullets_)
	{
		posB = bullet->GetWorldPosition();

		if (IsCollision(posA, posB, kPlayerRadius, kEnemyBulletRadius))
		{
			player_->OnCollision();
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region Player bullet and enemy collision
	for (Enemy* enemy : enemies_)
	{
		posA = enemy->GetWorldPosition();

		for (PlayerBullet* bullet : playerBullets)
		{
			posB = bullet->GetWorldPosition();

			if (IsCollision(posA, posB, kEnemyRadius, kPlayerBulletRadius))
			{
				enemy->OnCollision();
				bullet->OnCollision();
			}
		}
	}
#pragma endregion

#pragma region Player bullet and boss collision
	if (boss_ != nullptr && bossPhase_ == BossPhase::Battle && !boss_->IsDead())
	{
		constexpr float kBossRadius = 7.0f;
		posA = boss_->GetWorldPosition();
		for (PlayerBullet* bullet : playerBullets)
		{
			if (!bullet->IsDead() && IsCollision(posA, bullet->GetWorldPosition(), kBossRadius, kPlayerBulletRadius))
			{
				boss_->OnCollision();
				bullet->OnCollision();
			}
		}
	}
#pragma endregion

#pragma region Player bullet and enemy bullet collision
	for (PlayerBullet* playerBullet : playerBullets)
	{
		posA = playerBullet->GetWorldPosition();

		for (EnemyBullet* enemyBullet : enemyBullets_)
		{
			posB = enemyBullet->GetWorldPosition();

			if (IsCollision(posA, posB, kPlayerBulletRadius, kEnemyBulletRadius))
			{
				playerBullet->OnCollision();
				enemyBullet->OnCollision();
			}
		}
	}
#pragma endregion
}

void GameScene::Draw()
{
	Model::PreDraw(Model::CullingMode::kNone);

	skydome_->Draw(camera_);

	Model::PostDraw();

	// Short orange afterimages use additive blending to read as a glow trail.
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kAdd);
	player_->DrawBulletTrails(camera_);
	Model::PostDraw();

	// Draw only the enlarged model's back faces so the dark pass remains a rim.
	Model::PreDraw(Model::CullingMode::kFront);
	player_->DrawBulletOutlines(camera_);
	Model::PostDraw();

	Model::PreDraw();

	player_->Draw(camera_);

	for (EnemyBullet* enemyBullet : enemyBullets_)
	{
		enemyBullet->Draw(camera_);
	}
	if (boss_ != nullptr)
	{
		boss_->Draw(camera_);
	}

	Model::PostDraw();

	if (isTutorialMode_)
	{
		// Draw enlarged back faces after the normal cat. Read-only depth keeps the
		// white body visible while leaving a strong navy/orange silhouette outside it.
		Model::PreDraw(Model::CullingMode::kFront, Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);
		player_->DrawTrainingOutlineDark(camera_);
		Model::PostDraw();
		Model::PreDraw(Model::CullingMode::kFront, Model::BlendMode::kAdd, Model::DepthTestMode::kReadOnly);
		player_->DrawTrainingOutlineGlow(camera_);
		Model::PostDraw();
	}

	if (modelEnemyAnimated_)
	{
		for (Enemy* enemy : enemies_)
		{
			enemy->Draw(camera_);
		}
	}
	else
	{
		Model::PreDraw();
		for (Enemy* enemy : enemies_)
		{
			enemy->Draw(camera_);
		}
		Model::PostDraw();
	}

	AxisIndicator::GetInstance()->Draw();
	// AxisIndicator leaves its small top-right viewport active, so restore the
	// full-screen viewport before drawing 2D sprites.
	RestoreFullScreenViewport();
	if (showRailPath_)
	{
		PrimitiveDrawer::GetInstance()->Reset();
		railCamera_->DrawRailPath(camera_);
	}

	Sprite::PreDraw();
	lockOn_->Draw();
	if (!isTutorialMode_)
	{
		spriteHealthBack_->Draw();
		const float healthRatio = static_cast<float>(player_->GetHealth()) / static_cast<float>(player_->GetMaxHealth());
		spriteHealthFill_->SetSize({300.0f * healthRatio, 20.0f});
		if (player_->GetHealth() > 0)
		{
			spriteHealthFill_->Draw();
		}
	}
	if (dangerSequenceActive_)
	{
		spriteDanger_->Draw();
	}
	else if (!isTutorialMode_ && bossPhase_ == BossPhase::None)
	{
		// Keep the current objective visible throughout the regular-enemy wave.
		spriteObjectiveEnemies_->Draw();
	}
	if (boss_ != nullptr && bossPhase_ != BossPhase::None)
	{
		spriteBossName_->Draw();
		spriteBossHpBack_->Draw();
		const float bossHealthRatio =
		    static_cast<float>(boss_->GetHealth()) / static_cast<float>(boss_->GetMaxHealth());
		spriteBossHpFill_->SetSize({600.0f * bossHealthRatio, 24.0f});
		if (boss_->GetHealth() > 0)
		{
			spriteBossHpFill_->Draw();
		}
		if (bossPhase_ == BossPhase::Roaring || bossPhase_ == BossPhase::Battle)
		{
			spriteObjectiveBoss_->Draw();
		}
	}
	Sprite::PostDraw();

}

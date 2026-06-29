#include "GameScene.h"
#include "WorldTransformConfig.h"
#include <cassert>
#include <imgui.h>

using namespace KamataEngine;

GameScene::~GameScene() 
{
	ClearFieldObjects();
	delete modelBlock_;

	delete debugCamera_;

	delete skydome_;
	delete modelSkydome_;

	delete modelPlayer_;
	delete modelAttack_;

	delete modelEnemy_;

	delete modelShieldEnemy_;

	for (HitEffect* hitEffect : hitEffects_)
	{
		delete hitEffect;
	}
	hitEffects_.clear();
	HitEffect::SetModel(nullptr);
	HitEffect::SetCamera(nullptr);
	delete modelHitEffect_;

	for (GuardEffect* guardEffect : guardEffects_)
	{
		delete guardEffect;
	}
	guardEffects_.clear();
	GuardEffect::SetModel(nullptr);
	GuardEffect::SetCamera(nullptr);
	delete modelGuardEffect_;

	delete deathParticles_;
	delete modelDeathParticles_;
	delete fade_;

	delete mapChipField_;

	delete cameraController_;
}

void GameScene::Initialize() {
	// カメラの初期化
	//camera_ = new Camera();
	//camera_->Initialize();

	cameraController_ = new CameraController();
	cameraController_->Initialize();

	camera_ = cameraController_->GetCamera();

	// 3Dモデルの生成
	modelBlock_ = Model::CreateFromOBJ("block", true);
	assert(modelBlock_);

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// スカイドームの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);
	
	// マップチップフィールド
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/mapChip.csv");

	// プレイヤーの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	assert(modelPlayer_);
	modelAttack_ = Model::CreateSphere();
	assert(modelAttack_);

	CameraController::Rect cameraArea = {0.0f, 100.0f, 0.0f, 100.0f};
	cameraController_->SetMovableArea(cameraArea);

	// 座標をマップチップ番号で指定
	// プレイヤーの初期座標が決まってからカメラを合わせる
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	assert(modelEnemy_);

	modelHitEffect_ = Model::CreateFromOBJ("hitEffect", true);
	assert(modelHitEffect_);
	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(camera_);

	modelGuardEffect_ = Model::CreateFromOBJ("guardEffect", true);
	assert(modelGuardEffect_);
	GuardEffect::SetModel(modelGuardEffect_);
	GuardEffect::SetCamera(camera_);

	modelShieldEnemy_ = Model::CreateFromOBJ("Yeti", true);
	assert(modelShieldEnemy_);

	GenerateFieldObjects();
	if (!player_)
	{
		GeneratePlayer(1, 18);
	}

	cameraController_->SetTarget(player_);
	cameraController_->Reset();
	camera_ = cameraController_->GetCamera();

	modelDeathParticles_ = Model::CreateSphere();
	assert(modelDeathParticles_);
	deathParticles_ = nullptr;
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);
	phase_ = Phase::kFadeIn;
	finished_ = false;
	reloadRequested_ = false;
}

void GameScene::Update() {
	if (phase_ == Phase::kFadeIn)
	{
		UpdateFadeInPhase();
		ChangePhase();
		return;
	}

	if (phase_ == Phase::kDeath)
	{
		UpdateDeathPhase();
		ChangePhase();
		return;
	}

	if (phase_ == Phase::kFadeOut)
	{
		UpdateFadeOutPhase();
		ChangePhase();
		return;
	}

	// ブロックの更新
#ifdef _DEBUG
	ImGui::Begin("Stage");
	if (ImGui::Button("Reload"))
	{
		reloadRequested_ = true;
	}
	ImGui::End();
#endif

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
		{
			if (!worldTransformBlock)
			{
				continue;
			}

			WorldTransformConfig(*worldTransformBlock);
		}
	}

	debugCamera_->Update();

#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_TAB))
	{
		isDebugCameraActive_ = true;
	}

#endif

	// カメラの処理
	if (isDebugCameraActive_) 
	{
		camera_->matView = debugCamera_->GetCamera().matView;
		camera_->matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の転送
		camera_->TransferMatrix();
	} 
	else
	{
		// ビュープロジェクション行列の更新と転送
		//camera_->UpdateMatrix();

		cameraController_->Update();

		camera_ = cameraController_->GetCamera();
	}

	// スカイドームの更新
	skydome_->Update();

	// プレイヤーの更新
	player_->Update();

	for (Enemy* enemy : enemies_)
	{
		enemy->Update();
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_)
	{
		shieldEnemy->Update();
	}

	if (deathParticles_)
	{
		deathParticles_->Update();
	}

	for (HitEffect* hitEffect : hitEffects_)
	{
		hitEffect->Update();
	}

	for (GuardEffect* guardEffect : guardEffects_)
	{
		guardEffect->Update();
	}

	CheckAllCollisions();
	RemoveDeadEnemies();
	RemoveDeadShieldEnemies();
	RemoveDeadHitEffects();
	RemoveDeadGuardEffects();
	ChangePhase();
}

void GameScene::UpdateDeathPhase()
{
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_)
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine)
		{
			if (!worldTransformBlock)
			{
				continue;
			}

			WorldTransformConfig(*worldTransformBlock);
		}
	}

	skydome_->Update();

	for (Enemy* enemy : enemies_)
	{
		enemy->Update();
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_)
	{
		shieldEnemy->Update();
	}

	if (deathParticles_)
	{
		deathParticles_->Update();
	}

	for (HitEffect* hitEffect : hitEffects_)
	{
		hitEffect->Update();
	}
	for (GuardEffect* guardEffect : guardEffects_)
	{
		guardEffect->Update();
	}
	RemoveDeadHitEffects();
	RemoveDeadGuardEffects();

	if (deathParticles_ && deathParticles_->IsFinished())
	{
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::FadeOut, kFadeDuration);
	}
}

void GameScene::UpdateFadeInPhase()
{
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_)
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine)
		{
			if (!worldTransformBlock)
			{
				continue;
			}

			WorldTransformConfig(*worldTransformBlock);
		}
	}

	cameraController_->Update();
	camera_ = cameraController_->GetCamera();
	skydome_->Update();

	fade_->Update();
	if (fade_->IsFinished())
	{
		fade_->Stop();
		phase_ = Phase::kPlay;
	}
}

void GameScene::UpdateFadeOutPhase()
{
	fade_->Update();
	if (fade_->IsFinished())
	{
		finished_ = true;
	}
}

void GameScene::ChangePhase()
{
	switch (phase_)
	{
	case Phase::kFadeIn:
		break;
	case Phase::kPlay:
		if (player_->IsDead())
		{
			phase_ = Phase::kDeath;
			delete deathParticles_;
			deathParticles_ = new DeathParticles();
			deathParticles_->Initialize(modelDeathParticles_, camera_, player_->GetWorldPosition());
		}
		break;
	case Phase::kDeath:
		break;
	case Phase::kFadeOut:
		break;
	}
}

void GameScene::Draw() 
{
	Model::PreDraw();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) 
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
		{
			if (!worldTransformBlock) 
			{
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, *camera_);
		}
	}

	// スカイドームの描画
	skydome_->Draw(*camera_);

	// プレイヤーの描画
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kPlay)
	{
		player_->Draw(*camera_);
	}

	for (Enemy* enemy : enemies_)
	{
		enemy->Draw(*camera_);
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_)
	{
		shieldEnemy->Draw(*camera_);
	}

	Model::PostDraw();

	for (HitEffect* hitEffect : hitEffects_)
	{
		hitEffect->Draw();
	}

	for (GuardEffect* guardEffect : guardEffects_)
	{
		guardEffect->Draw();
	}

	if (phase_ == Phase::kDeath && deathParticles_)
	{
		deathParticles_->Draw();
	}

	fade_->Draw();
}

void GameScene::CheckAllCollisions()
{
	AABB playerAABB = player_->GetAABB();

	for (Enemy* enemy : enemies_)
	{
		if (enemy->IsCollisionDisabled())
		{
			continue;
		}

		AABB enemyAABB = enemy->GetAABB();
		if (IsCollision(playerAABB, enemyAABB))
		{
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_)
	{
		if (shieldEnemy->IsCollisionDisabled())
		{
			continue;
		}

		AABB shieldEnemyAABB = shieldEnemy->GetAABB();
		if (IsCollision(playerAABB, shieldEnemyAABB))
		{
			player_->OnCollision(nullptr);
			shieldEnemy->OnCollision(player_);
		}
	}
}

void GameScene::RemoveDeadEnemies()
{
	enemies_.remove_if([](Enemy* enemy)
	{
		if (enemy->IsDead())
		{
			delete enemy;
			return true;
		}

		return false;
	});
}

void GameScene::RemoveDeadShieldEnemies()
{
	shieldEnemies_.remove_if([](ShieldEnemy* shieldEnemy)
	{
		if (shieldEnemy->IsDead())
		{
			delete shieldEnemy;
			return true;
		}

		return false;
	});
}

void GameScene::RemoveDeadHitEffects()
{
	hitEffects_.remove_if([](HitEffect* hitEffect)
	{
		if (hitEffect->IsDead())
		{
			delete hitEffect;
			return true;
		}

		return false;
	});
}

void GameScene::RemoveDeadGuardEffects()
{
	guardEffects_.remove_if([](GuardEffect* guardEffect)
	{
		if (guardEffect->IsDead())
		{
			delete guardEffect;
			return true;
		}

		return false;
	});
}

void GameScene::CreateHitEffect(const Vector3& position)
{
	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const Vector3& position)
{
	GuardEffect* newGuardEffect = GuardEffect::Create(position);
	guardEffects_.push_back(newGuardEffect);
}

void GameScene::GenerateFieldObjects()
{

	// 要素数
	ClearFieldObjects();

	uint32_t kNumBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	worldTransformBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i)
	{
		// 1列の要素数を設定
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// キューブの生成
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) 
	{
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) 
		{
			MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);
			switch (mapChipType)
			{
			case MapChipType::kBlock:
				GenerateBlock(j, i);
				break;
			case MapChipType::kPlayer:
				GeneratePlayer(j, i);
				break;
			case MapChipType::kEnemy:
				GenerateEnemy(j, i, mapChipField_->GetMapChipSubIDByIndex(j, i));
				break;
			case MapChipType::kBlank:
			default:
				break;
			}
		}
	}
}

void GameScene::GenerateBlock(uint32_t xIndex, uint32_t yIndex)
{
	WorldTransform* worldTransform = new WorldTransform();
	worldTransform->Initialize();
	worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
	worldTransformBlocks_[yIndex][xIndex] = worldTransform;
	WorldTransformConfig(*worldTransformBlocks_[yIndex][xIndex]);
}

void GameScene::GeneratePlayer(uint32_t xIndex, uint32_t yIndex)
{
	assert(player_ == nullptr && "Player is already generated.");

	player_ = new Player();
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
	player_->Initialize(modelPlayer_, modelAttack_, camera_, playerPosition);
	player_->SetMapChipField(mapChipField_);
}

void GameScene::GenerateEnemy(uint32_t xIndex, uint32_t yIndex, uint8_t subID)
{
	switch (subID)
	{
	case 0:
	{
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
		enemyPosition.y = mapChipField_->GetRectByIndex(xIndex, yIndex).bottom + Enemy::GetGroundOffset();
		newEnemy->Initialize(modelEnemy_, camera_, enemyPosition);
		newEnemy->SetGameScene(this);
		enemies_.push_back(newEnemy);
		break;
	}
	case 1:
	{
		ShieldEnemy* newShieldEnemy = new ShieldEnemy();
		Vector3 shieldEnemyPosition = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);
		shieldEnemyPosition.y = mapChipField_->GetRectByIndex(xIndex, yIndex).bottom + ShieldEnemy::GetGroundOffset();
		newShieldEnemy->Initialize(modelShieldEnemy_, camera_, shieldEnemyPosition);
		newShieldEnemy->SetGameScene(this);
		shieldEnemies_.push_back(newShieldEnemy);
		break;
	}
	default:
		break;
	}
}

void GameScene::ClearFieldObjects()
{
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_)
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine)
		{
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete player_;
	player_ = nullptr;

	for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}
	enemies_.clear();

	for (ShieldEnemy* shieldEnemy : shieldEnemies_)
	{
		delete shieldEnemy;
	}
	shieldEnemies_.clear();
}

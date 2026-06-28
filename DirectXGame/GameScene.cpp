#include "GameScene.h"
#include "WorldTransformConfig.h"
#include <cassert>

using namespace KamataEngine;

GameScene::~GameScene() 
{
	delete modelBlock_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_)
	{
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) 
		{
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;

	delete modelSkydome_;

	delete player_;

	delete modelPlayer_;

	for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}
	enemies_.clear();

	delete modelEnemy_;

	delete deathParticles_;
	delete modelDeathParticles_;

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
	GenerateBlocks();

	// プレイヤーの生成
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	assert(modelPlayer_);
	player_ = new Player();

	// 追従対象をプレイヤーに設定
	cameraController_->SetTarget(player_); 
	// カメラをプレイヤーの背後に配置
	cameraController_->Reset();      

	CameraController::Rect cameraArea = {0.0f, 100.0f, 0.0f, 100.0f};
	cameraController_->SetMovableArea(cameraArea);

	// 座標をマップチップ番号で指定
	KamataEngine::Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(modelPlayer_, camera_, playerPosition);

	player_->SetMapChipField(mapChipField_);

	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	assert(modelEnemy_);

	const uint32_t enemyCount = 3;
	const uint32_t enemyXIndices[enemyCount] = {7, 10, 13};
	for (uint32_t i = 0; i < enemyCount; ++i)
	{
		Enemy* newEnemy = new Enemy();
		KamataEngine::Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(enemyXIndices[i], 19);
		enemyPosition.y = mapChipField_->GetRectByIndex(enemyXIndices[i], 19).top + Enemy::GetGroundOffset();
		newEnemy->Initialize(modelEnemy_, camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	modelDeathParticles_ = Model::CreateSphere();
	assert(modelDeathParticles_);
	deathParticles_ = nullptr;
	phase_ = Phase::kPlay;
	finished_ = false;
}

void GameScene::Update() {
	if (phase_ == Phase::kDeath)
	{
		UpdateDeathPhase();
		ChangePhase();
		return;
	}
	// ブロックの更新
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

	if (deathParticles_)
	{
		deathParticles_->Update();
	}

	CheckAllCollisions();
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

	if (deathParticles_)
	{
		deathParticles_->Update();
	}

	if (deathParticles_ && deathParticles_->IsFinished())
	{
		finished_ = true;
	}
}

void GameScene::ChangePhase()
{
	switch (phase_)
	{
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
	if (phase_ == Phase::kPlay)
	{
		player_->Draw(*camera_);
	}

	for (Enemy* enemy : enemies_)
	{
		enemy->Draw(*camera_);
	}

	Model::PostDraw();

	if (phase_ == Phase::kDeath && deathParticles_)
	{
		deathParticles_->Draw();
	}
}

void GameScene::CheckAllCollisions()
{
	AABB playerAABB = player_->GetAABB();

	for (Enemy* enemy : enemies_)
	{
		AABB enemyAABB = enemy->GetAABB();
		if (IsCollision(playerAABB, enemyAABB))
		{
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}

void GameScene::GenerateBlocks()
{

	// 要素数
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
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock)
			{
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

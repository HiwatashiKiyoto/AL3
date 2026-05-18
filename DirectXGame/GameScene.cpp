#include "GameScene.h"
#include "WorldTransformConfig.h"

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

	delete mapChipField_;
}

void GameScene::Initialize() {
	// カメラの初期化
	camera_ = new Camera();
	camera_->Initialize();

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
	// 座標をマップチップ番号で指定
	KamataEngine::Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);
	player_->Initialize(modelPlayer_, camera_, playerPosition);

}

void GameScene::Updata() {
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

	if (Input::GetInstance()->TriggerKey(DIK_SPACE))
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
		camera_->UpdateMatrix();
	}

	// スカイドームの更新
	skydome_->Update();

	// プレイヤーの更新
	player_->Update();
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
	player_->Draw(*camera_);

	Model::PostDraw();
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
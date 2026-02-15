#include "GameScene.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete modelBlock_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;
}

void GameScene::Initialize() {
	// カメラの初期化
	camera_ = new Camera();
	camera_->Initialize();

	// 3Dモデルの生成
	modelBlock_ = Model::Create();
	assert(modelBlock_);

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 要素数
	const uint32_t kNumBlockVirtical = 10;
	const uint32_t kNumBlockHorizontal = 20;

	// ブロック一個分の横幅
	const float kBlockWidth = 2.0f;
	const float kBlockHeight = 2.0f;

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
			if (i % 2 == 0 && (j >= 5 && j <= 10)) 
			{
				worldTransformBlocks_[i][j] = nullptr; // 実体を作らない
				continue;
			}
			worldTransformBlocks_[i][j] = new WorldTransform();
			worldTransformBlocks_[i][j]->Initialize();
			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
			worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
		}
	}
}

void GameScene::Updata() {
	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			float sx = worldTransformBlock->scale_.x;
			float sy = worldTransformBlock->scale_.y;
			float sz = worldTransformBlock->scale_.z;

			float rx = worldTransformBlock->rotation_.x;
			float ry = worldTransformBlock->rotation_.y;
			float rz = worldTransformBlock->rotation_.z;

			float tx = worldTransformBlock->translation_.x;
			float ty = worldTransformBlock->translation_.y;
			float tz = worldTransformBlock->translation_.z;

			// sin, cosを各軸分用意
			float sx_s = sinf(rx);
			float cx_c = cosf(rx);
			float sy_s = sinf(ry);
			float cy_c = cosf(ry);
			float sz_s = sinf(rz);
			float cz_c = cosf(rz);

			// ---アフィン行列の各要素に直接代入---
			// 1行目
			worldTransformBlock->matWorld_.m[0][0] = sx * (cy_c * cz_c + sy_s * sx_s * sz_s);
			worldTransformBlock->matWorld_.m[0][1] = sx * (sy_s * sx_s * cz_c - cy_c * sz_s);
			worldTransformBlock->matWorld_.m[0][2] = sx * (sy_s * cx_c);
			worldTransformBlock->matWorld_.m[0][3] = 0.0f;

			// 2行目
			worldTransformBlock->matWorld_.m[1][0] = sy * (cx_c * sz_s);
			worldTransformBlock->matWorld_.m[1][1] = sy * (cx_c * cz_c);
			worldTransformBlock->matWorld_.m[1][2] = sy * (-sx_s);
			worldTransformBlock->matWorld_.m[1][3] = 0.0f;

			// 3行目
			worldTransformBlock->matWorld_.m[2][0] = sz * (cy_c * sx_s * sz_s - sy_s * cz_c);
			worldTransformBlock->matWorld_.m[2][1] = sz * (cy_c * sx_s * cz_c + sy_s * sz_s);
			worldTransformBlock->matWorld_.m[2][2] = sz * (cy_c * cx_c);
			worldTransformBlock->matWorld_.m[2][3] = 0.0f;

			// 4行目（平行移動）
			worldTransformBlock->matWorld_.m[3][0] = tx;
			worldTransformBlock->matWorld_.m[3][1] = ty;
			worldTransformBlock->matWorld_.m[3][2] = tz;
			worldTransformBlock->matWorld_.m[3][3] = 1.0f;

			worldTransformBlock->TransferMatrix();
		}
	}

	debugCamera_->Update();

#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = true;
	}

#endif 

	// カメラの処理
	if (isDebugCameraActive_) {
		camera_->matView = debugCamera_->GetCamera().matView;
		camera_->matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の転送
		camera_->TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		camera_->UpdateMatrix();
	}
}

void GameScene::Draw() {
	Model::PreDraw();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, *camera_);
		}
	}

	Model::PostDraw();
}

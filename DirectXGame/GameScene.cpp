#include "GameScene.h"

using namespace KamataEngine;

GameScene::~GameScene()
{
#ifdef _DEBUG
	delete debugCamera_;
#endif
	delete player_;
	delete model_;

	if (textureHandle_ != 0u)
	{
		TextureManager::Unload(textureHandle_);
	}
}

void GameScene::Initialize()
{
	textureHandle_ = TextureManager::Load("white1x1.png");
	model_ = Model::CreateFromOBJ("cube", true);

	camera_.Initialize();

	player_ = new Player();
	player_->Initialize(model_, textureHandle_);

#ifdef _DEBUG
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
#endif

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
}

void GameScene::Updata()
{
	player_->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1) || Input::GetInstance()->TriggerKey(DIK_SPACE))
	{
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	if (isDebugCameraActive_)
	{
		debugCamera_->Update();
		const Camera& debugCamera = debugCamera_->GetCamera();
		camera_.matView = debugCamera.matView;
		camera_.matProjection = debugCamera.matProjection;
		camera_.translation_ = debugCamera.translation_;
		camera_.TransferMatrix();
	}
	else
#endif
	{
		camera_.UpdateMatrix();
	}

}

void GameScene::Draw()
{
	Model::PreDraw();

	player_->Draw(camera_);

	AxisIndicator::GetInstance()->Draw();

	Model::PostDraw();
}

#include "GameScene.h"

using namespace KamataEngine;

GameScene::~GameScene()
{
	delete player_;
	delete model_;

	if (textureHandle_ != 0u)
	{
		TextureManager::Unload(textureHandle_);
	}
}

void GameScene::Initialize()
{
	textureHandle_ = TextureManager::Load("uvChecker.png");
	model_ = Model::CreateFromOBJ("cube", true);

	camera_.Initialize();

	player_ = new Player();
	player_->Initialize(model_, textureHandle_);
}

void GameScene::Updata()
{
	player_->Update();
}

void GameScene::Draw()
{
	Model::PreDraw();

	player_->Draw(camera_);

	Model::PostDraw();
}

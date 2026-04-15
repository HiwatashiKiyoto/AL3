#include "GameScene.h"
#include"player.h"

using namespace KamataEngine;

GameScene::~GameScene() 
{
	delete model_;
	delete player_;
}

void GameScene::Initialize()
{ 
	textureHandle_ = TextureManager::Load("dvd.png"); 

	model_ = Model::Create();

	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280,720);

	debugCamera_->GetCamera(); 

	camera_.Initialize();

	//自キャラの生成
	player_ = new Player();

	//自キャラの初期化
	player_->Initialize(model_,textureHandle_, &camera_);
}

void GameScene::Updata() 
{ 
	//自キャラの更新
	player_->Update();
}

void GameScene::Draw() 
{
	//自キャラの描画
	Model::PreDraw();
	player_->Draw();
	Model::PostDraw();
}

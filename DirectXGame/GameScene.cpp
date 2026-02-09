#include "GameScene.h"
#include"player.h"

using namespace KamataEngine;

GameScene::~GameScene() 
{
	delete model_;
	delete debugCamera_;
	delete player_;
}

void GameScene::Initialize()
{ 
	textureHandle_ = TextureManager::Load("dvd.png"); 

	model_ = Model::Create();

	//デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280,720);

	debugCamera_->GetCamera(); 

	camera_ = 

	//自キャラの生成
	player_ = new Player();

	//自キャラの初期化
	player_->Initialize(model_,textureHandle_, );
}

void GameScene::Updata() 
{ 
	debugCamera_->Update();

	//自キャラの更新
	player_->Update();
}

void GameScene::Draw() 
{
	//自キャラの描画
	player_->Draw();
}

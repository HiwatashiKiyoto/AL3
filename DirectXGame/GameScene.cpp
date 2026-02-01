#include "GameScene.h"
#include "2d/ImGuiManager.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete sprite_;

	delete model_;

	delete debugCamera_;
}

void GameScene::Initialize() {
	// ファイル名をしていしてテクスチャを読み込む
	textureHandle_ = TextureManager::Load("dvd.png");

	// スプライトインストラクタの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	// 3Dモデルの生成
	model_ = Model::Create();

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// カメラの初期化
	camera_.Initialize();

	// サウンドデータの読み込み
	soundDateHandle_ = Audio::GetInstance()->LoadWave("fanfare.wav");

	// 音声再生
	Audio::GetInstance()->PlayWave(soundDateHandle_);

	// 音声再生
	voiceHandle_ = Audio::GetInstance()->PlayWave(soundDateHandle_, true);

	// ライン描画が参照するカメラを指定する（アドレス渡し）
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1280, 720);

	//軸方向の表示を有効にする
	AxisIndicator::GetInstance()->SetVisible(true);

	//軸方向表示が参照するビュープロジェクションを指定する(アドレス渡し)
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());
}

void GameScene::Updata() {

	ImGui::Begin("Debug1");

	// デバックテキストの表示
	ImGui::Text("Hiwatashi Kiyoto %d.%d.%d", 2007, 2, 24);

	// float3入力ボックス
	ImGui::InputFloat3("InputFloat3", inputFloat3);

	// float3スライダー
	ImGui::SliderFloat3("SliderFloat3", inputFloat3, 0.0f, 1.0f);

	// デモウィンドウの表示を有効化
	ImGui::ShowDemoWindow();

	ImGui::End();

	// スプライトの今の座標を取得
	Vector2 position = sprite_->GetPosition();

	// 座標を{2,1}移動
	position.x += 2.0f;
	position.y += 1.0f;

	// 移動した座標をスプライトに反映
	sprite_->SetPosition(position);

	// スペースキーを押した瞬間
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// 音声停止
		Audio::GetInstance()->StopWave(voiceHandle_);
	}

	// デバッグカメラの更新
	debugCamera_->Update();
}

void GameScene::Draw() {
	////スプライト描画処理
	// Sprite::PreDraw();

	/////ここにスプライトインスタンスの描画処理を記述する
	// sprite_->Draw();

	////スプライト描画後処理
	// Sprite::PostDraw();

	// モデル描画処理
	Model::PreDraw();

	/// ここにモデルインスタンスの描画処理を記述する
	model_->Draw(worldTransform_, debugCamera_->GetCamera(), textureHandle_);

	// ラインを描画する
	PrimitiveDrawer::GetInstance()->DrawLine3d({0, 0, 0}, {0, 10, 0}, {1.0f, 0.0f, 0.0f, 1.0f});


	// モデル描画後処理
	Model::PostDraw();

}
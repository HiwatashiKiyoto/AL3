#include "GameScene.h"

using namespace KamataEngine;

GameScene::~GameScene()
{
#ifdef _DEBUG
	delete debugCamera_;
#endif
	delete enemy_;
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

	enemy_ = new Enemy();
	enemy_->Initialize(model_, {30.0f, 2.0f, 35.0f});

#ifdef _DEBUG
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
#endif

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
}

void GameScene::Updata()
{
	player_->Update();

	if (enemy_)
	{
		enemy_->Update();
	}

#ifdef USE_IMGUI
	const ImGuiWindowFlags debugInfoFlags =
	    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
	    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;
	ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(360.0f, 90.0f), ImGuiCond_Always);
	ImGui::Begin("DebugInfo", nullptr, debugInfoFlags);
	const Vector3& playerPosition = player_->GetPosition();
	const Vector3& playerRotation = player_->GetRotation();
	ImGui::Text("Player Pos:(%.6f,%.6f,%.6f)", playerPosition.x, playerPosition.y, playerPosition.z);
	ImGui::Text("Player Rot:%.6f", playerRotation.y);
	if (enemy_)
	{
		const Vector3& enemyPosition = enemy_->GetPosition();
		ImGui::Text("Enemy Pos:(%.6f,%.6f,%.6f)", enemyPosition.x, enemyPosition.y, enemyPosition.z);
		ImGui::Text("Phase: %s", enemy_->GetPhaseName());
	}
	ImGui::End();
#endif

#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_F1))
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

	if (enemy_)
	{
		enemy_->Draw(camera_);
	}

	AxisIndicator::GetInstance()->Draw();

	Model::PostDraw();
}

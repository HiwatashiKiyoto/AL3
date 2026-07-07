#include "GameScene.h"

using namespace KamataEngine;

namespace
{
const float kCameraFarZ = 2000.0f;

bool IsCollision(const Vector3& positionA, const Vector3& positionB, float radiusA, float radiusB)
{
	const float dx = positionB.x - positionA.x;
	const float dy = positionB.y - positionA.y;
	const float dz = positionB.z - positionA.z;
	const float distanceSquared = dx * dx + dy * dy + dz * dz;
	const float radiusSum = radiusA + radiusB;

	return distanceSquared <= radiusSum * radiusSum;
}
}

GameScene::~GameScene()
{
#ifdef _DEBUG
	delete debugCamera_;
#endif
	delete enemy_;
	delete player_;
	delete skydome_;
	delete modelSkydome_;
	delete modelPlayer_;
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
	modelPlayer_ = Model::CreateFromOBJ("cat", true);
	modelSkydome_ = Model::CreateFromOBJ("mySkydome", true);

	camera_.Initialize();
	camera_.farZ = kCameraFarZ;

	player_ = new Player();
	player_->Initialize(modelPlayer_, model_, textureHandle_);

	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);

	enemy_ = new Enemy();
	enemy_->Initialize(model_, {30.0f, 2.0f, 35.0f});
	enemy_->SetPlayer(player_);

#ifdef _DEBUG
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	debugCamera_->SetFarZ(kCameraFarZ);
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

	skydome_->Update();

	CheckAllCollisions();

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

void GameScene::CheckAllCollisions()
{
	const float kPlayerRadius = 1.0f;
	const float kEnemyRadius = 1.0f;
	const float kPlayerBulletRadius = 1.0f;
	const float kEnemyBulletRadius = 1.0f;

	Vector3 posA;
	Vector3 posB;

	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();
	const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullets();

#pragma region Player and enemy bullet collision
	posA = player_->GetWorldPosition();

	for (EnemyBullet* bullet : enemyBullets)
	{
		posB = bullet->GetWorldPosition();

		if (IsCollision(posA, posB, kPlayerRadius, kEnemyBulletRadius))
		{
			player_->OnCollision();
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region Player bullet and enemy collision
	posA = enemy_->GetWorldPosition();

	for (PlayerBullet* bullet : playerBullets)
	{
		posB = bullet->GetWorldPosition();

		if (IsCollision(posA, posB, kEnemyRadius, kPlayerBulletRadius))
		{
			enemy_->OnCollision();
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region Player bullet and enemy bullet collision
	for (PlayerBullet* playerBullet : playerBullets)
	{
		posA = playerBullet->GetWorldPosition();

		for (EnemyBullet* enemyBullet : enemyBullets)
		{
			posB = enemyBullet->GetWorldPosition();

			if (IsCollision(posA, posB, kPlayerBulletRadius, kEnemyBulletRadius))
			{
				playerBullet->OnCollision();
				enemyBullet->OnCollision();
			}
		}
	}
#pragma endregion
}

void GameScene::Draw()
{
	Model::PreDraw(Model::CullingMode::kNone);

	skydome_->Draw(camera_);

	Model::PostDraw();

	Model::PreDraw();

	player_->Draw(camera_);

	if (enemy_)
	{
		enemy_->Draw(camera_);
	}

	AxisIndicator::GetInstance()->Draw();

	Model::PostDraw();
}

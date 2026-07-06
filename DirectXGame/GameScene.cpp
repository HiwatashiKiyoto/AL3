#include "GameScene.h"

#include <cassert>
#include <fstream>
#include <string>

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
	for (EnemyBullet* enemyBullet : enemyBullets_)
	{
		delete enemyBullet;
	}
	for (Enemy* enemy : enemies_)
	{
		delete enemy;
	}
	delete player_;
	delete railCamera_;
	delete skydome_;
	delete modelSkydome_;
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
	modelSkydome_ = Model::CreateFromOBJ("mySkydome", true);

	camera_.Initialize();
	camera_.farZ = kCameraFarZ;

	railCamera_ = new RailCameraController();
	railCamera_->Initialize({0.0f, 2.0f, -50.0f}, {0.0f, 0.0f, 0.0f}, kCameraFarZ);

	player_ = new Player();
	player_->Initialize(model_, textureHandle_);
	player_->SetParent(&railCamera_->GetWorldTransform());

	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_);

	LoadEnemyPopData();

#ifdef _DEBUG
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	debugCamera_->SetFarZ(kCameraFarZ);
#endif

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&camera_);
}

void GameScene::Updata()
{
	railCamera_->Update();

	player_->Update();

	UpdateEnemyPopCommands();

	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead())
		{
			delete enemy;
			return true;
		}

		return false;
	});

	for (Enemy* enemy : enemies_)
	{
		enemy->Update();
	}

	enemyBullets_.remove_if([](EnemyBullet* enemyBullet) {
		if (enemyBullet->IsDead())
		{
			delete enemyBullet;
			return true;
		}

		return false;
	});

	for (EnemyBullet* enemyBullet : enemyBullets_)
	{
		enemyBullet->Update();
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
	ImGui::Text("Enemy Count:%zu", enemies_.size());
	ImGui::Text("EnemyBullet Count:%zu", enemyBullets_.size());
	if (!enemies_.empty())
	{
		const Enemy* enemy = enemies_.front();
		const Vector3& enemyPosition = enemy->GetPosition();
		ImGui::Text("Enemy Pos:(%.6f,%.6f,%.6f)", enemyPosition.x, enemyPosition.y, enemyPosition.z);
		ImGui::Text("Phase: %s", enemy->GetPhaseName());
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
		const Camera& railCamera = railCamera_->GetCamera();
		camera_.matView = railCamera.matView;
		camera_.matProjection = railCamera.matProjection;
		camera_.translation_ = railCamera.translation_;
		camera_.TransferMatrix();
	}

}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet)
{
	assert(enemyBullet);
	enemyBullets_.push_back(enemyBullet);
}

void GameScene::SpawnEnemy(const Vector3& position)
{
	Enemy* enemy = new Enemy();
	enemy->Initialize(model_, position);
	enemy->SetPlayer(player_);
	enemy->SetGameScene(this);
	enemies_.push_back(enemy);
}

void GameScene::LoadEnemyPopData()
{
	std::ifstream file;
	file.open("Resources/enemyPop.csv");
	if (!file.is_open())
	{
		file.open("DirectXGame/Resources/enemyPop.csv");
	}
	assert(file.is_open());

	enemyPopCommands_ << file.rdbuf();

	file.close();
}

void GameScene::UpdateEnemyPopCommands()
{
	if (isWaitingEnemyPop_)
	{
		enemyPopWaitTimer_--;
		if (enemyPopWaitTimer_ <= 0)
		{
			isWaitingEnemyPop_ = false;
		}
		return;
	}

	std::string line;
	while (std::getline(enemyPopCommands_, line))
	{
		std::istringstream lineStream(line);

		std::string word;
		std::getline(lineStream, word, ',');

		if (word.find("//") == 0)
		{
			continue;
		}
		else if (word.find("POP") == 0)
		{
			std::getline(lineStream, word, ',');
			float x = std::stof(word);

			std::getline(lineStream, word, ',');
			float y = std::stof(word);

			std::getline(lineStream, word, ',');
			float z = std::stof(word);

			SpawnEnemy({x, y, z});
		}
		else if (word.find("WAIT") == 0)
		{
			std::getline(lineStream, word, ',');
			enemyPopWaitTimer_ = std::stoi(word);
			isWaitingEnemyPop_ = true;
			break;
		}
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

#pragma region Player and enemy bullet collision
	posA = player_->GetWorldPosition();

	for (EnemyBullet* bullet : enemyBullets_)
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
	for (Enemy* enemy : enemies_)
	{
		posA = enemy->GetWorldPosition();

		for (PlayerBullet* bullet : playerBullets)
		{
			posB = bullet->GetWorldPosition();

			if (IsCollision(posA, posB, kEnemyRadius, kPlayerBulletRadius))
			{
				enemy->OnCollision();
				bullet->OnCollision();
			}
		}
	}
#pragma endregion

#pragma region Player bullet and enemy bullet collision
	for (PlayerBullet* playerBullet : playerBullets)
	{
		posA = playerBullet->GetWorldPosition();

		for (EnemyBullet* enemyBullet : enemyBullets_)
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

	for (Enemy* enemy : enemies_)
	{
		enemy->Draw(camera_);
	}

	for (EnemyBullet* enemyBullet : enemyBullets_)
	{
		enemyBullet->Draw(camera_);
	}

	AxisIndicator::GetInstance()->Draw();

	Model::PostDraw();
}

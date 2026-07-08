#include "LockOn.h"

#include "Enemy.h"
#include "Player.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

LockOn::~LockOn()
{
}

void LockOn::Initialize(uint32_t textureHandle)
{
	(void)textureHandle;
}

void LockOn::Update(Player* player, const std::list<Enemy*>& enemies, const Camera& camera)
{
	assert(player);

	target_ = nullptr;
	std::list<std::pair<float, Enemy*>> targets;

	const Vector3 playerViewPosition = Transform(player->GetWorldPosition(), camera.matView);
	const Vector2 reticlePosition = player->Get2DReticlePosition();

	for (Enemy* enemy : enemies)
	{
		if (enemy == nullptr || enemy->IsDead())
		{
			continue;
		}

		const Vector3 positionWorld = enemy->GetWorldPosition();
		const Vector3 positionView = Transform(positionWorld, camera.matView);
		if (positionView.z <= 0.0f || positionView.z < playerViewPosition.z)
		{
			continue;
		}

		const Vector3 positionScreen =
		    Project(positionWorld, 0.0f, 0.0f, static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight), camera.matView * camera.matProjection);

		const float dx = positionScreen.x - reticlePosition.x;
		const float dy = positionScreen.y - reticlePosition.y;
		const float distance = std::sqrt(dx * dx + dy * dy);
		const float kDistanceLockOn = 160.0f;

		if (distance <= kDistanceLockOn)
		{
			targets.emplace_back(std::make_pair(distance, enemy));
		}
	}

	if (!targets.empty())
	{
		targets.sort();
		target_ = targets.front().second;

		const Vector3 targetScreen =
		    Project(target_->GetWorldPosition(), 0.0f, 0.0f, static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight), camera.matView * camera.matProjection);
		targetScreenPosition_ = {targetScreen.x, targetScreen.y};
	}

	Draw();
}

void LockOn::Draw()
{
#ifdef USE_IMGUI
	if (target_ != nullptr)
	{
		ImDrawList* drawList = ImGui::GetForegroundDrawList(ImGui::GetMainViewport());
		const ImVec2 center(targetScreenPosition_.x, targetScreenPosition_.y);
		const ImU32 color = IM_COL32(220, 210, 32, 230);
		const float radius = 38.0f;
		const float gap = 10.0f;
		const float lineLength = 20.0f;

		drawList->AddCircle(center, radius, color, 48, 5.0f);
		drawList->AddCircle(center, 8.0f, color, 32, 3.0f);
		drawList->AddLine(ImVec2(center.x - radius - lineLength, center.y), ImVec2(center.x - gap, center.y), color, 5.0f);
		drawList->AddLine(ImVec2(center.x + gap, center.y), ImVec2(center.x + radius + lineLength, center.y), color, 5.0f);
		drawList->AddLine(ImVec2(center.x, center.y - radius - lineLength), ImVec2(center.x, center.y - gap), color, 5.0f);
		drawList->AddLine(ImVec2(center.x, center.y + gap), ImVec2(center.x, center.y + radius + lineLength), color, 5.0f);
	}
#endif
}

Vector3 LockOn::Project(
    const Vector3& worldPosition, float viewportX, float viewportY, float viewportWidth, float viewportHeight, const Matrix4x4& viewProjection)
{
	Matrix4x4 matViewport = MakeIdentityMatrix();
	matViewport.m[0][0] = viewportWidth / 2.0f;
	matViewport.m[1][1] = -viewportHeight / 2.0f;
	matViewport.m[2][2] = 1.0f;
	matViewport.m[3][0] = viewportX + viewportWidth / 2.0f;
	matViewport.m[3][1] = viewportY + viewportHeight / 2.0f;

	return TransformCoord(worldPosition, viewProjection * matViewport);
}

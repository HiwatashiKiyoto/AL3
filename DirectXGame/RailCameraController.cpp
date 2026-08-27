#include "RailCameraController.h"

#include "CatmullRomSpline.h"
#include "WorldTransformUpdate.h"

#include <algorithm>
#include <climits>
#include <cmath>

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

namespace
{
constexpr size_t kRailSampleCount = 400;
// Keep the rail-shooter motion, but make it gentle enough that aiming does not
// feel as if the camera is constantly sliding away from the player.
constexpr float kRailSpeed = 0.09f;
constexpr float kLookAheadDistance = 6.0f;

float NormalizeThumbAxis(SHORT value, SHORT deadzone)
{
	const int magnitude = std::abs(static_cast<int>(value));
	if (magnitude <= deadzone)
	{
		return 0.0f;
	}

	const float normalizedMagnitude = std::clamp(
	    static_cast<float>(magnitude - deadzone) / static_cast<float>(SHRT_MAX - deadzone), 0.0f, 1.0f);
	return value < 0 ? -normalizedMagnitude : normalizedMagnitude;
}
}

void RailCameraController::Initialize(const Vector3& position, const Vector3& rotation, float farZ)
{
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_ = rotation;
	targetRotation_ = {};
	aimRotation_ = {};
	railRotation_ = rotation;
	railT_ = 0.0f;
	railDistance_ = 0.0f;
	controlPoints_ = {
	    {position.x, position.y, position.z},
	    {position.x + 0.8f, position.y + 0.3f, position.z + 25.0f},
	    {position.x - 0.6f, position.y + 0.2f, position.z + 55.0f},
	    {position.x + 1.0f, position.y + 0.5f, position.z + 85.0f},
	    {position.x - 0.7f, position.y + 0.3f, position.z + 120.0f},
	    {position.x, position.y + 0.4f, position.z + 155.0f},
	};

	// Precompute a length table so equal movement distances produce equal speed.
	railSamples_.clear();
	railCumulativeLengths_.clear();
	railSamples_.reserve(kRailSampleCount + 1);
	railCumulativeLengths_.reserve(kRailSampleCount + 1);
	railLength_ = 0.0f;
	for (size_t i = 0; i <= kRailSampleCount; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(kRailSampleCount);
		const Vector3 sample = CatmullRomPosition(controlPoints_, t);
		if (!railSamples_.empty())
		{
			railLength_ += Length(sample - railSamples_.back());
		}
		railSamples_.push_back(sample);
		railCumulativeLengths_.push_back(railLength_);
	}

	worldTransform_.translation_ = SampleRailByDistance(0.0f);
	Vector3 initialForward = SampleRailByDistance(kLookAheadDistance) - worldTransform_.translation_;
	Normalize(initialForward);
	const float initialHorizontalLength = std::sqrt(initialForward.x * initialForward.x + initialForward.z * initialForward.z);
	railRotation_ = {
	    -std::atan2(initialForward.y, initialHorizontalLength),
	    std::atan2(initialForward.x, initialForward.z),
	    0.0f,
	};
	worldTransform_.rotation_ = railRotation_;
	UpdateWorldTransform(worldTransform_);

	camera_.Initialize();
	camera_.farZ = farZ;
}

void RailCameraController::Update(bool advanceRail)
{
	Input* input = Input::GetInstance();

	if (advanceRail)
	{
		railDistance_ = std::clamp(railDistance_ + kRailSpeed, 0.0f, railLength_);
	}
	railT_ = railLength_ > 0.0f ? railDistance_ / railLength_ : 0.0f;
	const Vector3 eye = SampleRailByDistance(railDistance_);

	// Look a fixed world-space distance ahead. At the end, reuse the final tangent.
	Vector3 forward;
	if (railDistance_ < railLength_)
	{
		const Vector3 target = SampleRailByDistance(std::clamp(railDistance_ + kLookAheadDistance, 0.0f, railLength_));
		forward = target - eye;
	}
	else
	{
		const Vector3 previous = SampleRailByDistance(std::clamp(railLength_ - kLookAheadDistance, 0.0f, railLength_));
		forward = eye - previous;
	}
	Normalize(forward);

	const float horizontalLength = std::sqrt(forward.x * forward.x + forward.z * forward.z);
	const Vector3 desiredRailRotation = {
	    -std::atan2(forward.y, horizontalLength),
	    std::atan2(forward.x, forward.z),
	    0.0f,
	};

	const float kAimSpeed = 0.02f;
	if (input->PushKey(DIK_W))
	{
		targetRotation_.x -= kAimSpeed;
	}
	if (input->PushKey(DIK_S))
	{
		targetRotation_.x += kAimSpeed;
	}
	if (input->PushKey(DIK_A))
	{
		targetRotation_.y -= kAimSpeed;
	}
	if (input->PushKey(DIK_D))
	{
		targetRotation_.y += kAimSpeed;
	}

	XINPUT_STATE joyState{};
	if (input->GetJoystickState(0, joyState))
	{
		const float rightX = NormalizeThumbAxis(joyState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		const float rightY = NormalizeThumbAxis(joyState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		constexpr float kGamepadAimSpeed = 0.035f;
		targetRotation_.x -= rightY * kGamepadAimSpeed;
		targetRotation_.y += rightX * kGamepadAimSpeed;
	}

	// Keep the rail shooter facing generally forward and avoid flipping around.
	targetRotation_.x = std::clamp(targetRotation_.x, -0.45f, 0.45f);
	targetRotation_.y = std::clamp(targetRotation_.y, -0.75f, 0.75f);
	targetRotation_.z = 0.0f;

	// A quicker follow rate removes the long easing tail after releasing the
	// right stick, which was perceived as unintended camera drift.
	const float kFollowRate = 0.32f;
	aimRotation_.x += (targetRotation_.x - aimRotation_.x) * kFollowRate;
	aimRotation_.y += (targetRotation_.y - aimRotation_.y) * kFollowRate;
	aimRotation_.z = 0.0f;

	// Smooth the path tangent separately so tight bends do not snap the view.
	constexpr float kRailRotationFollowRate = 0.18f;
	railRotation_.x += (desiredRailRotation.x - railRotation_.x) * kRailRotationFollowRate;
	railRotation_.y += (desiredRailRotation.y - railRotation_.y) * kRailRotationFollowRate;
	railRotation_.z = 0.0f;

	worldTransform_.translation_ = eye;
	worldTransform_.rotation_ = railRotation_ + aimRotation_;

#ifdef USE_IMGUI
	if (isDebugUIEnabled_)
	{
		ImGui::Begin("Camera");
		ImGui::DragFloat3("Translation", &worldTransform_.translation_.x, 0.01f);
		ImGui::DragFloat3("Rotation", &worldTransform_.rotation_.x, 0.01f);
		ImGui::Text("Rail Progress: %.3f", railT_);
		ImGui::End();
	}
#endif

	UpdateWorldTransform(worldTransform_);

	camera_.translation_ = worldTransform_.translation_;
	camera_.rotation_ = worldTransform_.rotation_;
	camera_.matView = Inverse(worldTransform_.matWorld_);
	camera_.UpdateProjectionMatrix();
	camera_.TransferMatrix();
}

Vector3 RailCameraController::SampleRailByDistance(float distance) const
{
	if (railSamples_.empty())
	{
		return {};
	}

	distance = std::clamp(distance, 0.0f, railLength_);
	const auto upper = std::lower_bound(railCumulativeLengths_.begin(), railCumulativeLengths_.end(), distance);
	if (upper == railCumulativeLengths_.begin())
	{
		return railSamples_.front();
	}
	if (upper == railCumulativeLengths_.end())
	{
		return railSamples_.back();
	}

	const size_t upperIndex = static_cast<size_t>(upper - railCumulativeLengths_.begin());
	const size_t lowerIndex = upperIndex - 1;
	const float lowerDistance = railCumulativeLengths_[lowerIndex];
	const float upperDistance = railCumulativeLengths_[upperIndex];
	const float segmentLength = upperDistance - lowerDistance;
	const float localT = segmentLength > 0.0f ? (distance - lowerDistance) / segmentLength : 0.0f;
	return railSamples_[lowerIndex] + (railSamples_[upperIndex] - railSamples_[lowerIndex]) * localT;
}

void RailCameraController::DrawRailPath(const Camera& camera) const
{
	PrimitiveDrawer* drawer = PrimitiveDrawer::GetInstance();
	drawer->SetCamera(&camera);

	constexpr size_t kSegmentCount = 160;
	const Vector4 railColor = {1.0f, 0.15f, 0.05f, 1.0f};
	Vector3 previous = CatmullRomPosition(controlPoints_, 0.0f);
	for (size_t i = 1; i <= kSegmentCount; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(kSegmentCount);
		const Vector3 current = CatmullRomPosition(controlPoints_, t);
		drawer->DrawLine3d(previous, current, railColor);
		previous = current;
	}

	// Draw small crosses so the control points can be distinguished from the curve.
	const Vector4 pointColor = {1.0f, 0.85f, 0.05f, 1.0f};
	constexpr float kMarkerHalfSize = 0.7f;
	for (const Vector3& point : controlPoints_)
	{
		drawer->DrawLine3d({point.x - kMarkerHalfSize, point.y, point.z}, {point.x + kMarkerHalfSize, point.y, point.z}, pointColor);
		drawer->DrawLine3d({point.x, point.y - kMarkerHalfSize, point.z}, {point.x, point.y + kMarkerHalfSize, point.z}, pointColor);
		drawer->DrawLine3d({point.x, point.y, point.z - kMarkerHalfSize}, {point.x, point.y, point.z + kMarkerHalfSize}, pointColor);
	}
}

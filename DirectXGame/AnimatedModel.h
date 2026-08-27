#pragma once

#include "KamataEngine.h"

#include <d3d12.h>
#include <cstddef>
#include <string>
#include <vector>
#include <wrl.h>

struct ufbx_anim_stack;
struct ufbx_scene;

// FBX model with CPU-evaluated skinning and animation.
// This lives beside KamataEngine::Model because the distributed engine is a binary library.
class AnimatedModel
{
public:
	AnimatedModel() = default;
	~AnimatedModel();

	bool Load(const std::string& filePath);
	void Update(float deltaTime = 1.0f / 60.0f);
	void Draw(const KamataEngine::WorldTransform& worldTransform, const KamataEngine::Camera& camera) const;
	void SetColor(const KamataEngine::Vector4& color);

	bool IsLoaded() const { return sourceScene_ != nullptr; }
	bool HasAnimation() const { return animationStack_ != nullptr; }
	const std::string& GetAnimationName() const { return animationName_; }
	const std::string& GetLastError() const { return lastError_; }
	const KamataEngine::Vector3& GetBoundsCenter() const { return boundsCenter_; }
	const KamataEngine::Vector3& GetCurrentBoundsCenter() const
	{
		return frameBoundsCenters_.empty() ? boundsCenter_ : frameBoundsCenters_[currentFrame_];
	}
	const KamataEngine::Vector3& GetCurrentAimPoint() const
	{
		return frameAimPoints_.empty() ? GetCurrentBoundsCenter() : frameAimPoints_[currentFrame_];
	}
	float GetBoundsMaxExtent() const { return boundsMaxExtent_; }
	void SetPlaybackSpeed(float speed) { playbackSpeed_ = speed; }

private:
	struct Vertex
	{
		KamataEngine::Vector3 position;
		KamataEngine::Vector3 normal;
	};

	bool CreatePipeline();
	bool CreateColorBuffer();
	bool CreateVertexBuffer(size_t vertexCount, size_t frameCount);
	bool WriteVertices(const ufbx_scene* scene, size_t frameIndex);
	void ReleaseScenes();

	ufbx_scene* sourceScene_ = nullptr;
	ufbx_scene* evaluatedScene_ = nullptr;
	ufbx_anim_stack* animationStack_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Vertex* mappedVertices_ = nullptr;
	size_t vertexCount_ = 0;
	size_t frameCount_ = 1;
	size_t currentFrame_ = 0;
	UINT64 frameStrideBytes_ = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<ID3D12Resource> colorBuffer_;
	KamataEngine::Vector4* mappedColor_ = nullptr;

	double animationTime_ = 0.0;
	double animationDuration_ = 0.0;
	float playbackSpeed_ = 1.0f;
	size_t rootMotionNodeIndex_ = static_cast<size_t>(-1);
	size_t aimNodeIndex_ = static_cast<size_t>(-1);
	KamataEngine::Vector3 rootMotionOrigin_{};
	KamataEngine::Vector3 boundsCenter_{};
	std::vector<KamataEngine::Vector3> frameBoundsCenters_;
	std::vector<KamataEngine::Vector3> frameAimPoints_;
	float boundsMaxExtent_ = 1.0f;
	std::string animationName_;
	std::string lastError_;
};

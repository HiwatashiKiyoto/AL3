#include "AnimatedModel.h"

#include "ThirdParty/ufbx/ufbx.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <limits>
#include <vector>

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace KamataEngine;
using Microsoft::WRL::ComPtr;

namespace
{
std::wstring FindShader(const wchar_t* name)
{
	std::filesystem::path path = std::filesystem::path(L"Resources/shaders") / name;
	if (std::filesystem::exists(path))
	{
		return path.wstring();
	}
	return (std::filesystem::path(L"DirectXGame/Resources/shaders") / name).wstring();
}

ComPtr<ID3DBlob> CompileShader(const wchar_t* fileName, const char* profile, std::string& errorMessage)
{
	ComPtr<ID3DBlob> shader;
	ComPtr<ID3DBlob> errors;
#ifdef _DEBUG
	const UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	const UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	const std::wstring path = FindShader(fileName);
	const HRESULT result = D3DCompileFromFile(
	    path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", profile, flags, 0, &shader, &errors);
	if (FAILED(result))
	{
		if (errors)
		{
			errorMessage.assign(static_cast<const char*>(errors->GetBufferPointer()), errors->GetBufferSize());
		}
		else
		{
			errorMessage = "Failed to compile animated-model shader.";
		}
		return nullptr;
	}
	return shader;
}
}

AnimatedModel::~AnimatedModel()
{
	if (colorBuffer_ && mappedColor_)
	{
		colorBuffer_->Unmap(0, nullptr);
		mappedColor_ = nullptr;
	}
	if (vertexBuffer_ && mappedVertices_)
	{
		vertexBuffer_->Unmap(0, nullptr);
		mappedVertices_ = nullptr;
	}
	ReleaseScenes();
}

void AnimatedModel::ReleaseScenes()
{
	if (evaluatedScene_)
	{
		ufbx_free_scene(evaluatedScene_);
		evaluatedScene_ = nullptr;
	}
	if (sourceScene_)
	{
		ufbx_free_scene(sourceScene_);
		sourceScene_ = nullptr;
	}
	animationStack_ = nullptr;
}

bool AnimatedModel::Load(const std::string& filePath)
{
	ReleaseScenes();
	lastError_.clear();
	animationName_.clear();
	animationTime_ = 0.0;
	animationDuration_ = 0.0;
	frameCount_ = 1;
	currentFrame_ = 0;
	frameStrideBytes_ = 0;
	rootMotionNodeIndex_ = static_cast<size_t>(-1);
	aimNodeIndex_ = static_cast<size_t>(-1);
	rootMotionOrigin_ = {};
	boundsCenter_ = {};
	frameBoundsCenters_.clear();
	frameAimPoints_.clear();
	boundsMaxExtent_ = 1.0f;

	ufbx_load_opts opts{};
	opts.target_axes = ufbx_axes_left_handed_y_up;
	opts.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;
	opts.generate_missing_normals = true;
	opts.evaluate_skinning = true;

	ufbx_error error{};
	sourceScene_ = ufbx_load_file(filePath.c_str(), &opts, &error);
	if (!sourceScene_)
	{
		lastError_ = error.description.data ? error.description.data : "Failed to load FBX.";
		return false;
	}

	if (sourceScene_->anim_stacks.count > 0)
	{
		animationStack_ = sourceScene_->anim_stacks.data[0];
		animationTime_ = animationStack_->time_begin;
		animationDuration_ = (std::max)(0.0, animationStack_->time_end - animationStack_->time_begin);
		animationName_.assign(animationStack_->name.data, animationStack_->name.length);
	}

	// The complete animated bounds move heavily during the bicycle kick, so its
	// center is not a stable aiming point. Track the upper torso bone instead.
	for (size_t nodeIndex = 0; nodeIndex < sourceScene_->nodes.count; ++nodeIndex)
	{
		const ufbx_node* node = sourceScene_->nodes.data[nodeIndex];
		const std::string nodeName(node->name.data, node->name.length);
		if (nodeName == "mixamorig:Spine2" || nodeName == "Spine2")
		{
			aimNodeIndex_ = nodeIndex;
			break;
		}
	}

	// Use the highest bone in the first skinned mesh as the root-motion source.
	// Its animated translation is removed later so the FBX performs in place.
	for (size_t meshIndex = 0; meshIndex < sourceScene_->meshes.count && rootMotionNodeIndex_ == static_cast<size_t>(-1);
	     ++meshIndex)
	{
		const ufbx_mesh* mesh = sourceScene_->meshes.data[meshIndex];
		if (mesh->skin_deformers.count == 0 || mesh->skin_deformers.data[0]->clusters.count == 0)
		{
			continue;
		}
		const ufbx_node* root = mesh->skin_deformers.data[0]->clusters.data[0]->bone_node;
		while (root && root->parent && root->parent->bone)
		{
			root = root->parent;
		}
		for (size_t nodeIndex = 0; root && nodeIndex < sourceScene_->nodes.count; ++nodeIndex)
		{
			if (sourceScene_->nodes.data[nodeIndex] == root)
			{
				rootMotionNodeIndex_ = nodeIndex;
				rootMotionOrigin_ = {
				    static_cast<float>(root->node_to_world.m03), static_cast<float>(root->node_to_world.m13),
				    static_cast<float>(root->node_to_world.m23)};
				break;
			}
		}
	}

	size_t vertexCount = 0;
	for (size_t nodeIndex = 0; nodeIndex < sourceScene_->nodes.count; ++nodeIndex)
	{
		const ufbx_node* node = sourceScene_->nodes.data[nodeIndex];
		if (!node->mesh)
		{
			continue;
		}
		for (size_t faceIndex = 0; faceIndex < node->mesh->faces.count; ++faceIndex)
		{
			const uint32_t count = node->mesh->faces.data[faceIndex].num_indices;
			if (count >= 3)
			{
				vertexCount += static_cast<size_t>(count - 2) * 3;
			}
		}
	}

	if (vertexCount == 0)
	{
		lastError_ = "FBX contains no drawable mesh faces.";
		ReleaseScenes();
		return false;
	}

	// Bake CPU-skinned vertices once while loading. Runtime playback then only
	// changes a vertex-buffer offset and does no FBX evaluation or allocation.
	constexpr double kBakedFramesPerSecond = 15.0;
	constexpr size_t kAnimationMemoryBudget = 96u * 1024u * 1024u;
	const size_t frameBytes = sizeof(Vertex) * vertexCount;
	const size_t desiredFrames = animationDuration_ > 0.0
	                                 ? (std::max<size_t>)(2, static_cast<size_t>(std::ceil(animationDuration_ * kBakedFramesPerSecond)) + 1)
	                                 : 1;
	const size_t framesWithinBudget = frameBytes > 0 ? (std::max<size_t>)(1, kAnimationMemoryBudget / frameBytes) : 1;
	frameCount_ = (std::min)(desiredFrames, framesWithinBudget);

	if (!CreatePipeline() || !CreateColorBuffer() || !CreateVertexBuffer(vertexCount, frameCount_))
	{
		ReleaseScenes();
		return false;
	}

	if (animationStack_ && frameCount_ > 1)
	{
		for (size_t frameIndex = 0; frameIndex < frameCount_; ++frameIndex)
		{
			if (evaluatedScene_)
			{
				ufbx_free_scene(evaluatedScene_);
				evaluatedScene_ = nullptr;
			}
			const double ratio = static_cast<double>(frameIndex) / static_cast<double>(frameCount_ - 1);
			const double sampleTime = animationStack_->time_begin + animationDuration_ * ratio;
			ufbx_evaluate_opts evaluateOpts{};
			evaluateOpts.evaluate_skinning = true;
			ufbx_error evaluateError{};
			evaluatedScene_ = ufbx_evaluate_scene(
			    sourceScene_, animationStack_->anim, sampleTime, &evaluateOpts, &evaluateError);
			if (!evaluatedScene_)
			{
				lastError_ = evaluateError.description.data ? evaluateError.description.data : "Failed to bake FBX animation.";
				ReleaseScenes();
				return false;
			}
			if (frameIndex == 0 && rootMotionNodeIndex_ < evaluatedScene_->nodes.count)
			{
				const ufbx_node* root = evaluatedScene_->nodes.data[rootMotionNodeIndex_];
				rootMotionOrigin_ = {
				    static_cast<float>(root->node_to_world.m03), static_cast<float>(root->node_to_world.m13),
				    static_cast<float>(root->node_to_world.m23)};
			}
			if (!WriteVertices(evaluatedScene_, frameIndex))
			{
				ReleaseScenes();
				return false;
			}
		}
	}
	else if (!WriteVertices(sourceScene_, 0))
	{
		ReleaseScenes();
		return false;
	}
	if (evaluatedScene_)
	{
		ufbx_free_scene(evaluatedScene_);
		evaluatedScene_ = nullptr;
	}

	Vector3 minimum = mappedVertices_[0].position;
	Vector3 maximum = mappedVertices_[0].position;
	for (size_t index = 1; index < vertexCount_; ++index)
	{
		const Vector3& position = mappedVertices_[index].position;
		minimum.x = (std::min)(minimum.x, position.x);
		minimum.y = (std::min)(minimum.y, position.y);
		minimum.z = (std::min)(minimum.z, position.z);
		maximum.x = (std::max)(maximum.x, position.x);
		maximum.y = (std::max)(maximum.y, position.y);
		maximum.z = (std::max)(maximum.z, position.z);
	}
	boundsCenter_ = {(minimum.x + maximum.x) * 0.5f, (minimum.y + maximum.y) * 0.5f, (minimum.z + maximum.z) * 0.5f};
	boundsMaxExtent_ = (std::max)({maximum.x - minimum.x, maximum.y - minimum.y, maximum.z - minimum.z});
	if (boundsMaxExtent_ < 0.0001f)
	{
		boundsMaxExtent_ = 1.0f;
	}
	return true;
}

bool AnimatedModel::CreateVertexBuffer(size_t vertexCount, size_t frameCount)
{
	vertexCount_ = vertexCount;
	frameCount_ = frameCount;
	frameBoundsCenters_.assign(frameCount_, {});
	frameAimPoints_.assign(frameCount_, {});
	frameStrideBytes_ = static_cast<UINT64>(sizeof(Vertex) * vertexCount_);
	const UINT64 size = frameStrideBytes_ * frameCount_;
	const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	const HRESULT result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	    IID_PPV_ARGS(&vertexBuffer_));
	if (FAILED(result))
	{
		lastError_ = "Failed to create FBX vertex buffer.";
		return false;
	}

	if (FAILED(vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices_))))
	{
		lastError_ = "Failed to map FBX vertex buffer.";
		return false;
	}
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(frameStrideBytes_);
	vertexBufferView_.StrideInBytes = sizeof(Vertex);
	return true;
}

bool AnimatedModel::WriteVertices(const ufbx_scene* scene, size_t frameIndex)
{
	Vector3 rootMotionOffset{};
	if (rootMotionNodeIndex_ < scene->nodes.count)
	{
		const ufbx_node* root = scene->nodes.data[rootMotionNodeIndex_];
		rootMotionOffset = {
		    static_cast<float>(root->node_to_world.m03) - rootMotionOrigin_.x,
		    static_cast<float>(root->node_to_world.m13) - rootMotionOrigin_.y,
		    static_cast<float>(root->node_to_world.m23) - rootMotionOrigin_.z};
	}

	size_t destination = 0;
	for (size_t nodeIndex = 0; nodeIndex < scene->nodes.count; ++nodeIndex)
	{
		const ufbx_node* node = scene->nodes.data[nodeIndex];
		const ufbx_mesh* mesh = node->mesh;
		if (!mesh)
		{
			continue;
		}

		std::vector<uint32_t> triangleIndices(std::max<size_t>(mesh->max_face_triangles * 3, 3));
		for (size_t faceIndex = 0; faceIndex < mesh->faces.count; ++faceIndex)
		{
			const ufbx_face face = mesh->faces.data[faceIndex];
			const uint32_t triangleCount =
			    ufbx_triangulate_face(triangleIndices.data(), triangleIndices.size(), mesh, face);
			for (size_t index = 0; index < static_cast<size_t>(triangleCount) * 3; ++index)
			{
				if (destination >= vertexCount_)
				{
					lastError_ = "FBX triangulation produced an unexpected vertex count.";
					return false;
				}
				const uint32_t corner = triangleIndices[index];
				ufbx_vec3 position = ufbx_get_vertex_vec3(&mesh->skinned_position, corner);
				ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->skinned_normal, corner);
				if (mesh->skinned_is_local)
				{
					position = ufbx_transform_position(&node->geometry_to_world, position);
					normal = ufbx_transform_direction(&node->geometry_to_world, normal);
				}
				const double normalLength =
				    std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
				if (normalLength > 0.000001)
				{
					normal.x /= normalLength;
					normal.y /= normalLength;
					normal.z /= normalLength;
				}
				Vertex& output = mappedVertices_[frameIndex * vertexCount_ + destination];
				output.position = {
				    static_cast<float>(position.x) - rootMotionOffset.x,
				    static_cast<float>(position.y) - rootMotionOffset.y,
				    static_cast<float>(position.z) - rootMotionOffset.z};
				output.normal = {
				    static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z)};
				++destination;
			}
		}
	}

	if (destination != vertexCount_)
	{
		lastError_ = "FBX evaluated mesh topology changed during animation.";
		return false;
	}

	const size_t frameStart = frameIndex * vertexCount_;
	Vector3 minimum = mappedVertices_[frameStart].position;
	Vector3 maximum = minimum;
	for (size_t index = 1; index < vertexCount_; ++index)
	{
		const Vector3& position = mappedVertices_[frameStart + index].position;
		minimum.x = (std::min)(minimum.x, position.x);
		minimum.y = (std::min)(minimum.y, position.y);
		minimum.z = (std::min)(minimum.z, position.z);
		maximum.x = (std::max)(maximum.x, position.x);
		maximum.y = (std::max)(maximum.y, position.y);
		maximum.z = (std::max)(maximum.z, position.z);
	}
	frameBoundsCenters_[frameIndex] = {
	    (minimum.x + maximum.x) * 0.5f,
	    (minimum.y + maximum.y) * 0.5f,
	    (minimum.z + maximum.z) * 0.5f};
	if (aimNodeIndex_ < scene->nodes.count)
	{
		const ufbx_node* aimNode = scene->nodes.data[aimNodeIndex_];
		frameAimPoints_[frameIndex] = {
		    static_cast<float>(aimNode->node_to_world.m03) - rootMotionOffset.x,
		    static_cast<float>(aimNode->node_to_world.m13) - rootMotionOffset.y,
		    static_cast<float>(aimNode->node_to_world.m23) - rootMotionOffset.z};
	}
	else
	{
		frameAimPoints_[frameIndex] = frameBoundsCenters_[frameIndex];
	}
	return true;
}

void AnimatedModel::Update(float deltaTime)
{
	if (!sourceScene_ || !animationStack_ || animationDuration_ <= 0.0 || frameCount_ <= 1)
	{
		return;
	}

	const double begin = animationStack_->time_begin;
	animationTime_ += static_cast<double>((std::max)(deltaTime, 0.0f) * playbackSpeed_);
	animationTime_ = begin + std::fmod((std::max)(0.0, animationTime_ - begin), animationDuration_);
	const double ratio = (animationTime_ - begin) / animationDuration_;
	currentFrame_ = (std::min)(
	    frameCount_ - 1, static_cast<size_t>(std::floor(ratio * static_cast<double>(frameCount_ - 1) + 0.5)));
}

bool AnimatedModel::CreatePipeline()
{
	if (pipelineState_ && rootSignature_)
	{
		return true;
	}

	ComPtr<ID3DBlob> vertexShader = CompileShader(L"AnimatedModelVS.hlsl", "vs_5_0", lastError_);
	ComPtr<ID3DBlob> pixelShader = CompileShader(L"AnimatedModelPS.hlsl", "ps_5_0", lastError_);
	if (!vertexShader || !pixelShader)
	{
		return false;
	}

	CD3DX12_ROOT_PARAMETER rootParameters[3]{};
	rootParameters[0].InitAsConstantBufferView(0);
	rootParameters[1].InitAsConstantBufferView(1);
	rootParameters[2].InitAsConstantBufferView(2);
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc{};
	rootDesc.Init_1_0(
	    static_cast<UINT>(std::size(rootParameters)), rootParameters, 0, nullptr,
	    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootBlob;
	ComPtr<ID3DBlob> rootErrors;
	if (FAILED(D3DX12SerializeVersionedRootSignature(
	        &rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootBlob, &rootErrors)))
	{
		lastError_ = "Failed to serialize animated-model root signature.";
		return false;
	}

	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	if (FAILED(device->CreateRootSignature(
	        0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_))))
	{
		lastError_ = "Failed to create animated-model root signature.";
		return false;
	}

	const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
	pipelineDesc.pRootSignature = rootSignature_.Get();
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	pipelineDesc.InputLayout = {inputLayout, static_cast<UINT>(std::size(inputLayout))};
	pipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineDesc.NumRenderTargets = 1;
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	pipelineDesc.SampleDesc.Count = 1;

	if (FAILED(device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState_))))
	{
		lastError_ = "Failed to create animated-model graphics pipeline.";
		return false;
	}
	return true;
}

bool AnimatedModel::CreateColorBuffer()
{
	if (colorBuffer_)
	{
		return true;
	}
	const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
	const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(256);
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	if (FAILED(device->CreateCommittedResource(
	        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&colorBuffer_))))
	{
		lastError_ = "Failed to create animated-model color buffer.";
		return false;
	}
	if (FAILED(colorBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedColor_))))
	{
		lastError_ = "Failed to map animated-model color buffer.";
		return false;
	}
	*mappedColor_ = {0.85f, 0.35f, 0.12f, 1.0f};
	return true;
}

void AnimatedModel::SetColor(const Vector4& color)
{
	if (mappedColor_)
	{
		*mappedColor_ = color;
	}
}

void AnimatedModel::Draw(const WorldTransform& worldTransform, const Camera& camera) const
{
	if (!sourceScene_ || !pipelineState_ || vertexCount_ == 0)
	{
		return;
	}

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	commandList->SetPipelineState(pipelineState_.Get());
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW currentView = vertexBufferView_;
	currentView.BufferLocation += frameStrideBytes_ * currentFrame_;
	commandList->IASetVertexBuffers(0, 1, &currentView);
	commandList->SetGraphicsRootConstantBufferView(0, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, camera.GetConstBuffer()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(2, colorBuffer_->GetGPUVirtualAddress());
	commandList->DrawInstanced(static_cast<UINT>(vertexCount_), 1, 0, 0);
}

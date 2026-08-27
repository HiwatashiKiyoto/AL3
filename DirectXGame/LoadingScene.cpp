#include "LoadingScene.h"

#include "AnimatedModel.h"
#include "WorldTransformUpdate.h"

#include <chrono>
#include <filesystem>

using namespace KamataEngine;

LoadingScene::~LoadingScene()
{
	if (enemyLoadFuture_.valid())
	{
		delete enemyLoadFuture_.get();
	}
	delete preparedEnemyModel_;
	delete animatedModel_;
	for (Sprite* sprite : glyphSprites_)
	{
		delete sprite;
	}
	delete backgroundSprite_;
	if (fontTextureHandle_ != 0u)
	{
		TextureManager::Unload(fontTextureHandle_);
	}
}

void LoadingScene::Initialize(uint32_t blackTextureHandle)
{
	backgroundSprite_ = Sprite::Create(blackTextureHandle, {0.0f, 0.0f});
	backgroundSprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	backgroundSprite_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});

	fontTextureHandle_ = TextureManager::Load("debugfont.png");
	constexpr size_t kGlyphCount = 64;
	glyphSprites_.reserve(kGlyphCount);
	for (size_t i = 0; i < kGlyphCount; ++i)
	{
		glyphSprites_.push_back(Sprite::Create(fontTextureHandle_, {0.0f, 0.0f}));
	}

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.TransferMatrix();
	modelTransform_.Initialize();
	// Prepare the lightweight loading animation before the title appears. Scene
	// transitions can then display it immediately instead of spending their first
	// visible frames decoding this FBX.
	LoadAnimatedSilhouette();
}

void LoadingScene::Begin()
{
	frameCount_ = 0;
	delete preparedEnemyModel_;
	preparedEnemyModel_ = nullptr;
	backgroundLoadFinished_ = false;
	if (enemyLoadFuture_.valid())
	{
		delete enemyLoadFuture_.get();
	}
	StartBackgroundLoad();
}

void LoadingScene::Update()
{
	++frameCount_;
	if (animatedModel_)
	{
		animatedModel_->Update();
	}
	if (!preparedEnemyModel_ && enemyLoadFuture_.valid() &&
	    enemyLoadFuture_.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		preparedEnemyModel_ = enemyLoadFuture_.get();
		backgroundLoadFinished_ = true;
	}
}

void LoadingScene::Draw()
{
	Sprite::PreDraw();
	backgroundSprite_->Draw();
	Sprite::PostDraw();

	if (animatedModel_)
	{
		animatedModel_->Draw(modelTransform_, camera_);
	}

	Sprite::PreDraw();
	glyphSpriteIndex_ = 0;
	const int32_t dotCount = (frameCount_ / 20) % 4;
	DrawText("NOW LOADING" + std::string(static_cast<size_t>(dotCount), '.'), 500.0f, 330.0f, 2.2f,
	         {1.0f, 1.0f, 1.0f, 1.0f});
	Sprite::PostDraw();
}

void LoadingScene::LoadAnimatedSilhouette()
{
	if (animatedModel_)
	{
		return;
	}
	std::filesystem::path path = "Resources/roadScene.fbx";
	if (!std::filesystem::exists(path))
	{
		path = "DirectXGame/Resources/roadScene.fbx";
	}
	if (!std::filesystem::exists(path))
	{
		OutputDebugStringA("Loading animation not found: roadScene.fbx\n");
		return;
	}

	animatedModel_ = new AnimatedModel();
	if (!animatedModel_->Load(path.string()))
	{
		OutputDebugStringA(("Loading FBX failed: " + animatedModel_->GetLastError() + "\n").c_str());
		delete animatedModel_;
		animatedModel_ = nullptr;
		return;
	}
	animatedModel_->SetPlaybackSpeed(1.0f);
	animatedModel_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	const float scale = 18.0f / animatedModel_->GetBoundsMaxExtent();
	const Vector3 center = animatedModel_->GetBoundsCenter();
	modelTransform_.scale_ = {scale, scale, scale};
	modelTransform_.rotation_ = {0.0f, 3.14159265358979323846f, 0.0f};
	// Bottom-right placement. Centering by the FBX bounds keeps differently
	// authored transforms from moving the silhouette off-screen.
	modelTransform_.translation_ = {
	    3.5f + center.x * scale,
	    -1.6f - center.y * scale,
	    center.z * scale};
	UpdateWorldTransform(modelTransform_);
}

void LoadingScene::StartBackgroundLoad()
{
	enemyLoadFuture_ = std::async(std::launch::async, []() -> AnimatedModel* {
		std::filesystem::path path = "Resources/Enemy Bicycle/Enemy Bicycle.fbx";
		if (!std::filesystem::exists(path))
		{
			path = "DirectXGame/Resources/Enemy Bicycle/Enemy Bicycle.fbx";
		}
		if (!std::filesystem::exists(path))
		{
			OutputDebugStringA("Background FBX not found: Enemy Bicycle.fbx\n");
			return nullptr;
		}

		AnimatedModel* model = new AnimatedModel();
		if (!model->Load(path.string()))
		{
			OutputDebugStringA(("Background FBX load failed: " + model->GetLastError() + "\n").c_str());
			delete model;
			return nullptr;
		}
		model->Update(0.0f);
		return model;
	});
}

AnimatedModel* LoadingScene::TakePreparedEnemyModel()
{
	AnimatedModel* model = preparedEnemyModel_;
	preparedEnemyModel_ = nullptr;
	return model;
}

void LoadingScene::DrawText(const std::string& text, float x, float y, float scale, const Vector4& color)
{
	constexpr float kFontWidth = 9.0f;
	constexpr float kFontHeight = 18.0f;
	constexpr int32_t kCharactersPerRow = 14;
	for (const unsigned char character : text)
	{
		if (character < 32 || character > 126 || glyphSpriteIndex_ >= glyphSprites_.size())
		{
			x += kFontWidth * scale;
			continue;
		}
		const int32_t glyphIndex = static_cast<int32_t>(character) - 32;
		Sprite* sprite = glyphSprites_[glyphSpriteIndex_++];
		sprite->SetPosition({x, y});
		sprite->SetSize({kFontWidth * scale, kFontHeight * scale});
		sprite->SetColor(color);
		sprite->SetTextureRect(
		    {static_cast<float>(glyphIndex % kCharactersPerRow) * kFontWidth,
		     static_cast<float>(glyphIndex / kCharactersPerRow) * kFontHeight},
		    {kFontWidth, kFontHeight});
		sprite->Draw();
		x += kFontWidth * scale;
	}
}

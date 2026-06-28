#define NOMINMAX
#include "TitleScene.h"
#include "WorldTransformConfig.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

TitleScene::~TitleScene()
{
	delete spritePressSpace_;
	delete spriteTitle_;
	delete fade_;
	delete modelBlock_;
	delete modelPlayer_;
	delete modelSkydome_;
	delete camera_;
}

void TitleScene::Initialize()
{
	camera_ = new Camera();
	camera_->Initialize();
	camera_->translation_ = {0.0f, 0.2f, -12.0f};
	camera_->UpdateMatrix();

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelPlayer_ = Model::CreateFromOBJ("player", true);
	modelBlock_ = Model::CreateFromOBJ("block", true);
	textureHandleTitle_ = TextureManager::Load("title_destroy_murakami.png");
	textureHandlePressSpace_ = TextureManager::Load("title_press_space.png");
	spriteTitle_ = Sprite::Create(textureHandleTitle_, {260.0f, 80.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	spriteTitle_->SetSize({760.0f, 160.0f});
	spritePressSpace_ = Sprite::Create(textureHandlePressSpace_, {430.0f, 545.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
	spritePressSpace_->SetSize({420.0f, 70.0f});
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeDuration);

	worldTransformSkydome_.Initialize();

	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.translation_ = {0.0f, -0.7f, 0.0f};
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransformPlayer_.scale_ = {1.4f, 1.4f, 1.4f};

	worldTransformBlock_.Initialize();
	worldTransformBlock_.translation_ = {0.0f, -1.8f, 0.0f};
	worldTransformBlock_.scale_ = {1.6f, 0.35f, 1.6f};

	WorldTransformConfig(worldTransformSkydome_);
	WorldTransformConfig(worldTransformPlayer_);
	WorldTransformConfig(worldTransformBlock_);

	titleTimer_ = 0.0f;
	phase_ = Phase::kFadeIn;
	finished_ = false;
}

void TitleScene::Update()
{
	titleTimer_ += 1.0f / 60.0f;

	switch (phase_)
	{
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished())
		{
			fade_->Stop();
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE))
		{
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, kFadeDuration);
		}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished())
		{
			finished_ = true;
		}
		break;
	}

	worldTransformPlayer_.translation_.y = -0.7f + std::sin(titleTimer_ * 2.2f) * 0.18f;
	worldTransformPlayer_.rotation_.y += 0.025f;
	worldTransformBlock_.rotation_.y -= 0.012f;

	WorldTransformConfig(worldTransformSkydome_);
	WorldTransformConfig(worldTransformPlayer_);
	WorldTransformConfig(worldTransformBlock_);
	camera_->UpdateMatrix();
}

void TitleScene::Draw()
{
	Model::PreDraw();
	modelSkydome_->Draw(worldTransformSkydome_, *camera_);
	modelBlock_->Draw(worldTransformBlock_, *camera_);
	modelPlayer_->Draw(worldTransformPlayer_, *camera_);
	Model::PostDraw();

	Sprite::PreDraw();
	spriteTitle_->Draw();
	spritePressSpace_->Draw();
	Sprite::PostDraw();

	fade_->Draw();
}

#define NOMINMAX
#include "Fade.h"
#include <algorithm>

using namespace KamataEngine;

Fade::~Fade()
{
	delete sprite_;
}

void Fade::Initialize()
{
	textureHandle_ = TextureManager::Load("white1x1.png");
	sprite_ = Sprite::Create(textureHandle_, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f});
	sprite_->SetSize({1280.0f, 720.0f});
	color_ = {0.0f, 0.0f, 0.0f, 0.0f};
	sprite_->SetColor(color_);
	status_ = Status::None;
	duration_ = 0.0f;
	counter_ = 0.0f;
}

void Fade::Update()
{
	switch (status_)
	{
	case Status::None:
		break;
	case Status::FadeIn:
		counter_ += 1.0f / 60.0f;
		counter_ = std::min(counter_, duration_);
		color_.w = std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f);
		sprite_->SetColor(color_);
		break;
	case Status::FadeOut:
		counter_ += 1.0f / 60.0f;
		counter_ = std::min(counter_, duration_);
		color_.w = std::clamp(counter_ / duration_, 0.0f, 1.0f);
		sprite_->SetColor(color_);
		break;
	}
}

void Fade::Draw()
{
	if (status_ == Status::None)
	{
		return;
	}

	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration)
{
	status_ = status;
	duration_ = std::max(duration, 0.01f);
	counter_ = 0.0f;

	if (status_ == Status::FadeIn)
	{
		color_.w = 1.0f;
	}
	else if (status_ == Status::FadeOut)
	{
		color_.w = 0.0f;
	}

	sprite_->SetColor(color_);
}

void Fade::Stop()
{
	status_ = Status::None;
	counter_ = 0.0f;
	color_.w = 0.0f;
	sprite_->SetColor(color_);
}

bool Fade::IsFinished() const
{
	if (status_ == Status::None)
	{
		return true;
	}

	return counter_ >= duration_;
}

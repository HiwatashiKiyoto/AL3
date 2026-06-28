#pragma once

#include "KamataEngine.h"

class Fade
{
public:
	enum class Status
	{
		None,
		FadeIn,
		FadeOut,
	};

	~Fade();

	void Initialize();
	void Update();
	void Draw();
	void Start(Status status, float duration);
	void Stop();
	bool IsFinished() const;

private:
	KamataEngine::Sprite* sprite_ = nullptr;
	uint32_t textureHandle_ = 0;
	KamataEngine::Vector4 color_ = {0.0f, 0.0f, 0.0f, 0.0f};
	Status status_ = Status::None;
	float duration_ = 0.0f;
	float counter_ = 0.0f;
};

#pragma once

#include "KamataEngine.h"

class TitleScene
{
public:
	~TitleScene();

	void Initialize();
	void Update();
	void Draw();

	bool IsFinished() const { return finished_; }

private:
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Sprite* spriteTitle_ = nullptr;
	KamataEngine::Sprite* spritePressSpace_ = nullptr;
	uint32_t textureHandleTitle_ = 0;
	uint32_t textureHandlePressSpace_ = 0;
	KamataEngine::WorldTransform worldTransformSkydome_;
	KamataEngine::WorldTransform worldTransformPlayer_;
	KamataEngine::WorldTransform worldTransformBlock_;
	float titleTimer_ = 0.0f;
	bool finished_ = false;
};

#pragma once

#include "KamataEngine.h"

#include <future>
#include <string>
#include <vector>

class AnimatedModel;

// Lightweight interstitial shown before constructing tutorial/game resources.
class LoadingScene
{
public:
	~LoadingScene();
	void Initialize(uint32_t blackTextureHandle);
	void Begin();
	void Update();
	void Draw();
	bool IsReady() const { return backgroundLoadFinished_; }
	AnimatedModel* TakePreparedEnemyModel();

private:
	void LoadAnimatedSilhouette();
	void StartBackgroundLoad();
	void DrawText(const std::string& text, float x, float y, float scale, const KamataEngine::Vector4& color);

	KamataEngine::Sprite* backgroundSprite_ = nullptr;
	uint32_t fontTextureHandle_ = 0u;
	std::vector<KamataEngine::Sprite*> glyphSprites_;
	size_t glyphSpriteIndex_ = 0;
	AnimatedModel* animatedModel_ = nullptr;
	AnimatedModel* preparedEnemyModel_ = nullptr;
	std::future<AnimatedModel*> enemyLoadFuture_;
	bool backgroundLoadFinished_ = false;
	KamataEngine::WorldTransform modelTransform_;
	KamataEngine::Camera camera_;
	int32_t frameCount_ = 0;
};

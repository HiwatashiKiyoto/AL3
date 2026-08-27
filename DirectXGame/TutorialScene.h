#pragma once

#include "KamataEngine.h"

#include <string>
#include <vector>

class GameScene;
class AnimatedModel;

// Owns the interactive training scene and all tutorial-only presentation.
class TutorialScene
{
public:
	~TutorialScene();

	void Initialize(uint32_t overlayTextureHandle, AnimatedModel* preparedEnemyModel = nullptr);
	void Update();
	void Draw();
	bool IsComplete() const;
	bool ShouldReturnToTitle() const { return shouldReturnToTitle_; }
	bool ShouldExitGame() const { return shouldExitGame_; }

private:
	void DrawInstructions() const;
	void DrawPauseMenu() const;
	void DrawText(
	    const std::string& text, float x, float y, float scale,
	    const KamataEngine::Vector4& color = {0.02f, 0.02f, 0.02f, 1.0f}) const;

	GameScene* gameScene_ = nullptr;
	KamataEngine::Sprite* overlaySprite_ = nullptr;
	KamataEngine::Sprite* lowerShadeSprite_ = nullptr;
	KamataEngine::Sprite* pauseOverlaySprite_ = nullptr;
	uint32_t fontTextureHandle_ = 0;
	mutable std::vector<KamataEngine::Sprite*> glyphSprites_;
	mutable size_t glyphSpriteIndex_ = 0;
	bool isPaused_ = false;
	int32_t pauseSelection_ = 0;
	bool shouldReturnToTitle_ = false;
	bool shouldExitGame_ = false;
};

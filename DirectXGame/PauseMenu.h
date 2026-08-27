#pragma once

#include "KamataEngine.h"

#include <string>
#include <vector>

class PauseMenu
{
public:
	enum class Action
	{
		None,
		Resume,
		ReturnToTitle,
		ExitGame,
	};

	~PauseMenu();
	void Initialize(uint32_t overlayTextureHandle);
	Action Update();
	void Draw();
	bool IsOpen() const { return isOpen_; }

private:
	void DrawText(const std::string& text, float x, float y, float scale, const KamataEngine::Vector4& color);

	KamataEngine::Sprite* overlaySprite_ = nullptr;
	uint32_t fontTextureHandle_ = 0;
	std::vector<KamataEngine::Sprite*> glyphSprites_;
	size_t glyphSpriteIndex_ = 0;
	bool isOpen_ = false;
	int32_t selection_ = 0;
};

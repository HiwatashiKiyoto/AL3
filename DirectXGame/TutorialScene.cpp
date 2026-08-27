#include "TutorialScene.h"

#include "GameAudio.h"
#include "GameScene.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace
{
bool IsGamepadButtonTriggered(Input* input, WORD button)
{
	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	return input->GetJoystickState(0, current) && input->GetJoystickStatePrevious(0, previous) &&
	       (current.Gamepad.wButtons & button) != 0 && (previous.Gamepad.wButtons & button) == 0;
}

bool IsPauseTriggered(Input* input)
{
	return input->TriggerKey(DIK_ESCAPE) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_START);
}

bool IsCancelTriggered(Input* input)
{
	return input->TriggerKey(DIK_BACKSPACE) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_B);
}

bool IsLeftStickYTriggered(Input* input, bool up)
{
	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	if (!input->GetJoystickState(0, current) || !input->GetJoystickStatePrevious(0, previous))
	{
		return false;
	}
	const SHORT threshold = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	return up ? current.Gamepad.sThumbLY > threshold && previous.Gamepad.sThumbLY <= threshold
	          : current.Gamepad.sThumbLY < -threshold && previous.Gamepad.sThumbLY >= -threshold;
}

bool IsLeftStickXTriggered(Input* input, bool right)
{
	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	if (!input->GetJoystickState(0, current) || !input->GetJoystickStatePrevious(0, previous))
	{
		return false;
	}
	const SHORT threshold = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	return right ? current.Gamepad.sThumbLX > threshold && previous.Gamepad.sThumbLX <= threshold
	             : current.Gamepad.sThumbLX < -threshold && previous.Gamepad.sThumbLX >= -threshold;
}

std::string MakeVolumeText(const char* label, float volume)
{
	const int32_t filled = std::clamp(static_cast<int32_t>(std::round(volume * 10.0f)), 0, 10);
	std::string text = std::string(label) + " [";
	text.append(static_cast<size_t>(filled), '|');
	text.append(static_cast<size_t>(10 - filled), '.');
	text += "] " + std::to_string(filled * 10) + "%";
	return text;
}
}

TutorialScene::~TutorialScene()
{
	for (Sprite* glyphSprite : glyphSprites_)
	{
		delete glyphSprite;
	}
	delete pauseOverlaySprite_;
	delete lowerShadeSprite_;
	delete overlaySprite_;
	delete gameScene_;
	if (fontTextureHandle_ != 0u)
	{
		TextureManager::Unload(fontTextureHandle_);
	}
}

void TutorialScene::Initialize(uint32_t overlayTextureHandle, AnimatedModel* preparedEnemyModel)
{
	gameScene_ = new GameScene();
	gameScene_->Initialize(true, preparedEnemyModel);

	overlaySprite_ = Sprite::Create(overlayTextureHandle, {0.0f, 0.0f});
	overlaySprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	// The training-room texture contains near-white areas. Dimming only this
	// scene keeps the grid visible without changing the authored texture.
	overlaySprite_->SetColor({0.0f, 0.0f, 0.02f, 0.44f});
	const float lowerShadeY = static_cast<float>(WinApp::kWindowHeight) * 0.52f;
	lowerShadeSprite_ = Sprite::Create(overlayTextureHandle, {0.0f, lowerShadeY});
	lowerShadeSprite_->SetSize(
	    {static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight) - lowerShadeY});
	// The lower hemisphere of the authored room is much brighter than the top.
	// Apply a second subtle shade there while keeping the tutorial text untouched.
	lowerShadeSprite_->SetColor({0.0f, 0.01f, 0.025f, 0.24f});
	pauseOverlaySprite_ = Sprite::Create(overlayTextureHandle, {0.0f, 0.0f});
	pauseOverlaySprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	pauseOverlaySprite_->SetColor({0.0f, 0.0f, 0.0f, 0.62f});

	fontTextureHandle_ = TextureManager::Load("debugfont.png");
	constexpr size_t kGlyphSpriteCount = 256;
	glyphSprites_.reserve(kGlyphSpriteCount);
	for (size_t i = 0; i < kGlyphSpriteCount; ++i)
	{
		glyphSprites_.push_back(Sprite::Create(fontTextureHandle_, {0.0f, 0.0f}, {0.02f, 0.02f, 0.02f, 1.0f}));
	}
}

void TutorialScene::Update()
{
	Input* input = Input::GetInstance();
	if (IsPauseTriggered(input))
	{
		isPaused_ = !isPaused_;
		pauseSelection_ = 0;
		GameAudio::GetInstance()->PlaySe(isPaused_ ? GameAudio::Se::PauseOpen : GameAudio::Se::PauseClose);
		return;
	}

	if (isPaused_)
	{
		if (IsCancelTriggered(input))
		{
			isPaused_ = false;
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuCancel);
			return;
		}
		const bool selectUp = input->TriggerKey(DIK_UP) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_DPAD_UP) ||
		                      IsLeftStickYTriggered(input, true);
		const bool selectDown = input->TriggerKey(DIK_DOWN) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_DPAD_DOWN) ||
		                        IsLeftStickYTriggered(input, false);
		if (selectUp || selectDown)
		{
			pauseSelection_ = (pauseSelection_ + (selectDown ? 1 : 4)) % 5;
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuSelect);
		}

		const bool left = input->TriggerKey(DIK_LEFT) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_DPAD_LEFT) ||
		                  IsLeftStickXTriggered(input, false);
		const bool right = input->TriggerKey(DIK_RIGHT) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_DPAD_RIGHT) ||
		                   IsLeftStickXTriggered(input, true);
		if (left || right)
		{
			const float amount = right ? 0.1f : -0.1f;
			if (pauseSelection_ == 1)
			{
				GameAudio* audio = GameAudio::GetInstance();
				audio->SetBgmVolume(audio->GetBgmVolume() + amount);
			}
			else if (pauseSelection_ == 2)
			{
				GameAudio* audio = GameAudio::GetInstance();
				audio->SetSeVolume(audio->GetSeVolume() + amount);
			}
			if (pauseSelection_ == 1 || pauseSelection_ == 2)
			{
				GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuSelect);
			}
		}

		if (input->TriggerKey(DIK_SPACE) || IsGamepadButtonTriggered(input, XINPUT_GAMEPAD_A))
		{
			if (pauseSelection_ == 0)
			{
				GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
				isPaused_ = false;
			}
			else if (pauseSelection_ == 3)
			{
				GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
				shouldReturnToTitle_ = true;
			}
			else if (pauseSelection_ == 4)
			{
				GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
				shouldExitGame_ = true;
			}
		}
		return;
	}

	gameScene_->Updata();
}

void TutorialScene::Draw()
{
	gameScene_->Draw();

	Sprite::PreDraw();
	overlaySprite_->Draw();
	lowerShadeSprite_->Draw();
	glyphSpriteIndex_ = 0;
	if (isPaused_)
	{
		pauseOverlaySprite_->Draw();
		DrawPauseMenu();
	}
	else
	{
		DrawInstructions();
	}
	Sprite::PostDraw();
}

void TutorialScene::DrawPauseMenu() const
{
	const Vector4 white = {1.0f, 1.0f, 1.0f, 1.0f};
	const Vector4 selected = {1.0f, 0.82f, 0.18f, 1.0f};
	DrawText("PAUSE", 510.0f, 105.0f, 3.0f, white);
	DrawText("CONTINUE", 500.0f, 205.0f, 2.0f, pauseSelection_ == 0 ? selected : white);
	DrawText(MakeVolumeText("BGM VOLUME", GameAudio::GetInstance()->GetBgmVolume()), 390.0f, 270.0f, 1.6f,
	         pauseSelection_ == 1 ? selected : white);
	DrawText(MakeVolumeText("SE  VOLUME", GameAudio::GetInstance()->GetSeVolume()), 390.0f, 335.0f, 1.6f,
	         pauseSelection_ == 2 ? selected : white);
	DrawText("RETURN TO TITLE", 430.0f, 400.0f, 2.0f, pauseSelection_ == 3 ? selected : white);
	DrawText("EXIT GAME", 500.0f, 465.0f, 2.0f, pauseSelection_ == 4 ? selected : white);
	DrawText("UP/DOWN: SELECT   LEFT/RIGHT: VOLUME", 335.0f, 550.0f, 1.2f, white);
	DrawText("A: OK   B: BACK   MENU / ESC: CLOSE", 350.0f, 590.0f, 1.3f, white);
}

bool TutorialScene::IsComplete() const
{
	return gameScene_->IsTutorialComplete();
}

void TutorialScene::DrawInstructions() const
{
	DrawText("TUTORIAL", 450.0f, 105.0f, 3.0f);

	switch (gameScene_->GetTutorialStep())
	{
	case 0:
		DrawText("STEP 1 / 4", 525.0f, 220.0f, 1.5f);
		DrawText("MOVE", 545.0f, 275.0f, 2.5f);
		DrawText("LEFT STICK  /  ARROW KEYS", 390.0f, 355.0f, 2.0f);
		DrawText("MOVE ONCE TO CONTINUE", 455.0f, 430.0f, 1.5f);
		break;
	case 1:
		DrawText("STEP 2 / 4", 525.0f, 220.0f, 1.5f);
		DrawText("CAMERA", 500.0f, 275.0f, 2.5f);
		DrawText("RIGHT STICK  /  W A S D", 405.0f, 355.0f, 2.0f);
		DrawText("LOOK AROUND TO CONTINUE", 440.0f, 430.0f, 1.5f);
		break;
	case 2:
		DrawText("STEP 3 / 4", 525.0f, 220.0f, 1.5f);
		DrawText("AUTO LOCK-ON", 415.0f, 275.0f, 2.5f);
		DrawText("CENTER AN ENEMY TO LOCK ON", 390.0f, 355.0f, 1.8f);
		DrawText("KEEP THE TARGET LOCKED", 445.0f, 430.0f, 1.5f);
		break;
	case 3:
		DrawText("STEP 4 / 4", 525.0f, 220.0f, 1.5f);
		DrawText("SHOOT", 535.0f, 275.0f, 2.5f);
		DrawText("A BUTTON  /  SPACE KEY", 425.0f, 355.0f, 2.0f);
		DrawText("SHOOT ONCE TO START", 470.0f, 430.0f, 1.5f);
		break;
	default:
		DrawText("TUTORIAL COMPLETE", 350.0f, 285.0f, 2.5f);
		DrawText("STARTING MISSION...", 460.0f, 390.0f, 1.5f);
		break;
	}
}

void TutorialScene::DrawText(const std::string& text, float x, float y, float scale, const Vector4& color) const
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
		Sprite* glyphSprite = glyphSprites_[glyphSpriteIndex_++];
		glyphSprite->SetColor(color);
		glyphSprite->SetPosition({x, y});
		glyphSprite->SetSize({kFontWidth * scale, kFontHeight * scale});
		glyphSprite->SetTextureRect(
		    {static_cast<float>(glyphIndex % kCharactersPerRow) * kFontWidth,
		     static_cast<float>(glyphIndex / kCharactersPerRow) * kFontHeight},
		    {kFontWidth, kFontHeight});
		glyphSprite->Draw();
		x += kFontWidth * scale;
	}
}

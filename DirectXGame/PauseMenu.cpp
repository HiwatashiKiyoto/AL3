#include "PauseMenu.h"

#include "GameAudio.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace
{
bool IsButtonTriggered(Input* input, WORD button)
{
	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	return input->GetJoystickState(0, current) && input->GetJoystickStatePrevious(0, previous) &&
	       (current.Gamepad.wButtons & button) != 0 && (previous.Gamepad.wButtons & button) == 0;
}

bool IsPauseTriggered(Input* input)
{
	return input->TriggerKey(DIK_ESCAPE) || IsButtonTriggered(input, XINPUT_GAMEPAD_START);
}

bool IsCancelTriggered(Input* input)
{
	return input->TriggerKey(DIK_BACKSPACE) || IsButtonTriggered(input, XINPUT_GAMEPAD_B);
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

PauseMenu::~PauseMenu()
{
	for (Sprite* sprite : glyphSprites_)
	{
		delete sprite;
	}
	delete overlaySprite_;
	if (fontTextureHandle_ != 0u)
	{
		TextureManager::Unload(fontTextureHandle_);
	}
}

void PauseMenu::Initialize(uint32_t overlayTextureHandle)
{
	overlaySprite_ = Sprite::Create(overlayTextureHandle, {0.0f, 0.0f});
	overlaySprite_->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	overlaySprite_->SetColor({0.0f, 0.0f, 0.0f, 0.72f});

	fontTextureHandle_ = TextureManager::Load("debugfont.png");
	constexpr size_t kGlyphSpriteCount = 192;
	glyphSprites_.reserve(kGlyphSpriteCount);
	for (size_t i = 0; i < kGlyphSpriteCount; ++i)
	{
		glyphSprites_.push_back(Sprite::Create(fontTextureHandle_, {0.0f, 0.0f}));
	}
}

PauseMenu::Action PauseMenu::Update()
{
	Input* input = Input::GetInstance();
	if (IsPauseTriggered(input))
	{
		isOpen_ = !isOpen_;
		selection_ = 0;
		GameAudio::GetInstance()->PlaySe(isOpen_ ? GameAudio::Se::PauseOpen : GameAudio::Se::PauseClose);
		return isOpen_ ? Action::None : Action::Resume;
	}

	if (!isOpen_)
	{
		return Action::None;
	}
	if (IsCancelTriggered(input))
	{
		isOpen_ = false;
		GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuCancel);
		return Action::Resume;
	}

	const bool up = input->TriggerKey(DIK_UP) || IsButtonTriggered(input, XINPUT_GAMEPAD_DPAD_UP) ||
	                IsLeftStickYTriggered(input, true);
	const bool down = input->TriggerKey(DIK_DOWN) || IsButtonTriggered(input, XINPUT_GAMEPAD_DPAD_DOWN) ||
	                  IsLeftStickYTriggered(input, false);
	if (up || down)
	{
		selection_ = (selection_ + (down ? 1 : 4)) % 5;
		GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuSelect);
	}

	const bool left = input->TriggerKey(DIK_LEFT) || IsButtonTriggered(input, XINPUT_GAMEPAD_DPAD_LEFT) ||
	                  IsLeftStickXTriggered(input, false);
	const bool right = input->TriggerKey(DIK_RIGHT) || IsButtonTriggered(input, XINPUT_GAMEPAD_DPAD_RIGHT) ||
	                   IsLeftStickXTriggered(input, true);
	if (left || right)
	{
		const float amount = right ? 0.1f : -0.1f;
		if (selection_ == 1)
		{
			GameAudio* audio = GameAudio::GetInstance();
			audio->SetBgmVolume(audio->GetBgmVolume() + amount);
		}
		else if (selection_ == 2)
		{
			GameAudio* audio = GameAudio::GetInstance();
			audio->SetSeVolume(audio->GetSeVolume() + amount);
		}
		if (selection_ == 1 || selection_ == 2)
		{
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuSelect);
		}
	}

	if (input->TriggerKey(DIK_SPACE) || IsButtonTriggered(input, XINPUT_GAMEPAD_A))
	{
		if (selection_ == 0)
		{
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
			isOpen_ = false;
			return Action::Resume;
		}
		if (selection_ == 3)
		{
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
			isOpen_ = false;
			return Action::ReturnToTitle;
		}
		if (selection_ == 4)
		{
			GameAudio::GetInstance()->PlaySe(GameAudio::Se::MenuConfirm);
			isOpen_ = false;
			return Action::ExitGame;
		}
	}

	return Action::None;
}

void PauseMenu::Draw()
{
	if (!isOpen_)
	{
		return;
	}

	const Vector4 white = {1.0f, 1.0f, 1.0f, 1.0f};
	const Vector4 selected = {1.0f, 0.82f, 0.18f, 1.0f};
	Sprite::PreDraw();
	overlaySprite_->Draw();
	glyphSpriteIndex_ = 0;
	DrawText("PAUSE", 510.0f, 105.0f, 3.0f, white);
	DrawText("CONTINUE", 500.0f, 205.0f, 2.0f, selection_ == 0 ? selected : white);
	DrawText(MakeVolumeText("BGM VOLUME", GameAudio::GetInstance()->GetBgmVolume()), 390.0f, 270.0f, 1.6f,
	         selection_ == 1 ? selected : white);
	DrawText(MakeVolumeText("SE  VOLUME", GameAudio::GetInstance()->GetSeVolume()), 390.0f, 335.0f, 1.6f,
	         selection_ == 2 ? selected : white);
	DrawText("RETURN TO TITLE", 430.0f, 400.0f, 2.0f, selection_ == 3 ? selected : white);
	DrawText("EXIT GAME", 500.0f, 465.0f, 2.0f, selection_ == 4 ? selected : white);
	DrawText("UP/DOWN: SELECT   LEFT/RIGHT: VOLUME", 335.0f, 550.0f, 1.2f, white);
	DrawText("A: OK   B: BACK   MENU / ESC: CLOSE", 350.0f, 590.0f, 1.3f, white);
	Sprite::PostDraw();
}

void PauseMenu::DrawText(const std::string& text, float x, float y, float scale, const Vector4& color)
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

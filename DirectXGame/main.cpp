#include <Windows.h>
#include <cstdlib>
#include <string>
#include <vector>
#include "GameAudio.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "LoadingScene.h"
#include "PauseMenu.h"
#include "TutorialScene.h"

namespace
{
enum class SceneState
{
	kTitle,
	kTutorialChoice,
	kLoading,
	kTitleFadeOut,
	kTutorial,
	kTutorialFadeOut,
	kGameFadeIn,
	kGame,
	kGameOverFadeOut,
	kGameOverFadeIn,
	kGameOver,
	kGameClearFadeOut,
	kGameClearFadeIn,
	kGameClear,
};

enum class LoadingTarget
{
	Tutorial,
	Game,
};

const int32_t kFadeFrameCount = 30;

bool IsStartTriggered(KamataEngine::Input* input)
{
	if (input->TriggerKey(DIK_SPACE))
	{
		return true;
	}

	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	return input->GetJoystickState(0, current) && input->GetJoystickStatePrevious(0, previous) &&
	       (current.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0 && (previous.Gamepad.wButtons & XINPUT_GAMEPAD_A) == 0;
}

bool IsStartPressed(KamataEngine::Input* input)
{
	if (input->PushKey(DIK_SPACE))
	{
		return true;
	}

	XINPUT_STATE current{};
	return input->GetJoystickState(0, current) && (current.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
}

bool IsChoiceDirectionTriggered(KamataEngine::Input* input)
{
	if (input->TriggerKey(DIK_UP) || input->TriggerKey(DIK_DOWN) ||
	    input->TriggerKey(DIK_LEFT) || input->TriggerKey(DIK_RIGHT))
	{
		return true;
	}

	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	if (!input->GetJoystickState(0, current) || !input->GetJoystickStatePrevious(0, previous))
	{
		return false;
	}
	const WORD directions = XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN |
	                        XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT;
	const bool dpadTriggered = (current.Gamepad.wButtons & directions) != 0 &&
	                           (previous.Gamepad.wButtons & directions) == 0;
	const SHORT deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	const bool currentOutside = std::abs(static_cast<int>(current.Gamepad.sThumbLX)) > deadzone ||
	                            std::abs(static_cast<int>(current.Gamepad.sThumbLY)) > deadzone;
	const bool previousOutside = std::abs(static_cast<int>(previous.Gamepad.sThumbLX)) > deadzone ||
	                             std::abs(static_cast<int>(previous.Gamepad.sThumbLY)) > deadzone;
	return dpadTriggered || (currentOutside && !previousOutside);
}

bool IsExitTriggered(KamataEngine::Input* input)
{
	if (input->TriggerKey(DIK_ESCAPE))
	{
		return true;
	}
	XINPUT_STATE current{};
	XINPUT_STATE previous{};
	return input->GetJoystickState(0, current) && input->GetJoystickStatePrevious(0, previous) &&
	       (current.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0 &&
	       (previous.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) == 0;
}

}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	//////////////
	//初期化処理
	/////////////

	//エンジンの初期化
	KamataEngine::Initialize(L"ポラリスの宇宙防衛猫");
	using namespace KamataEngine;
	WinApp::GetInstance()->SetFullscreen(true);
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	GameAudio* gameAudio = GameAudio::GetInstance();
	gameAudio->Initialize();
	gameAudio->PlayBgm(GameAudio::Bgm::Title);

	// タイトル画面の初期化
	const uint32_t textureTitle = TextureManager::Load("title.png");
	const uint32_t textureBlack = TextureManager::Load("black.png");
	const uint32_t textureGameOver = TextureManager::Load("GameOver.png");
	const uint32_t textureGameClear = TextureManager::Load("GameClear.png");
	const uint32_t textureTutorialChoiceYes = TextureManager::Load("TutorialChoiceYes.png");
	const uint32_t textureTutorialChoiceNo = TextureManager::Load("TutorialChoiceNo.png");
	Sprite* spriteTitle = Sprite::Create(textureTitle, {0.0f, 0.0f});
	Sprite* spriteFade = Sprite::Create(textureBlack, {0.0f, 0.0f});
	Sprite* spriteGameOver = Sprite::Create(textureGameOver, {0.0f, 0.0f});
	Sprite* spriteGameClear = Sprite::Create(textureGameClear, {0.0f, 0.0f});
	Sprite* spriteTutorialChoiceYes = Sprite::Create(textureTutorialChoiceYes, {0.0f, 0.0f});
	Sprite* spriteTutorialChoiceNo = Sprite::Create(textureTutorialChoiceNo, {0.0f, 0.0f});
	const uint32_t textureDebugFont = TextureManager::Load("debugfont.png");
	std::vector<Sprite*> titlePromptGlyphs;
	titlePromptGlyphs.reserve(32);
	for (size_t i = 0; i < 32; ++i)
	{
		titlePromptGlyphs.push_back(Sprite::Create(textureDebugFont, {0.0f, 0.0f}));
	}
	spriteTitle->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteFade->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteGameOver->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteGameClear->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteTutorialChoiceYes->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteTutorialChoiceNo->SetSize({static_cast<float>(WinApp::kWindowWidth), static_cast<float>(WinApp::kWindowHeight)});
	spriteFade->SetColor({0.0f, 0.0f, 0.0f, 0.0f});

	// Keep tutorial-only lifetime and presentation outside the gameplay scene.
	GameScene* gameScene = nullptr;
	TutorialScene* tutorialScene = nullptr;
	PauseMenu* gamePauseMenu = nullptr;
	LoadingScene* loadingScene = new LoadingScene();
	loadingScene->Initialize(textureBlack);

	SceneState sceneState = SceneState::kTitle;
	int32_t fadeFrame = 0;
	bool isTitleStartArmed = true;
	bool hasCompletedTutorial = false;
	bool tutorialChoiceYes = false;
	bool destroyGameSessionAfterFrame = false;
	LoadingTarget loadingTarget = LoadingTarget::Tutorial;
	Input* input = Input::GetInstance();
	bool exitRequested = false;

	auto drawTitleExitPrompt = [&]() {
		const std::string text = "ESC / BACK : EXIT GAME";
		constexpr float glyphWidth = 9.0f;
		constexpr float glyphHeight = 18.0f;
		constexpr float scale = 1.25f;
		constexpr int32_t charactersPerRow = 14;
		float x = 505.0f;
		for (size_t i = 0; i < text.size() && i < titlePromptGlyphs.size(); ++i)
		{
			const unsigned char character = static_cast<unsigned char>(text[i]);
			if (character >= 32 && character <= 126)
			{
				const int32_t glyphIndex = static_cast<int32_t>(character) - 32;
				Sprite* glyph = titlePromptGlyphs[i];
				glyph->SetPosition({x, 670.0f});
				glyph->SetSize({glyphWidth * scale, glyphHeight * scale});
				glyph->SetColor({1.0f, 1.0f, 1.0f, 0.88f});
				glyph->SetTextureRect(
				    {static_cast<float>(glyphIndex % charactersPerRow) * glyphWidth,
				     static_cast<float>(glyphIndex / charactersPerRow) * glyphHeight},
				    {glyphWidth, glyphHeight});
				glyph->Draw();
			}
			x += glyphWidth * scale;
		}
	};

	auto createTutorial = [&](AnimatedModel* preparedEnemyModel) {
		tutorialScene = new TutorialScene();
		tutorialScene->Initialize(textureBlack, preparedEnemyModel);
	};
	auto createGame = [&](AnimatedModel* preparedEnemyModel) {
		gameScene = new GameScene();
		gameScene->Initialize(false, preparedEnemyModel);
		gamePauseMenu = new PauseMenu();
		gamePauseMenu->Initialize(textureBlack);
	};
	auto beginLoading = [&](LoadingTarget target) {
		loadingTarget = target;
		loadingScene->Begin();
		gameAudio->PlayBgm(target == LoadingTarget::Tutorial ? GameAudio::Bgm::Tutorial : GameAudio::Bgm::GamePlay);
		sceneState = SceneState::kLoading;
		fadeFrame = 0;
	};

	//メインループ
	while (true)
	{
		if (KamataEngine::Update())
		{
			break;
		}

#ifdef USE_IMGUI
		ImGuiManager::GetInstance()->Begin();
#endif

		switch (sceneState)
		{
		case SceneState::kTitle:
			if (IsExitTriggered(input))
			{
				exitRequested = true;
				break;
			}
			// The input used to leave a result screen must be released before
			// the title accepts a separate input to start the next game.
			if (!isTitleStartArmed)
			{
				if (!IsStartPressed(input))
				{
					isTitleStartArmed = true;
				}
			}
			else if (IsStartTriggered(input))
			{
				gameAudio->PlaySe(GameAudio::Se::MenuConfirm);
				if (hasCompletedTutorial)
				{
					tutorialChoiceYes = false;
					sceneState = SceneState::kTutorialChoice;
				}
				else
				{
					beginLoading(LoadingTarget::Tutorial);
				}
			}
			break;
		case SceneState::kTutorialChoice:
			if (IsChoiceDirectionTriggered(input))
			{
				tutorialChoiceYes = !tutorialChoiceYes;
				gameAudio->PlaySe(GameAudio::Se::MenuSelect);
			}
			if (input->TriggerKey(DIK_ESCAPE))
			{
				gameAudio->PlaySe(GameAudio::Se::MenuCancel);
				sceneState = SceneState::kTitle;
				isTitleStartArmed = false;
			}
			else if (IsStartTriggered(input))
			{
				gameAudio->PlaySe(GameAudio::Se::MenuConfirm);
				if (tutorialChoiceYes)
				{
					beginLoading(LoadingTarget::Tutorial);
				}
				else
				{
					beginLoading(LoadingTarget::Game);
				}
			}
			break;
		case SceneState::kLoading:
			loadingScene->Update();
			if (loadingScene->IsReady())
			{
				AnimatedModel* preparedEnemyModel = loadingScene->TakePreparedEnemyModel();
				if (loadingTarget == LoadingTarget::Tutorial)
				{
					createTutorial(preparedEnemyModel);
					sceneState = SceneState::kTutorial;
				}
				else
				{
					createGame(preparedEnemyModel);
					sceneState = SceneState::kGameFadeIn;
				}
				fadeFrame = 0;
			}
			break;
		case SceneState::kTitleFadeOut:
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kTutorial;
				fadeFrame = 0;
			}
			break;
		case SceneState::kTutorial:
			tutorialScene->Update();
			if (tutorialScene->ShouldExitGame())
			{
				exitRequested = true;
			}
			else if (tutorialScene->ShouldReturnToTitle())
			{
				delete tutorialScene;
				tutorialScene = nullptr;
				gameAudio->PlayBgm(GameAudio::Bgm::Title);
				sceneState = SceneState::kTitle;
				fadeFrame = 0;
				isTitleStartArmed = false;
			}
			else if (tutorialScene->IsComplete())
			{
				hasCompletedTutorial = true;
				loadingScene->Begin();
				sceneState = SceneState::kTutorialFadeOut;
				fadeFrame = 0;
			}
			break;
		case SceneState::kTutorialFadeOut:
			loadingScene->Update();
			if (++fadeFrame >= kFadeFrameCount)
			{
				delete tutorialScene;
				tutorialScene = nullptr;
				loadingTarget = LoadingTarget::Game;
				gameAudio->PlayBgm(GameAudio::Bgm::GamePlay);
				sceneState = SceneState::kLoading;
				fadeFrame = 0;
			}
			break;
		case SceneState::kGameFadeIn:
			gameScene->Updata();
			if (gameScene->IsGameOver())
			{
				gameAudio->PlayBgm(GameAudio::Bgm::GameOver);
				sceneState = SceneState::kGameOverFadeOut;
				fadeFrame = 0;
				break;
			}
			if (gameScene->IsGameClear())
			{
				gameAudio->PlayBgm(GameAudio::Bgm::GameClear);
				sceneState = SceneState::kGameClearFadeOut;
				fadeFrame = 0;
				break;
			}
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kGame;
			}
			break;
		case SceneState::kGame:
		{
			const PauseMenu::Action pauseAction = gamePauseMenu->Update();
			if (pauseAction == PauseMenu::Action::ExitGame)
			{
				exitRequested = true;
				break;
			}
			if (pauseAction == PauseMenu::Action::ReturnToTitle)
			{
				gameAudio->PlayBgm(GameAudio::Bgm::Title);
				destroyGameSessionAfterFrame = true;
				sceneState = SceneState::kTitle;
				fadeFrame = 0;
				isTitleStartArmed = false;
				break;
			}
			if (gamePauseMenu->IsOpen())
			{
				break;
			}

			gameScene->Updata();
			if (gameScene->IsGameOver())
			{
				gameAudio->PlayBgm(GameAudio::Bgm::GameOver);
				sceneState = SceneState::kGameOverFadeOut;
				fadeFrame = 0;
			}
			else if (gameScene->IsGameClear())
			{
				gameAudio->PlayBgm(GameAudio::Bgm::GameClear);
				sceneState = SceneState::kGameClearFadeOut;
				fadeFrame = 0;
			}
			break;
		}
		case SceneState::kGameOverFadeOut:
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kGameOverFadeIn;
				fadeFrame = 0;
			}
			break;
		case SceneState::kGameOverFadeIn:
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kGameOver;
			}
			break;
		case SceneState::kGameOver:
			if (IsStartTriggered(input))
			{
				gameAudio->PlaySe(GameAudio::Se::MenuConfirm);
				gameAudio->PlayBgm(GameAudio::Bgm::Title);
				destroyGameSessionAfterFrame = true;
				sceneState = SceneState::kTitle;
				fadeFrame = 0;
				isTitleStartArmed = false;
			}
			break;
		case SceneState::kGameClearFadeOut:
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kGameClearFadeIn;
				fadeFrame = 0;
			}
			break;
		case SceneState::kGameClearFadeIn:
			if (++fadeFrame >= kFadeFrameCount)
			{
				sceneState = SceneState::kGameClear;
			}
			break;
		case SceneState::kGameClear:
			if (IsStartTriggered(input))
			{
				gameAudio->PlaySe(GameAudio::Se::MenuConfirm);
				gameAudio->PlayBgm(GameAudio::Bgm::Title);
				destroyGameSessionAfterFrame = true;
				sceneState = SceneState::kTitle;
				fadeFrame = 0;
				isTitleStartArmed = false;
			}
			break;
		}

#ifdef USE_IMGUI
		ImGuiManager::GetInstance()->End();
#endif
		if (exitRequested)
		{
			break;
		}

		//描画開始
		dxCommon->PreDraw();

		if (sceneState == SceneState::kLoading)
		{
			loadingScene->Draw();
		}
		else if (sceneState == SceneState::kTitle || sceneState == SceneState::kTutorialChoice || sceneState == SceneState::kTitleFadeOut)
		{
			Sprite::PreDraw();
			spriteTitle->Draw();
			if (sceneState == SceneState::kTitle)
			{
				drawTitleExitPrompt();
			}
			if (sceneState == SceneState::kTutorialChoice)
			{
				spriteFade->SetColor({0.0f, 0.0f, 0.0f, 0.72f});
				spriteFade->Draw();
				(tutorialChoiceYes ? spriteTutorialChoiceYes : spriteTutorialChoiceNo)->Draw();
			}
			if (sceneState == SceneState::kTitleFadeOut)
			{
				const float alpha = static_cast<float>(fadeFrame) / static_cast<float>(kFadeFrameCount);
				spriteFade->SetColor({0.0f, 0.0f, 0.0f, alpha});
				spriteFade->Draw();
			}
			Sprite::PostDraw();
		}
		else if (sceneState == SceneState::kTutorial)
		{
			tutorialScene->Draw();
		}
		else if (sceneState == SceneState::kTutorialFadeOut)
		{
			loadingScene->Draw();
		}
		else if (sceneState == SceneState::kGameOverFadeIn || sceneState == SceneState::kGameOver)
		{
			Sprite::PreDraw();
			spriteGameOver->Draw();
			if (sceneState == SceneState::kGameOverFadeIn)
			{
				const float alpha = 1.0f - static_cast<float>(fadeFrame) / static_cast<float>(kFadeFrameCount);
				spriteFade->SetColor({0.0f, 0.0f, 0.0f, alpha});
				spriteFade->Draw();
			}
			Sprite::PostDraw();
		}
		else if (sceneState == SceneState::kGameClearFadeIn || sceneState == SceneState::kGameClear)
		{
			Sprite::PreDraw();
			spriteGameClear->Draw();
			if (sceneState == SceneState::kGameClearFadeIn)
			{
				const float alpha = 1.0f - static_cast<float>(fadeFrame) / static_cast<float>(kFadeFrameCount);
				spriteFade->SetColor({0.0f, 0.0f, 0.0f, alpha});
				spriteFade->Draw();
			}
			Sprite::PostDraw();
		}
		else
		{
			gameScene->Draw();

			if (sceneState == SceneState::kGameFadeIn || sceneState == SceneState::kGameOverFadeOut ||
			    sceneState == SceneState::kGameClearFadeOut)
			{
				const float alpha = sceneState == SceneState::kGameFadeIn
				                        ? 1.0f - static_cast<float>(fadeFrame) / static_cast<float>(kFadeFrameCount)
				                        : static_cast<float>(fadeFrame) / static_cast<float>(kFadeFrameCount);
				spriteFade->SetColor({0.0f, 0.0f, 0.0f, alpha});
				Sprite::PreDraw();
				spriteFade->Draw();
				Sprite::PostDraw();
			}

			if (sceneState == SceneState::kGame && gamePauseMenu != nullptr)
			{
				gamePauseMenu->Draw();
			}
		}

#ifdef USE_IMGUI
		ImGuiManager::GetInstance()->Draw();
#endif

		//描画終了
		dxCommon->PostDraw();

		// Destroy scene-owned GPU resources only after the frame that changed the
		// state has finished drawing. This avoids the result-to-title use-after-free.
		if (destroyGameSessionAfterFrame)
		{
			delete gamePauseMenu;
			gamePauseMenu = nullptr;
			delete gameScene;
			gameScene = nullptr;
			destroyGameSessionAfterFrame = false;
		}
	}

	// Scene-owned sprites must be released before their texture handles.
	delete gameScene;
	delete tutorialScene;
	delete gamePauseMenu;
	delete loadingScene;
	gameScene = nullptr;
	tutorialScene = nullptr;
	gamePauseMenu = nullptr;
	loadingScene = nullptr;
	for (Sprite* glyph : titlePromptGlyphs)
	{
		delete glyph;
	}
	titlePromptGlyphs.clear();

	// タイトル画面の解放
	delete spriteGameClear;
	delete spriteGameOver;
	delete spriteTutorialChoiceNo;
	delete spriteTutorialChoiceYes;
	delete spriteFade;
	delete spriteTitle;
	TextureManager::Unload(textureGameClear);
	TextureManager::Unload(textureGameOver);
	TextureManager::Unload(textureTutorialChoiceNo);
	TextureManager::Unload(textureTutorialChoiceYes);
	TextureManager::Unload(textureBlack);
	TextureManager::Unload(textureTitle);
	TextureManager::Unload(textureDebugFont);

	gameAudio->Finalize();

	//エンジンの終了処理
	KamataEngine::Finalize();

	///////////////////
	// 更新処理開始
	///////////////////



	///////////////////
	// 更新処理終了
	//////////////////



	//////////////////
	// 描画処理開始
	//////////////////




	//////////////////
	// 描画処理終了
	//////////////////

	return 0;
}

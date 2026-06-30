#include <Windows.h>
#include"KamataEngine.h"
#include"GameScene.h"
#include"TitleScene.h"
#include"StageManager.h"

#include <fstream>
#include <sstream>
#include <string>

enum class Scene
{
	kUnknown = 0,
	kTitle,
	kGame,
};

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
StageManager* stageManager = nullptr;
Scene scene = Scene::kUnknown;

void LoadDebugSettings()
{
	const std::string filePath = "DebugSettings.ini";
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		return;
	}

	std::stringstream debugSettings;
	debugSettings << file.rdbuf();
	file.close();

	std::string line;
	while (std::getline(debugSettings, line))
	{
		if (line.empty())
		{
			continue;
		}

		std::istringstream lineStream(line);
		std::string key;
		std::string value;
		std::getline(lineStream, key, '=');
		std::getline(lineStream, value, '=');
		if (!value.empty() && value.back() == '\r')
		{
			value.pop_back();
		}

		if (key == "InitialStage")
		{
			stageManager->SetCurrentStageIndexByName(value);
		}
	}
}

void ChangeScene()
{
	switch (scene)
	{
	case Scene::kTitle:
		if (titleScene->IsFinished())
		{
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;
	case Scene::kGame:
		if (gameScene->IsFinished())
		{
			scene = Scene::kTitle;
			delete gameScene;
			gameScene = nullptr;
			titleScene = new TitleScene();
			titleScene->Initialize();
		}
		else if (gameScene->IsReloadRequested())
		{
			delete gameScene;
			gameScene = nullptr;
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;
	default:
		break;
	}
}

void UpdateScene()
{
	switch (scene)
	{
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	default:
		break;
	}
}

void DrawScene()
{
	switch (scene)
	{
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	default:
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	//////////////
	//初期化処理
	/////////////

	//エンジンの初期化
	KamataEngine::Initialize(L"LC1B_24_ヒワタシ_キヨト_AL2");
	using namespace KamataEngine;
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	stageManager = new StageManager();
	stageManager->LoadStageDataFile();

#ifdef _DEBUG
	LoadDebugSettings();
#endif

	//ゲームシーンのインスタンス生成
	scene = Scene::kTitle;
	titleScene = new TitleScene();

	//ゲームシーンの初期化
	titleScene->Initialize();

	//メインループ
	while (true)
	{
		if (KamataEngine::Update())
		{
			break;
		}

		//ゲームシーンの更新
#ifdef _DEBUG
		ImGuiManager::GetInstance()->Begin();
#endif
		UpdateScene();
		ChangeScene();
#ifdef _DEBUG
		ImGuiManager::GetInstance()->End();
#endif

		//描画開始
		dxCommon->PreDraw();

		//ゲームシーンの描画
		DrawScene();
#ifdef _DEBUG
		ImGuiManager::GetInstance()->Draw();
#endif

		//描画終了
		dxCommon->PostDraw();
	}

	//ゲームシーンの解放
	delete titleScene;
	delete gameScene;
	delete stageManager;

	//nullptrの代入
	titleScene = nullptr;
	gameScene = nullptr;
	stageManager = nullptr;

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

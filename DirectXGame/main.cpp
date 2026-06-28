#include <Windows.h>
#include"KamataEngine.h"
#include"GameScene.h"
#include"TitleScene.h"

enum class Scene
{
	kUnknown = 0,
	kTitle,
	kGame,
};

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
Scene scene = Scene::kUnknown;

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
			gameScene->Initialize();
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
		ChangeScene();
		UpdateScene();

		//描画開始
		dxCommon->PreDraw();

		//ゲームシーンの描画
		DrawScene();

		//描画終了
		dxCommon->PostDraw();
	}

	//ゲームシーンの解放
	delete titleScene;
	delete gameScene;

	//nullptrの代入
	titleScene = nullptr;
	gameScene = nullptr;

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

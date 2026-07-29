#include <Windows.h>
#include"KamataEngine.h"
#include"GameScene.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) 
{
	//////////////
	//初期化処理
	/////////////

	//エンジンの初期化
	KamataEngine::Initialize(L"LE2B_23_ヒワタシ_キヨト_AL3");
	using namespace KamataEngine;
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//ゲームシーンのインスタンス生成
	GameScene* gameScene = new GameScene();

	//ゲームシーンの初期化
	gameScene->Initialize();

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

		//ゲームシーンの更新
		gameScene->Updata();

#ifdef USE_IMGUI
		ImGuiManager::GetInstance()->End();
#endif

		//描画開始
		dxCommon->PreDraw();

		//ゲームシーンの描画
		gameScene->Draw();

#ifdef USE_IMGUI
		ImGuiManager::GetInstance()->Draw();
#endif

		//描画終了
		dxCommon->PostDraw();
	}

	//ゲームシーンの解放
	delete gameScene;

	//nullptrの代入
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

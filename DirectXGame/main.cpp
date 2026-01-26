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
	KamataEngine::Initialize(L"LC1B_24_ヒワタシ_キヨト_AL2");
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

		//ゲームシーンの更新
		gameScene->Updata();

		//描画開始
		dxCommon->PreDraw();

		//ゲームシーンの描画
		gameScene->Draw();

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

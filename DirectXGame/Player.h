#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Player 
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model,uint32_t textureHandle, KamataEngine::Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();


	/// <summary>
	/// 初期化
	/// </summary>
	void Draw();

	private:
		//ワールド変換データ
	    WorldTransform worldTransform_;
		
		//モデル
	    Model* model_ = nullptr;

		uint32_t textureHandle_ = 0u;
		KamataEngine::Camera* camera_ = nullptr;

};

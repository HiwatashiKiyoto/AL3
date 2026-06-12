#pragma once
/// <summary>
/// マップチップフィールド
/// </summary>

#include <KamataEngine.h>

enum class MapChipType
{
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData 
{
	std::vector<std::vector<MapChipType>> data;
};

struct IndexSet 
{
	uint32_t xIndex;
	uint32_t yIndex;
};

//範囲矩形
struct Rect
{
	float left;	  //左端
	float right;  //右端
	float bottom; //下端
	float top;	  //上端
};

class MapChipField 
{
public:
	// 一ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }

	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
};

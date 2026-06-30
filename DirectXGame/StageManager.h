#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct StageData
{
	std::string name;
	int32_t timeLimit = 0;
};

class StageManager
{
public:
	void LoadStageDataFile();

	const StageData& GetStageData(int32_t index) const;
	const StageData& GetCurrentStageData() const;

	void SetCurrentStageIndex(int32_t index);
	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }
	void SetCurrentStageIndexByName(const std::string& name);

private:
	std::vector<StageData> stageDatas_;
	int32_t currentStageIndex_ = 0;
};

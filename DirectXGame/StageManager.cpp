#include "StageManager.h"

#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>

void StageManager::LoadStageDataFile()
{
	const std::string filePath = "Resources/stageDatas.csv";

	std::ifstream file(filePath);
	assert(file.is_open() && "Stage data file does not exist.");

	std::stringstream stageDataCsv;
	stageDataCsv << file.rdbuf();
	file.close();

	stageDatas_.clear();

	std::string line;
	while (std::getline(stageDataCsv, line))
	{
		if (line.empty())
		{
			continue;
		}

		std::istringstream lineStream(line);
		StageData stageData;
		std::string word;

		std::getline(lineStream, word, ',');
		stageData.name = word;

		std::getline(lineStream, word, ',');
		stageData.timeLimit = std::stoi(word);

		stageDatas_.push_back(stageData);
	}

	assert(!stageDatas_.empty() && "Stage data is empty.");
}

const StageData& StageManager::GetStageData(int32_t index) const
{
	assert(0 <= index && index < static_cast<int32_t>(stageDatas_.size()));
	return stageDatas_[index];
}

const StageData& StageManager::GetCurrentStageData() const
{
	return GetStageData(currentStageIndex_);
}

void StageManager::SetCurrentStageIndex(int32_t index)
{
	assert(0 <= index && index < static_cast<int32_t>(stageDatas_.size()));
	currentStageIndex_ = index;
}

void StageManager::SetCurrentStageIndexByName(const std::string& name)
{
	for (std::size_t i = 0; i < stageDatas_.size(); ++i)
	{
		if (stageDatas_[i].name == name)
		{
			currentStageIndex_ = static_cast<int32_t>(i);
			return;
		}
	}

	assert(false && "Specified stage does not exist.");
}

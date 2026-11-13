#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class stageDataLoad {
    friend struct std::default_delete<stageDataLoad>;

public:
    static stageDataLoad* GetInstance();

    stageDataLoad(const stageDataLoad&) = delete;
    stageDataLoad& operator=(const stageDataLoad&) = delete;

public:
    void SetStage(int loadstage);

    const std::string& GetStageObjectData() const { return loadStageObjectData; };
    const std::string& GetStageData() const { return loadStageData; };
    const std::string& GetEnemyPopData() const { return loadEnemyPopDat; };
    const int GetloadNumber() const { return loadNumber; };
    const int GetMaxStage() const { return maxStage; }

private:
    stageDataLoad() = default;
    ~stageDataLoad() = default;

private:
    static std::unique_ptr<stageDataLoad> instance_;

    static constexpr int maxStage = 2;
    int loadNumber = 1;
    std::string loadStageObjectData;
    std::string loadStageData;
    std::string loadEnemyPopDat;
};
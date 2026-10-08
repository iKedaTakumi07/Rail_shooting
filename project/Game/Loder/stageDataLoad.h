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

    std::string GetStageObjectData() const { return loadStageObjectData; };
    std::string GetStageData() const { return loadStageData; };
    std::string GetEnemyPopData() const { return loadEnemyPopDat; };

private:
    stageDataLoad() = default;
    ~stageDataLoad() = default;

private:
    static std::unique_ptr<stageDataLoad> instance_;

    int loadNumber = 1;
    std::string loadStageObjectData;
    std::string loadStageData;
    std::string loadEnemyPopDat;
};
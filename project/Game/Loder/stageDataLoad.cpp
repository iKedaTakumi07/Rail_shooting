#include "stageDataLoad.h"

std::unique_ptr<stageDataLoad> stageDataLoad::instance_ = nullptr;

stageDataLoad* stageDataLoad::GetInstance()
{
    if (!instance_) {
        // privateコンストラクタのため new を使って unique_ptr を生成
        instance_ = std::unique_ptr<stageDataLoad>(new stageDataLoad());
    }
    return instance_.get();
}

void stageDataLoad::SetStage(int loadstage)
{
    loadNumber = loadstage;
    if (loadNumber == 1) {
        loadStageObjectData = "resources/StageData/stageObjectData1.json";
        loadStageData = "resources/StageData/stageData1.json";
        loadEnemyPopDat = "resources/StageData/enemyPopData1.json";
    } else if (loadNumber == 2) {
        loadStageObjectData = "resources/StageData/stageObjectData2.json";
        loadStageData = "resources/StageData/stageData2.json";
        loadEnemyPopDat = "resources/StageData/enemyPopData2.json";
    }
}
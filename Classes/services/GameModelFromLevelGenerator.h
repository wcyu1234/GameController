#ifndef __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
#define __GAME_MODEL_FROM_LEVEL_GENERATOR_H__

#include <memory>
#include "configs/models/LevelConfig.h"
#include "models/GameModel.h"

// GameModelFromLevelGenerator：把静态 LevelConfig 转换成运行时 GameModel。
// 服务层无状态：不持有数据，纯静态方法。
// 后续若加入随机洗牌、难度调整、特殊关卡牌型生成等策略，扩展点都在此处。
class GameModelFromLevelGenerator
{
public:
    // 从关卡配置生成可玩的 GameModel；config 为空时返回 nullptr。
    static std::unique_ptr<GameModel> generate(const LevelConfig& config);
};

#endif

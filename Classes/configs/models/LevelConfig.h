#ifndef __LEVEL_CONFIG_H__
#define __LEVEL_CONFIG_H__

#include <vector>
#include "cocos2d.h"
#include "configs/models/CardTypes.h"

// LevelCardConfig: static description of one card in a level JSON.
// Holds only data parsed from JSON; no runtime state.
struct LevelCardConfig
{
    CardFaceType face = CFT_NONE;
    CardSuitType suit = CST_NONE;
    cocos2d::Vec2 position;
};

// LevelConfig: static description of a whole level.
// Loaded by LevelConfigLoader from JSON; consumed by GameModelFromLevelGenerator.
class LevelConfig
{
public:
    const std::vector<LevelCardConfig>& getPlayfield() const { return _playfield; }
    const std::vector<LevelCardConfig>& getStack() const { return _stack; }

    std::vector<LevelCardConfig>& mutablePlayfield() { return _playfield; }
    std::vector<LevelCardConfig>& mutableStack() { return _stack; }

private:
    std::vector<LevelCardConfig> _playfield;
    std::vector<LevelCardConfig> _stack;
};

#endif

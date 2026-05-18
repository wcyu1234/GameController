#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "controllers/GameController.h"

// GameScene：游戏场景，持有 GameController。
// 作为 cocos2d Scene 接入导演，负责创建 GameController 并启动关卡。
class GameScene : public cocos2d::Scene
{
public:
    // 创建并启动第 levelId 关；失败时返回 nullptr。
    static GameScene* create(int levelId = 1);

    virtual bool init(int levelId);

private:
    GameController* _gameController = nullptr;
};

#endif

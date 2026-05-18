#include "GameScene.h"

USING_NS_CC;

GameScene* GameScene::create(int levelId)
{
    auto* scene = new (std::nothrow) GameScene();
    if (scene && scene->init(levelId))
    {
        scene->autorelease();
        return scene;
    }
    delete scene;
    return nullptr;
}

bool GameScene::init(int levelId)
{
    if (!Scene::init()) return false;

    _gameController = new (std::nothrow) GameController();
    if (!_gameController) return false;

    if (!_gameController->startGame(levelId, this))
    {
        delete _gameController;
        _gameController = nullptr;
        return false;
    }

    return true;
}

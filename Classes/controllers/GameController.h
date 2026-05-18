#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include <memory>
#include "cocos2d.h"
#include "models/GameModel.h"
#include "models/UndoModel.h"
#include "views/GameView.h"
#include "controllers/PlayFieldController.h"
#include "controllers/StackController.h"
#include "managers/UndoManager.h"

// GameController：顶层游戏控制器，管理整局游戏的生命周期。
// 职责：
//   - 加载关卡配置并生成 GameModel；
//   - 持有 GameView、子控制器（PlayFieldController、StackController）以及 UndoManager；
//   - 把 UndoManager 的回滚执行器绑定到对应的子控制器；
//   - 响应 Undo 按钮点击并刷新按钮可用态。
// 用法：在 scene init 中 new GameController，调用 startGame，再 addChild(getView())。
class GameController
{
public:
    GameController();
    ~GameController() = default;

    // 加载指定关卡，初始化所有子系统。关卡文件加载失败时返回 false。
    bool startGame(int levelId, cocos2d::Node* parentNode);

    // 返回根视图（已 add 到 parentNode，调用方无需再 addChild）。
    GameView* getView() const { return _gameView; }

private:
    void onUndoClick();
    void executeUndo(const UndoModel::Action& action);
    void refreshUndoButton();

    std::unique_ptr<GameModel>  _gameModel;
    GameView*                   _gameView    = nullptr;

    PlayFieldController         _playFieldCtrl;
    StackController             _stackCtrl;
    UndoManager                 _undoMgr;
};

#endif

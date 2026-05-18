#include "controllers/GameController.h"

#include "configs/loaders/LevelConfigLoader.h"
#include "services/GameModelFromLevelGenerator.h"
#include "views/CardView.h"

USING_NS_CC;

GameController::GameController() = default;

bool GameController::startGame(int levelId, cocos2d::Node* parentNode)
{
    // 1) 加载关卡配置。
    auto levelCfg = LevelConfigLoader::loadLevelConfig(levelId);
    if (!levelCfg)
    {
        CCLOGERROR("[GameController] failed to load level %d", levelId);
        return false;
    }

    // 2) 生成运行时 GameModel。
    _gameModel = GameModelFromLevelGenerator::generate(*levelCfg);
    if (!_gameModel)
    {
        CCLOGERROR("[GameController] failed to generate GameModel");
        return false;
    }

    // 3) 创建根视图并挂到 scene 上。
    _gameView = GameView::create();
    parentNode->addChild(_gameView);

    PlayFieldView* pfv = _gameView->getPlayFieldView();
    StackView*     sv  = _gameView->getStackView();

    // 4) 初始化 UndoManager，注入回滚执行器。
    _undoMgr.init(_gameModel.get(), [this](const UndoModel::Action& a) {
        executeUndo(a);
    });
    // 撤销栈状态变化时（push / pop / clear）刷新按钮可点态——
    // controller 子层只调 record/undo，不需要也不应感知 GameController。
    _undoMgr.setChangeListener([this]() { refreshUndoButton(); });

    // 5) 初始化子控制器。
    _playFieldCtrl.init(_gameModel.get(), pfv, sv, &_undoMgr);
    _stackCtrl.init(_gameModel.get(), sv, &_undoMgr);

    // 6) 构建桌面牌 CardView。
    for (int id : _gameModel->playfieldIds())
    {
        CardModel* cm = _gameModel->getCard(id);
        if (!cm) continue;
        CardView* cv = CardView::create(cm);
        pfv->addCardView(cv);
    }

    // 7) 构建备用牌堆 CardView。
    const auto& stackIds = _gameModel->stackIds();
    for (size_t i = 0; i < stackIds.size(); ++i)
    {
        int id = stackIds[i];
        CardModel* cm = _gameModel->getCard(id);
        if (!cm) continue;
        CardView* cv = CardView::create(cm);
        cv->setPosition(sv->getStackSlotPosition());
        cv->setLocalZOrder(static_cast<int>(i + 1));
        sv->addStackCardView(cv);
        cm->setPosition(sv->getStackSlotPosition());
    }

    // 8) 构建初始 tray 顶牌 CardView。
    for (int id : _gameModel->trayIds())
    {
        CardModel* cm = _gameModel->getCard(id);
        if (!cm) continue;
        CardView* cv = CardView::create(cm);
        cv->setPosition(sv->getTraySlotPosition());
        cv->setInteractive(false);
        sv->addTrayCardView(cv);
        cm->setPosition(sv->getTraySlotPosition());
    }

    // 9) 绑定 Undo 按钮回调，并把按钮初始化为禁用态（此刻撤销栈为空）。
    _gameView->setOnUndoClick([this]() { onUndoClick(); });
    refreshUndoButton();

    return true;
}

void GameController::onUndoClick()
{
    _undoMgr.undo();
}

void GameController::executeUndo(const UndoModel::Action& action)
{
    switch (action.type)
    {
        case UndoModel::AT_PLAYFIELD_TO_TRAY:
            _playFieldCtrl.undoPlayfieldToTray(action);
            break;
        case UndoModel::AT_STACK_TO_TRAY:
            _stackCtrl.undoStackToTray(action);
            break;
        default:
            CCLOGWARN("[GameController] unknown undo action type %d", action.type);
            break;
    }
    // 按钮可点态由 UndoManager 的 ChangeListener 统一刷新，这里无需再调。
}

void GameController::refreshUndoButton()
{
    if (_gameView)
        _gameView->setUndoEnabled(_undoMgr.canUndo());
}

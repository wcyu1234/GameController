#ifndef __PLAY_FIELD_CONTROLLER_H__
#define __PLAY_FIELD_CONTROLLER_H__

#include "models/GameModel.h"
#include "models/UndoModel.h"
#include "views/PlayFieldView.h"
#include "views/StackView.h"

class UndoManager;

// PlayFieldController：处理主牌区的交互。
// 响应桌面牌点击；若与 tray 顶牌匹配，则把该牌平移到 tray 并在 UndoManager 中记录撤销动作。
class PlayFieldController
{
public:
    PlayFieldController() = default;

    // 注入运行时依赖；不获取任何所有权。
    void init(GameModel* model, PlayFieldView* playFieldView, StackView* stackView, UndoManager* undoMgr);

    // 处理一次桌面牌点击：若与 tray 顶牌满足匹配规则，
    // 触发完整的"桌面牌 → tray"流程（记录撤销 + 更新 model + 播放视图动画）。
    bool handleCardClick(int cardId);

    // 由 GameController 在 UndoManager 触发 AT_PLAYFIELD_TO_TRAY 回滚时调用。
    void undoPlayfieldToTray(const UndoModel::Action& a);

private:
    void replaceTrayWithPlayFieldCard(int cardId);

    GameModel*      _model        = nullptr;
    PlayFieldView*  _playFieldView = nullptr;
    StackView*      _stackView    = nullptr;
    UndoManager*    _undoMgr      = nullptr;
};

#endif

#ifndef __STACK_CONTROLLER_H__
#define __STACK_CONTROLLER_H__

#include "models/GameModel.h"
#include "models/UndoModel.h"
#include "views/StackView.h"

class UndoManager;

// StackController：处理备用牌堆交互。
// 职责：响应备用牌点击，将备用堆顶牌移动到 tray 顶部，并记录回退动作。
// 不处理匹配规则（由 MatchRuleService 负责），不持有 GameController 引用。
class StackController
{
public:
    StackController() = default;

    // 注入依赖；不获取所有权。
    void init(GameModel* model, StackView* stackView, UndoManager* undoMgr);

    // 处理备用牌堆点击：将堆顶牌移至 tray，并记录撤销动作。
    bool handleStackClick(int cardId);

    // 由 GameController 在 UndoManager.undo() 时调用，回滚一次「备用牌->tray」。
    void undoStackToTray(const UndoModel::Action& a);

private:
    void promoteStackCardToTray(int cardId);

    GameModel*   _model     = nullptr;
    StackView*   _stackView = nullptr;
    UndoManager* _undoMgr   = nullptr;
};

#endif

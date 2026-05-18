#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#include <functional>
#include "cocos2d.h"
#include "views/PlayFieldView.h"
#include "views/StackView.h"

// GameView：游戏根视图，组合 PlayFieldView（上 1500）+ StackView（下 580）+ Undo 按钮。
// 它本身仅负责"摆放子视图、转发回调"，不持有 controller/model。
class GameView : public cocos2d::Node
{
public:
    using UndoCallback = std::function<void()>;

    static GameView* create();

    PlayFieldView* getPlayFieldView() const { return _playField; }
    StackView*     getStackView()     const { return _stackView; }

    void setOnUndoClick(UndoCallback cb) { _onUndoClick = std::move(cb); }

    // Undo 按钮可点状态（无可回退记录时禁用）。
    void setUndoEnabled(bool v);

protected:
    bool initView();
    void createUndoButton();

private:
    PlayFieldView* _playField = nullptr;
    StackView*     _stackView = nullptr;
    cocos2d::MenuItemLabel* _undoItem = nullptr;
    UndoCallback _onUndoClick;
};

#endif

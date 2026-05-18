#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include <functional>
#include "models/GameModel.h"
#include "models/UndoModel.h"

// UndoManager：撤销栈管理器。
// - 由 GameController 持有（成员变量，禁止单例）；
// - 持有 GameModel 的非拥有指针，操作其内嵌的 UndoModel；
// - 不直接接触 view，回滚效果通过回调 _undoExecutor 通知 controller，
//   再由 controller 触发对应 view 动画与 model 状态回滚。
//
// 扩展新撤销类型：
//   1) 在 UndoModel::ActionType 增加枚举；
//   2) 在 controller 中调用 recordXxx() 把动作压栈；
//   3) 让 _undoExecutor 的实现（GameController）能识别新 type 并执行回滚。
//   UndoManager 自身无需改动。
class UndoManager
{
public:
    // 回滚执行器：接收要回滚的 Action，由 controller 提供具体处理。
    using UndoExecutor = std::function<void(const UndoModel::Action&)>;
    // 栈变化通知：push / pop / clear 之后触发，便于 controller 刷新 UI（如撤销按钮可点态）。
    using ChangeListener = std::function<void()>;

    UndoManager() = default;

    void init(GameModel* gameModel, UndoExecutor executor)
    {
        _gameModel = gameModel;
        _undoExecutor = std::move(executor);
    }

    // 设置栈变化监听；不参与生命周期管理，调用方负责 listener 闭包捕获的对象在 manager 之前不被销毁。
    void setChangeListener(ChangeListener listener) { _changeListener = std::move(listener); }

    // 记录一次「桌面牌 -> 顶部底牌」操作。
    void recordPlayfieldToTray(int movedCardId, int previousTrayCardId, const cocos2d::Vec2& fromPos)
    {
        if (!_gameModel) return;
        UndoModel::Action a;
        a.type = UndoModel::AT_PLAYFIELD_TO_TRAY;
        a.movedCardId = movedCardId;
        a.previousTrayCardId = previousTrayCardId;
        a.fromPosition = fromPos;
        a.previousZone = CardModel::ZONE_PLAYFIELD;
        _gameModel->undoModel().push(a);
        _fireChanged();
    }

    // 记录一次「备用牌 -> 顶部底牌」操作。
    void recordStackToTray(int movedCardId, int previousTrayCardId, const cocos2d::Vec2& fromPos)
    {
        if (!_gameModel) return;
        UndoModel::Action a;
        a.type = UndoModel::AT_STACK_TO_TRAY;
        a.movedCardId = movedCardId;
        a.previousTrayCardId = previousTrayCardId;
        a.fromPosition = fromPos;
        a.previousZone = CardModel::ZONE_STACK;
        _gameModel->undoModel().push(a);
        _fireChanged();
    }

    bool canUndo() const
    {
        return _gameModel && !_gameModel->undoModel().empty();
    }

    // 弹出栈顶并交给 executor 执行回滚。
    void undo()
    {
        if (!canUndo() || !_undoExecutor) return;
        UndoModel::Action a = _gameModel->undoModel().pop();
        _undoExecutor(a);
        _fireChanged();
    }

    void clear()
    {
        if (_gameModel) _gameModel->undoModel().clear();
        _fireChanged();
    }

private:
    void _fireChanged() const
    {
        if (_changeListener) _changeListener();
    }

    GameModel*      _gameModel = nullptr;       // 不拥有
    UndoExecutor    _undoExecutor;
    ChangeListener  _changeListener;
};

#endif

#ifndef __UNDO_MODEL_H__
#define __UNDO_MODEL_H__

#include <vector>
#include "cocos2d.h"

// UndoModel: undo action stack.
// Each Action records the minimum snapshot needed to reverse one move.
// To add a new undo type: add an ActionType value and any extra fields to Action.
class UndoModel
{
public:
    enum ActionType
    {
        AT_NONE = 0,
        AT_PLAYFIELD_TO_TRAY,   // playfield card -> tray top
        AT_STACK_TO_TRAY,       // stack card -> tray top
    };

    struct Action
    {
        ActionType type = AT_NONE;
        int movedCardId = -1;
        int previousTrayCardId = -1;
        cocos2d::Vec2 fromPosition;
        int previousZone = 0;
    };

    void push(const Action& a) { _stack.push_back(a); }
    bool empty() const { return _stack.empty(); }
    size_t size() const { return _stack.size(); }

    Action pop()
    {
        Action a = _stack.back();
        _stack.pop_back();
        return a;
    }

    void clear() { _stack.clear(); }

    const std::vector<Action>& getActions() const { return _stack; }
    std::vector<Action>& mutableActions() { return _stack; }

private:
    std::vector<Action> _stack;
};

#endif

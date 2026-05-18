#ifndef __STACK_VIEW_H__
#define __STACK_VIEW_H__

#include <functional>
#include <unordered_map>
#include "cocos2d.h"
#include "views/CardView.h"

// StackView：屏幕下半部 1080 x 580，包含：
//   - 备用牌堆（左侧，玩家点击翻一张到 tray）；
//   - 手牌区/底牌堆 tray（右侧，显示顶部底牌）。
// 视图层只负责定位与显示，业务由 StackController/GameController 处理。
class StackView : public cocos2d::Node
{
public:
    using CardClickCallback = std::function<void(int /*cardId*/)>;

    static StackView* create(const cocos2d::Size& size);

    // 备用牌堆位置（设计分辨率坐标，相对于 StackView 父节点，即 GameView）。
    cocos2d::Vec2 getStackSlotPosition() const { return _stackSlot; }
    // 手牌区顶部牌目标位置。
    cocos2d::Vec2 getTraySlotPosition() const { return _traySlot; }

    void addStackCardView(CardView* card);
    void addTrayCardView(CardView* card);

    CardView* getCardView(int cardId) const;
    CardView* detachCardView(int cardId);

    void setOnStackClick(CardClickCallback cb) { _onStackClick = std::move(cb); }
    void setOnTrayClick(CardClickCallback cb)  { _onTrayClick = std::move(cb); }

protected:
    bool initWithSize(const cocos2d::Size& size);

private:
    cocos2d::Size _designSize;
    cocos2d::Vec2 _stackSlot;
    cocos2d::Vec2 _traySlot;
    std::unordered_map<int, CardView*> _cardViews;
    std::unordered_map<int, bool> _isStackCard;  // true: 备用堆 / false: tray

    CardClickCallback _onStackClick;
    CardClickCallback _onTrayClick;
};

#endif

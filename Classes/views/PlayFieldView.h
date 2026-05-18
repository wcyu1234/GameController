#ifndef __PLAY_FIELD_VIEW_H__
#define __PLAY_FIELD_VIEW_H__

#include <functional>
#include <unordered_map>
#include "cocos2d.h"
#include "views/CardView.h"

// PlayFieldView：主牌区视图（1080 x 1500，置于屏幕上方）。
// 负责：根据 model 的 playfield 列表生成 CardView、转发点击给 controller。
// 不维护任何业务规则。
class PlayFieldView : public cocos2d::Node
{
public:
    using CardClickCallback = std::function<void(int /*cardId*/)>;

    static PlayFieldView* create(const cocos2d::Size& size);

    void setOnCardClick(CardClickCallback cb) { _onCardClick = std::move(cb); }

    // 把一张卡的视图添加进来。
    void addCardView(CardView* cardView);

    // 通过 id 获取视图，找不到返回 nullptr。
    CardView* getCardView(int cardId) const;

    // 移除（并销毁）一个视图——例如卡牌平移到 tray 之后由 controller 负责调用，
    // 但在本作业里平移过去的卡视图会被 reparent 到 StackView，此函数仅保留扩展点。
    void removeCardView(int cardId);

    // 解除 view 的归属（但不销毁），用于把卡 reparent 到其他容器（比如 StackView）。
    CardView* detachCardView(int cardId);

protected:
    bool initWithSize(const cocos2d::Size& size);

private:
    cocos2d::Size _designSize;
    std::unordered_map<int, CardView*> _cardViews;  // weak refs
    CardClickCallback _onCardClick;
};

#endif

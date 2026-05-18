#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__

#include <functional>
#include "cocos2d.h"
#include "models/CardModel.h"

// CardView：一张卡牌的视图。
// 仅负责绘制 + 点击事件捕获，不持有业务逻辑；
// 与 controller 通过点击回调通信，禁止反向调用 controller 接口。
// 当前实现以矢量绘制（颜色块 + 文本）渲染——美术资源尚未提供时也能跑通；
// 接入真实卡面图后，把 createVisual() 改为加载 Sprite 即可，外部接口不变。
class CardView : public cocos2d::Node
{
public:
    using ClickCallback = std::function<void(int /*cardId*/)>;

    static CardView* create(const CardModel* model);

    int getCardId() const { return _cardId; }

    // 平移到目标位置（设计分辨率坐标），结束时回调；用于消除匹配 + 撤销回退。
    void moveTo(const cocos2d::Vec2& targetPos, float duration, std::function<void()> onDone);

    // 点击事件回调注册。
    void setOnClick(ClickCallback cb) { _onClick = std::move(cb); }

    // 开关交互（手牌区顶部牌、被覆盖牌等可被禁用）。
    void setInteractive(bool v) { _interactive = v; }
    bool isInteractive() const { return _interactive; }

protected:
    bool initWithModel(const CardModel* model);
    void createVisual(const CardModel* model);
    void setupTouchListener();

private:
    int _cardId = -1;
    bool _interactive = true;
    ClickCallback _onClick;
    cocos2d::EventListenerTouchOneByOne* _touchListener = nullptr;
};

#endif

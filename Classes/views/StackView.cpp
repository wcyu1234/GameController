#include "views/StackView.h"

USING_NS_CC;

StackView* StackView::create(const Size& size)
{
    auto* v = new (std::nothrow) StackView();
    if (v && v->initWithSize(size))
    {
        v->autorelease();
        return v;
    }
    delete v;
    return nullptr;
}

bool StackView::initWithSize(const Size& size)
{
    if (!Node::init()) return false;
    _designSize = size;
    setContentSize(size);
    setAnchorPoint(Vec2::ZERO);

    auto bg = LayerColor::create(Color4B(30, 80, 45, 255), size.width, size.height);
    addChild(bg, -1);

    // 设计分辨率 1080 x 580：备用牌堆放左侧 1/4，tray 放右侧 3/4。
    _stackSlot = Vec2(size.width * 0.25f, size.height * 0.5f);
    _traySlot  = Vec2(size.width * 0.65f, size.height * 0.5f);

    // 卡牌槽位占位描边（即使备用堆抽空 / tray 暂未放牌也能看见位置）。
    auto drawSlot = [this](const Vec2& center) {
        auto node = DrawNode::create();
        const float w = 75.f;   // CardView 半宽 + 留白
        const float h = 105.f;
        Color4F line(1.f, 1.f, 1.f, 0.18f);
        node->drawRect(center + Vec2(-w, -h), center + Vec2(w, h), line);
        addChild(node, -1);
    };
    drawSlot(_stackSlot);
    drawSlot(_traySlot);

    return true;
}

void StackView::addStackCardView(CardView* card)
{
    if (!card) return;
    int id = card->getCardId();
    _cardViews[id] = card;
    _isStackCard[id] = true;
    card->setOnClick([this](int cardId) {
        if (_onStackClick) _onStackClick(cardId);
    });
    addChild(card);
}

void StackView::addTrayCardView(CardView* card)
{
    if (!card) return;
    int id = card->getCardId();
    _cardViews[id] = card;
    _isStackCard[id] = false;
    card->setOnClick([this](int cardId) {
        if (_onTrayClick) _onTrayClick(cardId);
    });
    addChild(card);
}

CardView* StackView::getCardView(int cardId) const
{
    auto it = _cardViews.find(cardId);
    return it == _cardViews.end() ? nullptr : it->second;
}

CardView* StackView::detachCardView(int cardId)
{
    auto it = _cardViews.find(cardId);
    if (it == _cardViews.end()) return nullptr;
    CardView* v = it->second;
    _cardViews.erase(it);
    _isStackCard.erase(cardId);
    v->retain();
    v->removeFromParent();
    v->autorelease();
    return v;
}

#include "views/PlayFieldView.h"

USING_NS_CC;

PlayFieldView* PlayFieldView::create(const Size& size)
{
    auto* v = new (std::nothrow) PlayFieldView();
    if (v && v->initWithSize(size))
    {
        v->autorelease();
        return v;
    }
    delete v;
    return nullptr;
}

bool PlayFieldView::initWithSize(const Size& size)
{
    if (!Node::init()) return false;
    _designSize = size;
    setContentSize(size);
    setAnchorPoint(Vec2::ZERO);

    // 一层浅灰背板，方便和堆牌区区分。
    auto bg = LayerColor::create(Color4B(40, 110, 60, 255), size.width, size.height);
    addChild(bg, -1);
    return true;
}

void PlayFieldView::addCardView(CardView* cardView)
{
    if (!cardView) return;
    int id = cardView->getCardId();
    _cardViews[id] = cardView;
    cardView->setOnClick([this](int cardId) {
        if (_onCardClick) _onCardClick(cardId);
    });
    addChild(cardView);
}

CardView* PlayFieldView::getCardView(int cardId) const
{
    auto it = _cardViews.find(cardId);
    return it == _cardViews.end() ? nullptr : it->second;
}

void PlayFieldView::removeCardView(int cardId)
{
    auto it = _cardViews.find(cardId);
    if (it == _cardViews.end()) return;
    it->second->removeFromParent();
    _cardViews.erase(it);
}

CardView* PlayFieldView::detachCardView(int cardId)
{
    auto it = _cardViews.find(cardId);
    if (it == _cardViews.end()) return nullptr;
    CardView* v = it->second;
    _cardViews.erase(it);
    v->retain();
    v->removeFromParent();
    v->autorelease();
    return v;
}

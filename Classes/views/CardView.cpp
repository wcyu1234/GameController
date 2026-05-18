#include "views/CardView.h"

#include "configs/models/CardResConfig.h"

USING_NS_CC;

namespace
{
    constexpr float kCardWidth  = 150.f;
    constexpr float kCardHeight = 210.f;
}

CardView* CardView::create(const CardModel* model)
{
    auto* v = new (std::nothrow) CardView();
    if (v && v->initWithModel(model))
    {
        v->autorelease();
        return v;
    }
    delete v;
    return nullptr;
}

bool CardView::initWithModel(const CardModel* model)
{
    if (!Node::init()) return false;
    if (!model) return false;

    _cardId = model->getId();
    setContentSize(Size(kCardWidth, kCardHeight));
    // 注意：anchor 保持 (0,0)（Node 默认值），让子节点直接以 local (0,0) = 视觉中心
    // 排版（用 ±w/2、±h/2）。改成 (0.5,0.5) 会让 setPosition() 把 (w/2,h/2) 视为 pivot，
    // 与子节点的 ±w/2 排版坐标系冲突，导致整张牌视觉偏左下 (w/2, h/2)。
    setPosition(model->getPosition());

    createVisual(model);
    setupTouchListener();
    return true;
}

void CardView::createVisual(const CardModel* model)
{
    // 卡背：白底圆角感（用两层 LayerColor 模拟内描边）。
    auto bg = LayerColor::create(Color4B(248, 248, 248, 255), kCardWidth, kCardHeight);
    bg->setPosition(Vec2(-kCardWidth * 0.5f, -kCardHeight * 0.5f));
    addChild(bg);

    auto inner = LayerColor::create(Color4B::WHITE, kCardWidth - 8.f, kCardHeight - 8.f);
    inner->setPosition(Vec2(-kCardWidth * 0.5f + 4.f, -kCardHeight * 0.5f + 4.f));
    addChild(inner);

    // 外边框（细黑线）。
    auto border = DrawNode::create();
    Vec2 verts[4] = {
        Vec2(-kCardWidth * 0.5f, -kCardHeight * 0.5f),
        Vec2( kCardWidth * 0.5f, -kCardHeight * 0.5f),
        Vec2( kCardWidth * 0.5f,  kCardHeight * 0.5f),
        Vec2(-kCardWidth * 0.5f,  kCardHeight * 0.5f),
    };
    border->drawPoly(verts, 4, true, Color4F(0.15f, 0.15f, 0.15f, 1.f));
    addChild(border);

    Color3B color = CardResConfig::isRedSuit(model->getSuit())
                    ? Color3B(200, 30, 30) : Color3B(20, 20, 20);
    Color4B color4 = Color4B(color);

    const std::string faceText = CardResConfig::getFaceText(model->getFace());
    const std::string suitSym  = CardResConfig::getSuitSymbol(model->getSuit());

    // 中央大花色符号。
    auto centerSuit = Label::createWithSystemFont(suitSym, "Arial", 96);
    centerSuit->setTextColor(color4);
    centerSuit->setPosition(Vec2(0, -8));
    addChild(centerSuit);

    // 左上角点数 + 小花色。
    auto cornerFaceTL = Label::createWithSystemFont(faceText, "Arial", 36);
    cornerFaceTL->setTextColor(color4);
    cornerFaceTL->setAnchorPoint(Vec2(0, 1));
    cornerFaceTL->setPosition(Vec2(-kCardWidth * 0.5f + 12, kCardHeight * 0.5f - 10));
    addChild(cornerFaceTL);

    auto cornerSuitTL = Label::createWithSystemFont(suitSym, "Arial", 28);
    cornerSuitTL->setTextColor(color4);
    cornerSuitTL->setAnchorPoint(Vec2(0, 1));
    cornerSuitTL->setPosition(Vec2(-kCardWidth * 0.5f + 14, kCardHeight * 0.5f - 50));
    addChild(cornerSuitTL);

    // 右下角点数 + 小花色（旋转 180° 模拟扑克对称风）。
    auto cornerFaceBR = Label::createWithSystemFont(faceText, "Arial", 36);
    cornerFaceBR->setTextColor(color4);
    cornerFaceBR->setAnchorPoint(Vec2(0, 1));
    cornerFaceBR->setRotation(180.f);
    cornerFaceBR->setPosition(Vec2(kCardWidth * 0.5f - 12, -kCardHeight * 0.5f + 10));
    addChild(cornerFaceBR);

    auto cornerSuitBR = Label::createWithSystemFont(suitSym, "Arial", 28);
    cornerSuitBR->setTextColor(color4);
    cornerSuitBR->setAnchorPoint(Vec2(0, 1));
    cornerSuitBR->setRotation(180.f);
    cornerSuitBR->setPosition(Vec2(kCardWidth * 0.5f - 14, -kCardHeight * 0.5f + 50));
    addChild(cornerSuitBR);
}

void CardView::setupTouchListener()
{
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);

    _touchListener->onTouchBegan = [this](Touch* touch, Event*) -> bool {
        if (!_interactive || !isVisible()) return false;
        Vec2 local = convertToNodeSpace(touch->getLocation());
        Rect bb(-kCardWidth * 0.5f, -kCardHeight * 0.5f, kCardWidth, kCardHeight);
        return bb.containsPoint(local);
    };
    _touchListener->onTouchEnded = [this](Touch* touch, Event*) {
        if (!_interactive) return;
        Vec2 local = convertToNodeSpace(touch->getLocation());
        Rect bb(-kCardWidth * 0.5f, -kCardHeight * 0.5f, kCardWidth, kCardHeight);
        if (bb.containsPoint(local) && _onClick)
        {
            _onClick(_cardId);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardView::moveTo(const Vec2& targetPos, float duration, std::function<void()> onDone)
{
    auto move = MoveTo::create(duration, targetPos);
    if (onDone)
    {
        auto seq = Sequence::create(move, CallFunc::create(onDone), nullptr);
        runAction(seq);
    }
    else
    {
        runAction(move);
    }
}

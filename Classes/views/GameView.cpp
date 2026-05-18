#include "views/GameView.h"

USING_NS_CC;

namespace
{
    constexpr float kDesignW = 1080.f;
    constexpr float kDesignH = 2080.f;
    constexpr float kPlayFieldH = 1500.f;
    constexpr float kStackH     = 580.f;
}

GameView* GameView::create()
{
    auto* v = new (std::nothrow) GameView();
    if (v && v->initView())
    {
        v->autorelease();
        return v;
    }
    delete v;
    return nullptr;
}

bool GameView::initView()
{
    if (!Node::init()) return false;
    setContentSize(Size(kDesignW, kDesignH));
    setAnchorPoint(Vec2::ZERO);

    _stackView = StackView::create(Size(kDesignW, kStackH));
    _stackView->setPosition(Vec2(0, 0));
    addChild(_stackView);

    _playField = PlayFieldView::create(Size(kDesignW, kPlayFieldH));
    _playField->setPosition(Vec2(0, kStackH));
    addChild(_playField);

    createUndoButton();
    return true;
}

void GameView::createUndoButton()
{
    // 用一个带圆角背景的胶囊状按钮，比纯文本更醒目；放在 StackView 顶部偏右。
    const float btnW = 220.f;
    const float btnH = 90.f;

    auto label = Label::createWithSystemFont("Undo", "Arial", 56);
    label->setTextColor(Color4B::WHITE);
    _undoItem = MenuItemLabel::create(label, [this](Ref*) {
        if (_onUndoClick) _onUndoClick();
    });
    _undoItem->setContentSize(Size(btnW, btnH));

    // 背景框（橙红色），DrawNode 画在按钮 menu 之下；置于 StackView 上方约 60px。
    auto menu = Menu::create(_undoItem, nullptr);
    menu->setPosition(Vec2(kDesignW - btnW * 0.5f - 40.f, kStackH + 60.f));

    auto bgNode = DrawNode::create();
    bgNode->drawSolidRect(Vec2(-btnW * 0.5f, -btnH * 0.5f),
                          Vec2( btnW * 0.5f,  btnH * 0.5f),
                          Color4F(0.85f, 0.35f, 0.20f, 1.f));
    bgNode->drawRect(Vec2(-btnW * 0.5f, -btnH * 0.5f),
                     Vec2( btnW * 0.5f,  btnH * 0.5f),
                     Color4F::WHITE);
    bgNode->setPosition(menu->getPosition());
    addChild(bgNode);
    addChild(menu);
}

void GameView::setUndoEnabled(bool v)
{
    if (_undoItem)
    {
        _undoItem->setEnabled(v);
        _undoItem->setOpacity(v ? 255 : 120);
    }
}

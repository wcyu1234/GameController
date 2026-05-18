#include "controllers/StackController.h"

#include "managers/UndoManager.h"

USING_NS_CC;

namespace
{
    constexpr float kMoveDuration = 0.3f;
}

void StackController::init(GameModel* model, StackView* stackView, UndoManager* undoMgr)
{
    _model     = model;
    _stackView = stackView;
    _undoMgr   = undoMgr;

    if (_stackView)
    {
        _stackView->setOnStackClick([this](int cardId) {
            handleStackClick(cardId);
        });
    }
}

bool StackController::handleStackClick(int cardId)
{
    if (!_model) return false;
    CardModel* card = _model->getCard(cardId);
    if (!card || card->getZone() != CardModel::ZONE_STACK) return false;

    // 只允许点击备用堆顶牌。
    auto& stackIds = _model->stackIds();
    if (stackIds.empty() || stackIds.back() != cardId) return false;

    promoteStackCardToTray(cardId);
    return true;
}

void StackController::promoteStackCardToTray(int cardId)
{
    CardModel* card = _model->getCard(cardId);
    if (!card) return;

    // 1) 记录撤销（先记录，再改 model）。
    int prevTopId = _model->trayIds().empty() ? -1 : _model->trayIds().back();
    Vec2 fromPos  = card->getPosition();
    _undoMgr->recordStackToTray(cardId, prevTopId, fromPos);

    // 2) Model 更新：从 stackIds 移除，压入 trayIds 顶。
    auto& st = _model->stackIds();
    st.erase(std::remove(st.begin(), st.end(), cardId), st.end());
    _model->trayIds().push_back(cardId);
    card->setZone(CardModel::ZONE_TRAY);

    // 3) View：在 StackView 内直接平移到 tray 槽位。
    CardView* view = _stackView->getCardView(cardId);
    if (!view) return;

    // 更新归属标记（从 stack 侧变成 tray 侧）：先 detach 再作为 tray 卡重新注册。
    _stackView->detachCardView(cardId);  // 从 _cardViews 移除
    _stackView->addTrayCardView(view);   // 重新注册为 tray 卡，设置新回调

    view->setOnClick(nullptr);
    view->setInteractive(false);
    view->setLocalZOrder(static_cast<int>(_model->trayIds().size()));

    Vec2 traySlot = _stackView->getTraySlotPosition();
    view->moveTo(traySlot, kMoveDuration, nullptr);

    // 4) 同步 model 坐标。
    card->setPosition(traySlot);
}

void StackController::undoStackToTray(const UndoModel::Action& a)
{
    if (!_model) return;
    CardModel* card = _model->getCard(a.movedCardId);
    if (!card) return;

    // Model 回滚：trayIds 弹顶，归还到 stackIds 末尾。
    auto& tray = _model->trayIds();
    if (!tray.empty() && tray.back() == a.movedCardId)
        tray.pop_back();

    _model->stackIds().push_back(a.movedCardId);
    card->setZone(CardModel::ZONE_STACK);
    card->setPosition(a.fromPosition);

    // View 回滚：重新注册为 stack 卡，反向平移。
    CardView* view = _stackView->detachCardView(a.movedCardId);
    if (!view) return;

    _stackView->addStackCardView(view);  // 重新挂回备用堆，恢复点击回调
    view->setInteractive(true);
    view->setLocalZOrder(static_cast<int>(_model->stackIds().size()));
    view->moveTo(a.fromPosition, kMoveDuration, nullptr);
}

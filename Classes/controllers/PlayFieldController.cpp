#include "controllers/PlayFieldController.h"

#include "managers/UndoManager.h"
#include "services/MatchRuleService.h"

USING_NS_CC;

namespace
{
    constexpr float kMoveDuration = 0.3f;
}

void PlayFieldController::init(GameModel* model, PlayFieldView* pfv, StackView* sv, UndoManager* undoMgr)
{
    _model = model;
    _playFieldView = pfv;
    _stackView = sv;
    _undoMgr = undoMgr;

    if (_playFieldView)
    {
        _playFieldView->setOnCardClick([this](int cardId) {
            handleCardClick(cardId);
        });
    }
}

bool PlayFieldController::handleCardClick(int cardId)
{
    if (!_model) return false;
    CardModel* card = _model->getCard(cardId);
    if (!card || card->getZone() != CardModel::ZONE_PLAYFIELD) return false;

    CardModel* top = _model->topTrayCard();
    if (!top) return false;
    if (!MatchRuleService::canMatch(card->getFace(), top->getFace())) return false;

    replaceTrayWithPlayFieldCard(cardId);
    return true;
}

void PlayFieldController::replaceTrayWithPlayFieldCard(int cardId)
{
    CardModel* card = _model->getCard(cardId);
    if (!card) return;

    // 1) 撤销栈记录（先记录，再改 model，避免遗失上一次 tray 顶部 id）。
    int prevTopId = _model->trayIds().empty() ? -1 : _model->trayIds().back();
    Vec2 fromPos = card->getPosition();
    _undoMgr->recordPlayfieldToTray(cardId, prevTopId, fromPos);

    // 2) Model 更新：从 playfield 列表移除，压入 tray 顶。
    auto& pf = _model->playfieldIds();
    pf.erase(std::remove(pf.begin(), pf.end(), cardId), pf.end());
    _model->trayIds().push_back(cardId);
    card->setZone(CardModel::ZONE_TRAY);

    // 3) View：跨父节点迁移，先记录世界坐标，再 detach，再以等价的本地坐标挂到 StackView 上。
    CardView* view = _playFieldView->getCardView(cardId);
    if (!view) return;
    Vec2 worldPos = _playFieldView->convertToWorldSpace(view->getPosition());
    _playFieldView->detachCardView(cardId);
    Vec2 localInStack = _stackView->convertToNodeSpace(worldPos);
    view->setPosition(localInStack);
    _stackView->addTrayCardView(view);

    Vec2 traySlot = _stackView->getTraySlotPosition();
    view->setInteractive(false);
    view->setLocalZOrder(static_cast<int>(_model->trayIds().size())); // 保证压在旧底牌之上
    view->moveTo(traySlot, kMoveDuration, nullptr);

    // 4) Model 同步坐标（便于后续 undo 直接读 fromPos）。
    card->setPosition(traySlot);
}

void PlayFieldController::undoPlayfieldToTray(const UndoModel::Action& a)
{
    if (!_model) return;
    CardModel* card = _model->getCard(a.movedCardId);
    if (!card) return;

    // Model 回滚：trayIds 弹出顶（应等于 movedCardId），重新加入 playfieldIds。
    auto& tray = _model->trayIds();
    if (!tray.empty() && tray.back() == a.movedCardId)
        tray.pop_back();

    _model->playfieldIds().push_back(a.movedCardId);
    card->setZone(CardModel::ZONE_PLAYFIELD);
    card->setPosition(a.fromPosition);

    // View 回滚：跨父节点迁回 PlayFieldView，反向 MoveTo 原坐标，恢复点击响应。
    CardView* view = _stackView->getCardView(a.movedCardId);
    if (!view) return;
    Vec2 worldPos = _stackView->convertToWorldSpace(view->getPosition());
    _stackView->detachCardView(a.movedCardId);
    Vec2 localInPf = _playFieldView->convertToNodeSpace(worldPos);
    view->setPosition(localInPf);
    _playFieldView->addCardView(view);
    view->setInteractive(true);
    view->setLocalZOrder(0);
    view->moveTo(a.fromPosition, kMoveDuration, nullptr);
}

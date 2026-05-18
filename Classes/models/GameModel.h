#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include <vector>
#include <memory>
#include <unordered_map>
#include "models/CardModel.h"
#include "models/UndoModel.h"

// GameModel: full runtime state for one game session.
// - Owns all CardModels (by id).
// - Maintains three zone id-lists: playfield / stack / tray.
//   The top tray card is the last element of trayIds.
// - Embeds UndoModel (undo stack).
// Business rules (matching, generation) live in service/controller layers.
class GameModel
{
public:
    GameModel() = default;

    // Add a card (takes ownership); returns raw pointer for caller reference.
    CardModel* addCard(std::unique_ptr<CardModel> card)
    {
        CardModel* raw = card.get();
        _cardsById[raw->getId()] = std::move(card);
        return raw;
    }

    CardModel* getCard(int id) const
    {
        auto it = _cardsById.find(id);
        return it == _cardsById.end() ? nullptr : it->second.get();
    }

    std::vector<int>& playfieldIds() { return _playfieldIds; }
    std::vector<int>& stackIds()     { return _stackIds; }
    std::vector<int>& trayIds()      { return _trayIds; }

    const std::vector<int>& playfieldIds() const { return _playfieldIds; }
    const std::vector<int>& stackIds()     const { return _stackIds; }
    const std::vector<int>& trayIds()      const { return _trayIds; }

    // Top tray card (base card); nullptr if tray is empty.
    CardModel* topTrayCard() const
    {
        if (_trayIds.empty()) return nullptr;
        return getCard(_trayIds.back());
    }

    UndoModel& undoModel() { return _undoModel; }
    const UndoModel& undoModel() const { return _undoModel; }

private:
    std::unordered_map<int, std::unique_ptr<CardModel>> _cardsById;
    std::vector<int> _playfieldIds;
    std::vector<int> _stackIds;
    std::vector<int> _trayIds;
    UndoModel _undoModel;
};

#endif

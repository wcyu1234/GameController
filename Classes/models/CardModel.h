#ifndef __CARD_MODEL_H__
#define __CARD_MODEL_H__

#include "cocos2d.h"
#include "configs/models/CardTypes.h"

// CardModel: runtime data for a single card.
// Holds a stable id (used for cross-layer references), face, suit,
// world position (in design-resolution coordinates), and current zone.
class CardModel
{
public:
    enum Zone
    {
        ZONE_NONE = 0,
        ZONE_PLAYFIELD,   // main play area (table cards)
        ZONE_STACK,       // draw stack
        ZONE_TRAY,        // top card of the tray (base card)
    };

    CardModel() = default;
    CardModel(int id, CardFaceType face, CardSuitType suit)
        : _id(id), _face(face), _suit(suit) {}

    int getId() const { return _id; }
    CardFaceType getFace() const { return _face; }
    CardSuitType getSuit() const { return _suit; }
    const cocos2d::Vec2& getPosition() const { return _position; }
    Zone getZone() const { return _zone; }

    void setId(int id) { _id = id; }
    void setFace(CardFaceType face) { _face = face; }
    void setSuit(CardSuitType suit) { _suit = suit; }
    void setPosition(const cocos2d::Vec2& p) { _position = p; }
    void setZone(Zone z) { _zone = z; }

private:
    int _id = -1;
    CardFaceType _face = CFT_NONE;
    CardSuitType _suit = CST_NONE;
    cocos2d::Vec2 _position;
    Zone _zone = ZONE_NONE;
};

#endif

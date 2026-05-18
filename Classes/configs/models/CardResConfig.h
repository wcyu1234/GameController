#ifndef __CARD_RES_CONFIG_H__
#define __CARD_RES_CONFIG_H__

#include <string>
#include "configs/models/CardTypes.h"

// CardResConfig: maps card enums to display text and color helpers.
// When real art assets arrive, update getFaceImage()/getSuitImage() here;
// view layer stays untouched.
class CardResConfig
{
public:
    static std::string getFaceText(CardFaceType face)
    {
        switch (face)
        {
            case CFT_ACE:   return "A";
            case CFT_TWO:   return "2";
            case CFT_THREE: return "3";
            case CFT_FOUR:  return "4";
            case CFT_FIVE:  return "5";
            case CFT_SIX:   return "6";
            case CFT_SEVEN: return "7";
            case CFT_EIGHT: return "8";
            case CFT_NINE:  return "9";
            case CFT_TEN:   return "10";
            case CFT_JACK:  return "J";
            case CFT_QUEEN: return "Q";
            case CFT_KING:  return "K";
            default:        return "?";
        }
    }

    static std::string getSuitText(CardSuitType suit)
    {
        switch (suit)
        {
            case CST_CLUBS:    return "C";
            case CST_DIAMONDS: return "D";
            case CST_HEARTS:   return "H";
            case CST_SPADES:   return "S";
            default:           return "?";
        }
    }

    // 返回 UTF-8 编码的花色符号 (♣ ♦ ♥ ♠)。
    // 用 \xHH 形式直接写字节，避免源码编码 / 编译器解释的差异。
    static std::string getSuitSymbol(CardSuitType suit)
    {
        switch (suit)
        {
            case CST_CLUBS:    return "\xE2\x99\xA3"; // U+2663 ♣
            case CST_DIAMONDS: return "\xE2\x99\xA6"; // U+2666 ♦
            case CST_HEARTS:   return "\xE2\x99\xA5"; // U+2665 ♥
            case CST_SPADES:   return "\xE2\x99\xA0"; // U+2660 ♠
            default:           return "?";
        }
    }

    static bool isRedSuit(CardSuitType suit)
    {
        return suit == CST_DIAMONDS || suit == CST_HEARTS;
    }
};

#endif

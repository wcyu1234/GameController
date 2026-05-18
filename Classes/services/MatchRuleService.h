#ifndef __MATCH_RULE_SERVICE_H__
#define __MATCH_RULE_SERVICE_H__

#include "configs/models/CardTypes.h"

// MatchRuleService：纯规则判定，无状态。
// 任何"两张牌能否消除/匹配"的判定走这里，方便后续扩展规则（比如同花、连号等）。
class MatchRuleService
{
public:
    // 默认规则：点数相差恰好 1，无花色限制。
    static bool canMatch(CardFaceType a, CardFaceType b)
    {
        if (a == CFT_NONE || b == CFT_NONE) return false;
        int diff = static_cast<int>(a) - static_cast<int>(b);
        return diff == 1 || diff == -1;
    }
};

#endif

#include "services/GameModelFromLevelGenerator.h"

std::unique_ptr<GameModel> GameModelFromLevelGenerator::generate(const LevelConfig& config)
{
    auto model = std::unique_ptr<GameModel>(new GameModel());
    int nextId = 0;

    for (const auto& c : config.getPlayfield())
    {
        auto card = std::unique_ptr<CardModel>(new CardModel(nextId, c.face, c.suit));
        card->setPosition(c.position);
        card->setZone(CardModel::ZONE_PLAYFIELD);
        model->addCard(std::move(card));
        model->playfieldIds().push_back(nextId);
        ++nextId;
    }

    // Stack 约定（按当前关卡需求）：
    //   - 最后一张 = 初始底牌（直接进 tray）
    //   - 其余按顺序进备用堆 stackIds；stackIds 的"末尾"为可点击的栈顶
    // 这样关卡 JSON 中 Stack 列表的尾部直观对应"屏幕上看见的底牌"，前段是抽牌堆从底到顶的顺序。
    const auto& stackCfg = config.getStack();
    if (!stackCfg.empty())
    {
        // 备用堆：前 N-1 张，按出现顺序压入；最后一个 push_back 的就是顶部可点。
        for (size_t i = 0; i + 1 < stackCfg.size(); ++i)
        {
            const auto& c = stackCfg[i];
            auto card = std::unique_ptr<CardModel>(new CardModel(nextId, c.face, c.suit));
            card->setPosition(c.position);
            card->setZone(CardModel::ZONE_STACK);
            model->addCard(std::move(card));
            model->stackIds().push_back(nextId);
            ++nextId;
        }

        // 初始底牌（tray）：Stack 列表的最后一张。
        const auto& tc = stackCfg.back();
        auto trayCard = std::unique_ptr<CardModel>(new CardModel(nextId, tc.face, tc.suit));
        trayCard->setPosition(tc.position);
        trayCard->setZone(CardModel::ZONE_TRAY);
        model->addCard(std::move(trayCard));
        model->trayIds().push_back(nextId);
        ++nextId;
    }

    return model;
}

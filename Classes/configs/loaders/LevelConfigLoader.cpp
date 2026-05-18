#include "configs/loaders/LevelConfigLoader.h"

#include "cocos2d.h"
#include "json/document.h"

USING_NS_CC;

std::shared_ptr<LevelConfig> LevelConfigLoader::loadLevelConfig(int levelId)
{
    char path[128];
    snprintf(path, sizeof(path), "levels/level_%d.json", levelId);
    return loadFromFile(path);
}

std::shared_ptr<LevelConfig> LevelConfigLoader::loadFromFile(const std::string& fullPath)
{
    auto fu = FileUtils::getInstance();
    if (!fu->isFileExist(fullPath))
    {
        CCLOGERROR("[LevelConfigLoader] file not found: %s", fullPath.c_str());
        return nullptr;
    }
    std::string content = fu->getStringFromFile(fullPath);
    return parseJson(content);
}

std::shared_ptr<LevelConfig> LevelConfigLoader::parseJson(const std::string& jsonStr)
{
    rapidjson::Document doc;
    doc.Parse<0>(jsonStr.c_str());
    if (doc.HasParseError() || !doc.IsObject())
    {
        CCLOGERROR("[LevelConfigLoader] parse error");
        return nullptr;
    }

    auto cfg = std::make_shared<LevelConfig>();

    auto readArray = [](const rapidjson::Value& arr, std::vector<LevelCardConfig>& out)
    {
        if (!arr.IsArray()) return;
        out.reserve(arr.Size());
        for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
        {
            const auto& it = arr[i];
            if (!it.IsObject()) continue;
            LevelCardConfig c;
            if (it.HasMember("CardFace") && it["CardFace"].IsInt())
                c.face = static_cast<CardFaceType>(it["CardFace"].GetInt());
            if (it.HasMember("CardSuit") && it["CardSuit"].IsInt())
                c.suit = static_cast<CardSuitType>(it["CardSuit"].GetInt());
            if (it.HasMember("Position") && it["Position"].IsObject())
            {
                const auto& p = it["Position"];
                float x = (p.HasMember("x") && p["x"].IsNumber()) ? p["x"].GetFloat() : 0.f;
                float y = (p.HasMember("y") && p["y"].IsNumber()) ? p["y"].GetFloat() : 0.f;
                c.position = cocos2d::Vec2(x, y);
            }
            out.push_back(c);
        }
    };

    if (doc.HasMember("Playfield"))
        readArray(doc["Playfield"], cfg->mutablePlayfield());
    if (doc.HasMember("Stack"))
        readArray(doc["Stack"], cfg->mutableStack());

    return cfg;
}

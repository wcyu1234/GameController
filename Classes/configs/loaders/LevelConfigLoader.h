#ifndef __LEVEL_CONFIG_LOADER_H__
#define __LEVEL_CONFIG_LOADER_H__

#include <string>
#include <memory>
#include "configs/models/LevelConfig.h"

// 关卡配置加载器：从 Resources 目录中按关卡 ID 读取 JSON 并解析成 LevelConfig。
// 静态方法，无状态。失败时返回 nullptr，调用方负责日志/兜底。
class LevelConfigLoader
{
public:
    // 按关卡 ID 加载（约定路径：levels/level_<id>.json）。
    static std::shared_ptr<LevelConfig> loadLevelConfig(int levelId);

    // 按完整文件路径加载（便于测试或自定义路径）。
    static std::shared_ptr<LevelConfig> loadFromFile(const std::string& fullPath);

private:
    static std::shared_ptr<LevelConfig> parseJson(const std::string& jsonStr);
};

#endif

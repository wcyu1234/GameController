# Cardgame 程序设计文档

> 配合本仓库源码阅读。覆盖：MVC 分层、目录约定、关键数据流、以及"未来怎么扩展"两个典型场景的步骤清单。

## 1. 设计目标

* 满足三条主玩法需求：
  1. **手牌区翻牌**：点备用堆顶牌 → 移到 tray 顶部，替换原顶牌；
  2. **桌面牌匹配**：点桌面牌，与 tray 顶牌点数差 1 即可消除（无花色限制），平移到 tray 成为新顶牌；
  3. **回退**：依次反向播放最近若干次操作的平移动画，直到撤销栈清空。
* 严格遵循 MVC + Services + Managers 分层（详见 `CLAUDE.md` 与本文 §3）。
* 让"新增一种卡牌 / 新增一种回退类型"等扩展，只改少量类、且改动点是可预测的（详见 §6）。

## 2. 目录速览（与代码 1:1）

```
Classes/
├── configs/                       静态、只读
│   ├── models/CardTypes.h         共享枚举：CardFaceType / CardSuitType
│   ├── models/LevelConfig.h       关卡 JSON 解析后的纯数据
│   ├── models/CardResConfig.h     枚举 → 显示文本 / 颜色（接入美术资源时只改这里）
│   └── loaders/LevelConfigLoader  FileUtils + rapidjson → LevelConfig
│
├── models/                        运行时可变状态，不放业务逻辑
│   ├── CardModel                  id / face / suit / pos / zone
│   ├── GameModel                  拥有全部 CardModel，维护 playfieldIds / stackIds / trayIds，内嵌 UndoModel
│   └── UndoModel                  Action 栈（type + movedCardId + previousTrayCardId + fromPosition + previousZone）
│
├── services/                      无状态、可静态/单例
│   ├── GameModelFromLevelGenerator  LevelConfig → GameModel
│   └── MatchRuleService             canMatch(a,b) = |a−b|==1
│
├── managers/                      controller 的成员变量，可持 model，禁单例
│   └── UndoManager                  push/pop 撤销栈；通过 UndoExecutor 回调把回滚交给 controller
│
├── controllers/                   协调 model ↔ view，处理输入
│   ├── GameController               顶层；装配子控制器与 manager；绑定 Undo 按钮
│   ├── PlayFieldController          桌面牌点击 → 匹配 → 入 tray + 记录 undo；以及对应回滚
│   └── StackController              备用堆点击 → 升级到 tray + 记录 undo；以及对应回滚
│
└── views/                         纯展示 + 输入捕获，禁业务
    ├── CardView                     150×210 矢量绘制卡面；touch listener 触发 onClick 回调
    ├── PlayFieldView                上方 1080×1500；持子 CardView map，转发点击
    ├── StackView                    下方 1080×580；分备用堆 / tray 两个槽位
    └── GameView                     根节点，组合 PlayFieldView + StackView + Undo 按钮
```

外层入口：`GameScene` 作为 cocos2d Scene 的容器，仅持一个 `GameController`，由 `GameController::startGame(levelId, scene)` 完成所有装配。

## 3. 分层职责约束

| 层 | 可否持 model 数据 | 可否依赖 controller | 单例 |
| --- | --- | --- | --- |
| configs | 是（静态） | 否 | — |
| models | 是 | 否 | 否 |
| views | 仅 const 指针 | 仅 const 指针 / 回调 | 否 |
| controllers | 是 | 是（顶层 ↔ 子层） | 否 |
| managers | 是 | **否**（仅通过 callback） | **禁** |
| services | **否** | 否 | 可静态 / 可单例 |
| utils | 否 | 否 | 可 |

> View 跨父节点的位置保持由 controller 调度（`PlayFieldController.cpp::replaceTrayWithPlayFieldCard`）：先记录 worldPos → detach（解除注册）→ 在新父节点上换算 local → addXxx 重新注册并设置回调与 zOrder → MoveTo。

## 4. 关键数据流

### 4.1 启动流程

```
AppDelegate::applicationDidFinishLaunching
        ↓
GameScene::create(levelId)
        ↓
GameController::startGame(levelId, scene)
        ├─ LevelConfigLoader::loadLevelConfig(levelId)        — 读 JSON
        ├─ GameModelFromLevelGenerator::generate(cfg)         — JSON → GameModel
        │      约定：Stack 列表最后一张 → 初始 tray；其余进 stackIds，末尾为可点击的栈顶
        ├─ GameView::create() → scene->addChild
        ├─ UndoManager::init(model, undoExecutor)              — 注入回滚执行器
        ├─ PlayFieldController::init(model, pfv, sv, undoMgr)  — 注入依赖 + 注册点击
        ├─ StackController::init(model, sv, undoMgr)
        ├─ 为 playfield / stack / tray 中每张牌创建 CardView 并加进对应 View
        └─ 接 Undo 按钮 onClick → undoMgr.undo() + 刷新按钮可用态
```

### 4.2 桌面牌点击（需求 2）

```
CardView::onTouchEnded → _onClick(cardId)
        ↓
PlayFieldView::_onCardClick(cardId)                     // setOnCardClick 注册的回调
        ↓
PlayFieldController::handleCardClick(cardId)
   ├─ MatchRuleService::canMatch(face, trayTopFace)     // 规则在 service
   └─ replaceTrayWithPlayFieldCard(cardId)
        ├─ UndoManager::recordPlayfieldToTray(...)      // 先记录再改 model
        ├─ model: playfieldIds 删 / trayIds push / zone = TRAY
        ├─ view : 跨父节点 reparent（PlayFieldView → StackView），保留世界坐标
        └─ CardView::moveTo(traySlot, 0.3s)
```

### 4.3 备用堆点击（需求 1）

```
CardView::onTouchEnded → _onClick(cardId)
        ↓
StackView::_onStackClick(cardId)
        ↓
StackController::handleStackClick(cardId)
   └─ promoteStackCardToTray(cardId)
        ├─ UndoManager::recordStackToTray(...)
        ├─ model: stackIds 末尾弹 / trayIds push / zone = TRAY
        ├─ view : 不跨父节点（仍在 StackView 内）；从备用堆注册表移到 tray 注册表，重设点击回调
        └─ CardView::moveTo(traySlot, 0.3s)
```

### 4.4 回退（需求 3）

```
Undo 按钮 → GameView::_onUndoClick → GameController::onUndoClick
        ↓
UndoManager::undo()  pop 一个 Action → 调用 _undoExecutor(action)
        ↓
GameController::executeUndo(action)
   switch action.type:
     AT_PLAYFIELD_TO_TRAY → PlayFieldController::undoPlayfieldToTray(action)
     AT_STACK_TO_TRAY     → StackController::undoStackToTray(action)
   每个子分支:
        ├─ model 反向回滚（trayIds.pop → 原 zone 列表 push）
        ├─ view 反向 reparent / 重注册
        └─ CardView::moveTo(action.fromPosition, 0.3s)
GameController::refreshUndoButton()  // 撤销栈空时禁用按钮
```

## 5. 关键设计取舍

1. **Manager 禁单例、禁反向依赖 controller**：UndoManager 操作什么、回滚到哪儿，全部用 `std::function<void(const UndoModel::Action&)>` 回调向 GameController 注入。Manager 自己只懂"栈"，不懂"动画"。便于将来加 controller 单测，给 manager 喂 mock executor 验证状态机即可。
2. **Service 无状态**：MatchRuleService、GameModelFromLevelGenerator 都不持数据，参数进参数出。新规则、新洗牌策略加在这里，不污染 model。
3. **撤销动作=最小快照**：UndoModel::Action 只存"反向播放需要的最少字段"——`movedCardId / previousTrayCardId / fromPosition / previousZone`。这样多种动作（playfield→tray、stack→tray、未来的 stack→playfield…）共用同一个 struct。
4. **View 不知道规则**：CardView 只触发 `onClick(cardId)`；是不是合法、要不要动画，由 controller 决定。视图层不读 GameModel。
5. **跨父节点 reparent 在 controller 里做**：因为它涉及"原父节点 → 新父节点 + 注册表 + 回调"的协同，让 view 知道这件事会破坏分层。当前实现见 `PlayFieldController.cpp::replaceTrayWithPlayFieldCard` 与 `undoPlayfieldToTray`。
6. **关卡 JSON 约定**：`Stack` 数组的最后一张作为初始 tray，前 N-1 张按顺序入备用堆（末尾为栈顶）。关卡作者直观写"看见的底牌写最后"，无需关心运行时数据结构。

## 6. 扩展点 ✦

下面两条是改动量最小的"加新功能"路线。每条都标了**必改文件**与**禁改文件**——按清单走即可，越界改动应在 review 时被打回。

### 6.1 新增一种卡牌 / 新增一种花色或点数

**必改：**

1. `configs/models/CardTypes.h` — 在 `CardFaceType` 或 `CardSuitType` 中追加枚举（务必排在 `_NUM_*` 之前）。
2. `configs/models/CardResConfig.h` — `getFaceText` / `getSuitText` / `isRedSuit` 增加分支；接入真实美术资源时把 `CardView::createVisual` 改为加载 Sprite——`CardView` 对外接口不变，其它层零感知。
3. **如果新类型的匹配规则不同**（比如"小王只能和大王匹配"），改 `services/MatchRuleService::canMatch`。可考虑把签名从 `(CardFaceType, CardFaceType)` 升级为 `(const CardModel&, const CardModel&)` 以利用花色信息——这是预期内的局部重构，仍然只在 service 内。

**不需要改：**
* models 层（CardModel 已经只存 enum，对枚举值无限扩展）
* GameModel / playfieldIds / trayIds / stackIds（容器是按 id 索引的，与"长什么样"无关）
* 任何 controller / view / manager

### 6.2 新增一种回退类型（例如「stack 牌 → playfield」）

**必改：**

1. `models/UndoModel.h` — 在 `ActionType` 追加 `AT_STACK_TO_PLAYFIELD`。如果新动作需要额外快照，在 `Action` struct 加字段（旧动作不需要时默认值无所谓，避免破坏既有动作的快照含义）。
2. `managers/UndoManager.h` — 加一个 `recordStackToPlayfield(...)`，填好 Action 并 `push`。**UndoManager 的现有 API 与栈结构无需任何修改**——这是这一层的设计目的。
3. 触发该动作的 controller（比如 `StackController`）— 在执行 model 变更**之前**调 `recordStackToPlayfield(...)`；并新增一个 `undoStackToPlayfield(const UndoModel::Action&)` 方法负责反向播放（model 回滚 + view 反向 reparent + MoveTo）。
4. `controllers/GameController.cpp::executeUndo` — `switch(action.type)` 增加 `case AT_STACK_TO_PLAYFIELD: stackCtrl.undoStackToPlayfield(a); break;`。

**不需要改：**
* views（已经具备 `addXxxCardView / detachCardView` 的对称 API，跨父节点工具已经存在于 controller）
* services / models（除上述 UndoModel 字段）
* 其他 controller

> 把上面 4 步当作"加一种 undo"的检查表：缺任意一步就会出 bug——`record` 漏掉 → undo 时栈里没记录；`executeUndo` 漏掉 case → undo 静默失败；新 `undoXxx` 方法漏掉 → 类型识别但行为空。

### 6.3 新增关卡

* 在 `Resources/levels/` 加 `level_<id>.json`，按现有 schema 写 Playfield 与 Stack；
* 在 `GameScene::create(levelId)` 传入对应 id；
* 关卡 JSON 由 `<CustomBuildStep>`（vcxproj 中）每次构建自动复制到输出目录，无需手动拷贝。

## 7. 编码约束（与 `CLAUDE.md` 同步）

* 所有 `Classes/` 下 `.h/.cpp` **必须 UTF-8 + BOM**——MSVC 在中文 Windows 上会把无 BOM 文件按 GBK 解码，导致中文注释触发大批 C2447 假错误。
* 新增源文件**双注册**：
  * `proj.win32/cardgame.vcxproj` 的 `<ClCompile>` / `<ClInclude>` 以及 `cardgame.vcxproj.filters` 的过滤器；
  * `CMakeLists.txt` 的 `GAME_SOURCE` / `GAME_HEADER` 列表。
* 类命名大写起、函数/变量小驼峰、私有成员 `_` 前缀、常量 `k` 前缀。
* 类 > 500 行或函数 > 50 行须重构。
* 公有方法与成员变量需写注释（用途 / 参数 / 返回值）；不解释能从代码名字直接看出来的事。

## 8. 已知约定与坑

* **CardView 跨父节点**：`Node::removeFromParent()` 默认 `cleanup=true`，但 cocos2d-x 3.16+ 出于兼容性**未**在 `cleanup()` 里销毁事件监听器（见 `CCNode.cpp` 注释），所以重新 addChild 后 CardView 的 touch listener 依然有效。如果将来升级到行为不同的 cocos2d-x 分支，需要在 reparent 前后显式重新注册 listener。
* **撤销栈不持有 CardView 指针**：只存 `movedCardId`。如果哪天加上"局中销毁卡牌"，需要在销毁时先 `_gameModel->undoModel().clear()` 或过滤相关动作，避免 undo 时找不到对应 view/model。
* **Undo 动画期间的二次点击**：当前 controller 在 reparent 后 `setInteractive(true/false)` 但不阻止 0.3s 动画进行中的二次输入。需求未要求，未额外加锁。如果将来要加，建议在 GameController 加一个 `_inputLocked` 计数，动画开始 +1 / 结束 -1，view 层从 controller 读取一个 const flag。

---
**最后更新**：2026-05-17；对应代码状态见 git 当前 HEAD。

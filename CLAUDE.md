# CLAUDE.md

This file provides guidance to Claude (claude.ai/code) when working with code in this repository.

## Project overview

Cocos2d-x **3.17** C++ card-matching game. Engine is vendored at `cocos2d/` and built from source. Cross-platform game code lives in `Classes/`; resources in `Resources/`. Platform targets (`proj.win32`, `proj.android`, `proj.ios_mac`, `proj.linux`) all share the same `Classes/` and `Resources/` trees.

**Game rules (brief):** A playfield of face-up/face-down cards; a tray (top card is the active "base"); a draw stack. Clicking a playfield card whose face value differs by exactly 1 from the tray top moves it to the tray. Clicking the draw stack promotes the top stack card to the tray. An Undo button reverses the last move. No suit restriction.

## Building and running (Windows — primary workflow)

Open `proj.win32/cardgame.sln` in Visual Studio. Configurations: `Debug|Win32` and `Release|Win32` only (no x64). Output goes to `Debug.win32/` or `Release.win32/` at the repo root.

MSBuild from CLI (run from `proj.win32/`):
```bash
MSBuild cardgame.sln //p:Configuration=Debug //p:Platform=Win32
```

The debugger working directory is hard-coded to `..\Resources` in `cardgame.vcxproj.user`, which is required for asset paths to resolve at runtime.

The CMake build at the repo root (`CMakeLists.txt`) is the cross-platform path (Android/Linux/macOS/iOS). Its `APP_NAME` is `TemplateCpp`; the `.sln` produces `cardgame.exe` — these are separate pipelines.

## Design resolution

```cpp
glview = GLViewImpl::createWithRect("Test", cocos2d::Rect(0, 0, 1080, 2080), 0.5);
glview->setDesignResolutionSize(1080, 2080, ResolutionPolicy::FIXED_WIDTH);
```

Layout: PlayFieldView occupies the top 1500 px; StackView occupies the bottom 580 px.

## Adding new source files — mandatory double-registration

Every new `.cpp`/`.h` in `Classes/` must be registered in **two** places:

1. `proj.win32/cardgame.vcxproj` — `<ClCompile>` / `<ClInclude>` ItemGroups, and `cardgame.vcxproj.filters` for the filter tree.
2. `CMakeLists.txt` — `GAME_SOURCE` / `GAME_HEADER` `list(APPEND …)` blocks (around lines 56–63).

Omitting either will break the corresponding build pipeline silently.

### Known `.sln` gotcha

`cardgame.sln` lists four projects but `cardgame.vcxproj` has a `<ProjectReference>` to `cocos2d/external/Box2D/proj.win32/libbox2d.vcxproj` that is **not** in the `.sln`. If you ever regenerate the `.sln`, re-add `libbox2d.vcxproj` as a solution project.

## Architecture — MVC layers in `Classes/`

```
Classes/
├── configs/   — static config (load-once, read-only)
│   ├── models/CardTypes.h          shared enums (CardFaceType, CardSuitType)
│   ├── models/LevelConfig.h        raw data parsed from JSON
│   ├── models/CardResConfig.h      enum→display-text/color helpers
│   └── loaders/LevelConfigLoader   FileUtils + rapidjson → LevelConfig
├── models/    — runtime mutable state (no business logic)
│   ├── CardModel   id, face, suit, Vec2 position, Zone enum
│   ├── GameModel   owns all CardModels; exposes playfieldIds/stackIds/trayIds vectors; embeds UndoModel
│   └── UndoModel   action stack (Action = type + movedCardId + previousTrayCardId + fromPosition + previousZone)
├── services/  — stateless, no data ownership
│   ├── GameModelFromLevelGenerator  LevelConfig → GameModel (Stack[0] becomes initial tray card)
│   └── MatchRuleService             canMatch(a,b): |a−b|==1, no suit check
├── managers/  — stateful helpers, owned by controller (never singleton)
│   └── UndoManager  wraps UndoModel push/pop; dispatches rollback via UndoExecutor callback
├── controllers/  — orchestrate model↔view, handle user events
│   └── PlayFieldController  wires PlayFieldView click → MatchRuleService → model update → view animation → UndoManager record
└── views/     — cocos2d Nodes, no business logic
    ├── CardView       150×210 vector-drawn card; touch listener fires onClick callback
    ├── PlayFieldView  1080×1500 green bg; owns CardView map; forwards clicks via setOnCardClick
    ├── StackView      1080×580 green bg; two slots (stack left, tray right); owns CardView map
    └── GameView       root Node composing PlayFieldView + StackView + Undo button
```

### Data flow for a card click

```
User tap → CardView::onTouchEnded → _onClick(cardId)
  → PlayFieldView::_onCardClick(cardId)
  → PlayFieldController::handleCardClick(cardId)
      MatchRuleService::canMatch(card, trayTop)   // rule check
      UndoManager::recordPlayfieldToTray(…)       // push undo action BEFORE model change
      model update (playfieldIds ↔ trayIds, Zone, position)
      PlayFieldView::detachCardView → reparentPreservingWorld → StackView
      CardView::moveTo(traySlot, 0.3s)
```

### Undo flow

```
User taps Undo button → GameView::_onUndoClick
  → UndoManager::undo()
      pops UndoModel::Action
      calls _undoExecutor(action)           // lambda set by GameController
  → PlayFieldController::undoPlayfieldToTray(action)
      model rollback (trayIds pop, playfieldIds push, Zone/position reset)
      StackView::detachCardView → reparentPreservingWorld → PlayFieldView
      CardView::moveTo(action.fromPosition, 0.3s)
```

### Cross-parent card reparenting pattern

When a CardView moves between PlayFieldView and StackView, `reparentPreservingWorld()` (in PlayFieldController.cpp) converts world coordinates to preserve visual position:
```cpp
Vec2 world = oldParent->convertToWorldSpace(card->getPosition());
card->retain(); card->removeFromParent();
Vec2 local = newParent->convertToNodeSpace(world);
card->setPosition(local); newParent->addChild(card); card->release();
```

## Extending the game

### Adding a new card type or new face/suit

1. Add the enum value to `CardFaceType` or `CardSuitType` in `configs/models/CardTypes.h`.
2. Update `CardResConfig::getFaceText` / `getSuitText` / `isRedSuit` for display.
3. Update `MatchRuleService::canMatch` if the new type needs different matching rules.
4. No changes required in models, views, or controllers for a pure data extension.

### Adding a new undo action type

1. Add a new `ActionType` value to `UndoModel::ActionType` (e.g., `AT_STACK_TO_TRAY`).
2. Add any new snapshot fields to `UndoModel::Action` if needed.
3. Add a `recordXxx()` method to `UndoManager` that fills and pushes the Action.
4. Call `recordXxx()` from the relevant controller *before* modifying model state.
5. Extend the `_undoExecutor` lambda in `GameController` (or add a new `undoXxxToYyy()` method on the relevant controller) to handle the new type.
6. `UndoManager` itself requires no structural changes.

## Resource pipeline

Source assets live in `Resources/`. A `<CustomBuildStep>` in `cardgame.vcxproj` runs `xcopy ..\Resources <OutDir> /D /E /I /F /Y` on each build. Level JSON files are expected at `Resources/levels/level_<id>.json`. New asset folders go under `Resources/`, not under `proj.win32/`.

## Audio

`AppDelegate.cpp` has `USE_AUDIO_ENGINE` / `USE_SIMPLE_AUDIO_ENGINE` macros commented out. Uncomment exactly one before using audio — both defined simultaneously triggers a `#error`.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project context

- Cocos2d-x **3.17** C++ game project (`engine_version` in `../.cocos-project.json`).
- The current directory `proj.win32/` is **only the Windows build target**. Cross-platform game code lives in `../Classes/`. Resources are in `../Resources/`. Sibling targets (`../proj.android`, `../proj.ios_mac`, `../proj.linux`) consume the same `Classes/` and `Resources/` trees.
- The cocos2d-x engine is vendored at `../cocos2d/` and is built from source as part of the solution — there is no external engine install step.
- The project currently contains the stock cocos2d-x C++ template (a `HelloWorld` scene). No card-game logic has been added yet; treat anything beyond `AppDelegate` / `HelloWorldScene` as new work.

## Building and running (Windows)

Two build systems coexist at the repo root. For Win32 work, prefer the solution file:

- **Open** `cardgame.sln` in Visual Studio (the file is marked VS2013 format but the `.vcxproj` auto-selects `v120` / `v140` / `v141` / `v143` toolsets based on the installed VS — newer VS works fine).
- **Configurations:** `Debug|Win32` and `Release|Win32` only (no x64 configuration is defined). Build output goes to `../Debug.win32/` or `../Release.win32/` (`OutDir` is `$(SolutionDir)$(Configuration).win32\`).
- **Run:** the produced binary is `cardgame.exe` in the output dir. The VS debugger working directory is hard-coded to `$(ProjectDir)..\Resources` in `cardgame.vcxproj.user` — needed so relative asset paths like `"HelloWorld.png"` resolve.
- **MSBuild from CLI** (from `proj.win32/`):
  ```bash
  MSBuild cardgame.sln //p:Configuration=Debug //p:Platform=Win32
  ```
  Use forward-slashed switches under bash on Windows.

The CMake build at the repo root (`../CMakeLists.txt`) is the cross-platform path used by Android/Linux/macOS/iOS — it's wired up but not the primary Windows workflow. Note its `APP_NAME` is `TemplateCpp`, while the `.sln`/`.vcxproj` build a `cardgame.exe` — these two pipelines produce differently-named binaries.

## Solution layout and a build gotcha

`cardgame.sln` references four projects: `cardgame`, `libcocos2d`, `libSpine`, `librecast`. However `cardgame.vcxproj` adds a `<ProjectReference>` to `..\cocos2d\external\Box2D\proj.win32\libbox2d.vcxproj` that is **not** listed in the `.sln`. The solution still builds because of a stray Box2D GUID entry in `GlobalSection(ProjectConfigurationPlatforms)`, but if you ever regenerate or "clean" the `.sln`, re-add `libbox2d.vcxproj` as a solution project to keep the link step working.

## Resource pipeline

- Source assets live in `../Resources/`. A `<CustomBuildStep>` in `cardgame.vcxproj` runs `xcopy ..\Resources <OutDir> /D /E /I /F /Y` so changes flow into the runtime working dir on each build.
- Cocos2d-x resolves filenames against `FileUtils` search paths; default search root is the working directory, hence the `LocalDebuggerWorkingDirectory = ..\Resources` setting. New asset folders should be added under `../Resources/`, not under `proj.win32/`.

## Code architecture

The runtime entry chain is short and worth knowing before adding scenes/systems:

1. `proj.win32/main.cpp` — `_tWinMain` constructs a stack-allocated `AppDelegate` and calls `Application::getInstance()->run()`. This is the only platform-specific bootstrap on Windows; Android/iOS/macOS/Linux have their own `main`s under their respective `proj.*` dirs but converge on the same `AppDelegate`.
2. `../Classes/AppDelegate.cpp::applicationDidFinishLaunching` — sets up the GL view, design resolution, content-scale tiers, then calls `director->runWithScene(HelloWorld::createScene())`. **This is where you swap in a different initial scene.**
3. `../Classes/HelloWorldScene.cpp` — current placeholder scene. Subclasses `cocos2d::Scene`, uses the `CREATE_FUNC` macro for the autorelease `create()` factory, and sets up nodes inside `init()`.

### Resolution policy (already wired)

`AppDelegate.cpp` defines three resolution tiers (`smallResolutionSize` 480x320, `mediumResolutionSize` 1024x768, `largeResolutionSize` 2048x1536) against a `designResolutionSize` of 480x320 with `ResolutionPolicy::NO_BORDER`. `setContentScaleFactor` is chosen by frame height. If you add `resources-hd/` / `resources-ipadhd/` style asset folders, register them via `FileUtils::getInstance()->setSearchResolutionsOrder(...)` here — the hookpoint exists but isn't populated.

### Audio

`AppDelegate.cpp` has `USE_AUDIO_ENGINE` / `USE_SIMPLE_AUDIO_ENGINE` macros commented out. Pick **one** (the file `#error`s if both are defined) and uncomment before using sound — pause/resume is already wired into `applicationDidEnterBackground` / `applicationWillEnterForeground` for both options.

### Adding new gameplay code

- New `.cpp` / `.h` files belong in `../Classes/` (cross-platform). They must be added in **two** places to be picked up everywhere:
  - `cardgame.vcxproj` (`<ClCompile>` and `<ClInclude>` ItemGroups) and `cardgame.vcxproj.filters` for Win32.
  - `../CMakeLists.txt` (`GAME_SOURCE` / `GAME_HEADER` `list(APPEND ...)` blocks around lines 56–63) for the other platforms.
- Include paths already cover `Classes/`, `cocos2d/cocos`, `cocos2d/cocos/audio/include`, `cocos2d/external`, and chipmunk — no need to add per-file include dirs for engine headers.
- The `register_all_packages()` stub in `AppDelegate.cpp` is reserved for the cocos package manager — don't repurpose or delete it.

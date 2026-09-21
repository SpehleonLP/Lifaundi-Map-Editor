# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A Qt5/Qt6 + OpenGL desktop editor for authoring **metaroom** maps for the *Creatures*
game series (Creatures 1/2/3, distinguished by background formats: `.spr`, `.s16`,
`.blk`). A metaroom is a mesh of quad "faces" (rooms) sharing edges, plus per-room
properties (music track, room type, gravity, directional/ambient shade, depth, audio)
and per-edge-pair "door" permeability values. The editor renders the map with custom
GLSL shaders and edits it through a modal tool system with full undo/redo.

## Build & run

This is a **qmake** project (`MapEditor.pro`), not CMake. Qt Creator builds in-tree at
`build/Desktop_Qt_6_7_0-Debug`; a command-line build can live anywhere:

```
# Active out-of-source build dir (Qt 6.7.0 is the current default):
cd ~/Developer/Build/MapEditor/Qt6-Debug

~/Qt/6.7.0/gcc_64/bin/qmake /mnt/Passport/Kreatures-Engine/Tooling/MapEditor/MapEditor.pro \
    -spec linux-g++ CONFIG+=debug

make -j$(nproc)          # build (clean: 0 errors, 0 warnings)
./MapEditor              # run (writes logs to ./Log/status.log and ./Log/error.log)
```

The `.pro` also still builds under **Qt 5.12.0** (`~/Qt/5.12.0/gcc_64/bin/qmake`; older
build dir `~/Developer/Build/MapEditor/Debug`); it supports both via
`QT += opengl openglwidgets` on Qt6. Uses **C++20** (`CONFIG += c++2a`). When adding Qt
signal/slot `connect`s, use direct member-function pointers (or `qOverload<>`) — the
old pointer-to-member disambiguation casts are Qt5-only and break under Qt6.

There are **no automated tests**. The `Metaroom::TestTreeSymmetry` /
`TestDoorSymmetry` methods are in-app runtime invariant checks, not a test suite.

### External dependencies (superproject siblings, not vendored)

This repo is the `Tooling/MapEditor` submodule of the Kreatures-Engine superproject;
`MapEditor.pro` resolves everything relative to `$$PWD/../..`:
- `Spehleon/lib/` — `qt-gl/` (`gl_viewwidget`, `initialize_gl`, `viewparentinterface`)
  and `Support/` (`shared_array`, `counted_ptr`, `compressedshadersource`), compiled
  directly into this target.
- `ThirdParty/loguru` — logging (`loguru.cpp` compiled in).
- `ThirdParty/lz4/build/cmake` — `liblz4.a` for `.blk` background compression.
- System libs: GLEW, GL, GLU, drm, z. GLM is the math library (`glm::ivec2` everywhere).

`SimpleShaderBase` is MapEditor's own (`src/Shaders/simpleshaderbase.*`); Spehleon
deleted its copy as unused, so don't look for it there.

## Architecture

### Ownership graph

`MainWindow` (Qt widget, `mainwindow.{h,cpp,ui}`) is the root. It owns:
- `ControllerFSM toolbox` — the modal editing state machine (always present).
- `std::unique_ptr<Document> document` — the currently-open map (swapped on new/open).

`Document` (`src/document.{h,cpp}`) is the model root and the API surface the UI calls
(Undo, Copy, SetRoomMusic, SetGravity, etc.). It owns:
- `Metaroom m_metaroom` — the room mesh + all room/door data.
- `std::unique_ptr<BackgroundImage> m_background` — the loaded game background image.
- `std::vector<unique_ptr<CommandInterface>> m_history` + `m_command` cursor — undo stack.

### Metaroom data model (the core)

`Metaroom` (`src/metaroom.h`) inherits `MetaroomMemory` (`src/metaroom_memory.h`) and
composes several concern-specific mixins/members, each in its own file:
- `MetaroomMemory` — **the actual storage**. Rooms are stored SoA in parallel
  `shared_array`s: `_verts` (`array<ivec2,4>` per face), plus per-room `_gravity`,
  `_music`, `_roomType`, `_color`, `_directionalShade`, `_ambientShade`, `_audio`,
  `_depth`, and a `map<uint64_t,float> _permeabilities` keyed by door pairs.
- `EntitySystem` (`src/entitysystem.{h,cpp}`) — allocates/frees face IDs and drives
  reallocation of all the parallel arrays via a callback (`MetaroomMemory::Realloc`).
  Iterate live faces via `range()`; iterate live edges via `edgeRange()`.
- `MetaroomSelection`, `MetaroomDoors`, `MetaroomGL gl`, `QuadTree _tree` — selection
  state, door/link topology, GPU upload, and spatial index respectively.

**Edge/vertex indexing convention** (pervasive, memorize this): a face has 4 vertices;
an edge/vertex global id `i` maps to face `i>>2` and local corner `i&3`. Helpers:
`NextInEdge(id)`, `PrevInEdge(id)`, `GetOppositeEdge(id) = id^2`. Doors are keyed by
`GetDoorKey(a,b)` = the two face ids packed low<<high into a `uint64_t`.

### Editing: ControllerFSM

`ControllerFSM` (`src/controllerfsm.{h,cpp}`) turns mouse/keyboard events
(`OnLeftDown/Up`, `OnMouseMove`, `OnDoubleClick`, `SetTool`) into geometry operations.
`Tool` and `State` enums live in `src/enums.hpp`. Tools: Create, Duplicate, Select,
Translate, Rotate, Scale, Slice, Extrude, Order, Face, plus gravity-specific tools.
Keyboard shortcuts for tools are `QShortcut` members on `MainWindow`. Moves are
staged (`BeginMove`/`CommitMove`/`CancelMove`) using `_scratch`/`_prev` vertex arrays
before being committed.

### Undo/redo

Every mutation goes through `Document::PushCommand`, which stores a
`CommandInterface` (`src/commandlist.h`) implementing `RollForward()`/`RollBack()`
(`TransformCommand`, `DeleteCommand`, `SettingCommand`, `DifferentialSetCommmand`,
etc.). `Undo`/`Redo` move the `m_command` cursor over `m_history`.

### Rendering

`GLViewWidget` (`src/glviewwidget.{h,cpp}`) is the `QOpenGLWidget` (GL 4.5 core). It
owns a `Shaders` bundle (`src/Shaders/shaders.h`) — a struct aggregating every shader
object (`ArrowShader`, `DoorShader`, `RoomOutlineShader`, `SelectedRoomShader`,
`TransparencyShader`, `UniformColorShader`, `MouseShader`, `BlitShader`,
`ComputeHistogram`) plus `DefaultTextures`/`DefaultVAOs`. Shaders derive from
Spehleon's `SimpleShaderBase`. Many render paths take `Shaders *` as their first
argument (`Document::RenderBackground`, `ControllerFSM::Render`, `Metaroom::gl`).

### File format

`Metaroom::Read`/`Write` (`src/metaroom.cpp`) is a hand-rolled binary format with a
4-byte magic, a `uint16` version (`VERSION = 6`; older versions gated by the
`VERSION_ADDED_*` enum in `metaroom.h`), face count, then the SoA arrays. When adding
a persisted room property, bump `VERSION`, add a `VERSION_ADDED_*` constant, and gate
the new field's read on `version >= ...` to stay backward-compatible.

## Conventions & gotchas

- **Coordinates are integer** (`glm::ivec2`/`i16vec2`) for room geometry; only
  rendering/zoom uses floats. Vertices are `i16` on disk.
- Errors surface through **loguru → Qt message boxes** (see `main.cpp` `LogHandler`):
  `LOG_F(ERROR, ...)` pops a dialog and closes the window; `FATAL` exits. Prefer
  `WARNING` for recoverable issues.
- Feature flags gate incomplete subsystems: `HAVE_FMOD 0` (music playback),
  `HAVE_WALL_TYPE 0`, `HAVE_UUID 1`. Check these before touching FMOD/wall-type code.
- `.pro.user*` files are QtCreator local state — do not treat as source of truth.
- `src/Layers/` and `src/Compresser/` are largely empty/WIP (only an `.autosave`);
  the live background code is `src/backgroundimage.{h,cpp}`.
- Generated `moc_*`, `ui_*`, and object files live in the external build dir, never
  in the source tree.

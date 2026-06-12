# RAD-2D

**RAD-2D** (Raylib Animation & Drawing, 2D) is a **2D drawing and animation
wrapper** for [raylib](https://www.raylib.com/), written in C++17.

The point of this library is to make drawing — and especially animation — more
abstract, so you're not manually juggling source rectangles and frame timing
throughout your code. You describe *what* to draw and *which* animation to play;
the library handles the per-frame cropping, timing, and rendering.

It's built around three ideas:

- **Asset registries** — load textures and fonts once, look them up by id, and
  have them unloaded automatically when the registry dies.
- **A central animation registry** — one place to define every animation in
  your game. Define a `"walk"` cycle once and any number of sprites can share
  it, which is ideal when characters reuse a consistent sprite-sheet layout.
- **Drawables** — objects that represent anything you put on screen: sprites,
  scrolling backgrounds, text, HUD/UI elements, and tile layers. They share a
  common `Drawable` base (so you can store them polymorphically and call
  `draw()` on each), and the animated ones pull from the shared animation
  registry and render the current frame in a single `draw()` call.

You still call raylib directly for the window and frame loop (`InitWindow`,
`BeginDrawing`, …) — this library sits on top to remove the bookkeeping, not to
hide raylib from you.

> 📖 **Full usage guide:** see **[DOCUMENTATION.md](DOCUMENTATION.md)** for the
> complete walkthrough of every type and how the pieces fit together.

---

## Quick start

All public types live in the `rad2d` namespace, and a single umbrella header
(`rad2d.hpp`) pulls in the whole library.

```cpp
#include "raylib.h"
#include "rad2d.hpp"

using namespace rad2d;

enum TextureKey { SHEET };
enum AnimId     { IDLE };

int main() {
    InitWindow(1080, 720, "RAD-2D");
    SetTargetFPS(60);

    // Load the sprite sheet. The registry owns it and unloads it for you.
    TextureRegistry textures;
    textures.load(SHEET, "player.png");

    // Register a looping animation: frames 0-2, each shown for 0.3s.
    // AnimRule(isRepeating, returnToFirstFrame, pingPong)
    AnimationRegistry anims;
    anims.add(IDLE, std::make_shared<Animation>(
        "idle",
        std::vector<Frame>{
            Frame(0.30f, Rectangle{   0, 0, 64, 64 }),
            Frame(0.30f, Rectangle{  64, 0, 64, 64 }),
            Frame(0.30f, Rectangle{ 128, 0, 64, 64 }),
        },
        AnimRule(true, false, false)
    ));

    // Build the sprite at (100,100), drawn at 256x256, and play the animation.
    Sprite player("player", &anims, 100, 100, 0, 256, 256);
    player.setTexture(textures.get(SHEET));
    player.addAnimation(IDLE);
    player.setAnimation(IDLE, /*play=*/true);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            player.draw();   // advances the animation and draws the current frame
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

For a complete tour of sprites, backgrounds, text, UI, and tilemaps, read
**[DOCUMENTATION.md](DOCUMENTATION.md)**.

---

## Building

Requires a C++17 compiler and [raylib](https://www.raylib.com/) (found via
its CMake config or `pkg-config`). There are no other dependencies.

### CMake (recommended)

```bash
cmake -S . -B build
cmake --build build      # produces librad2d.a
```

To consume the library from your own CMake project, vendor it (or use
`FetchContent`) and link the exported target:

```cmake
add_subdirectory(rad2d)              # or FetchContent_Declare(...)
target_link_libraries(your_app PRIVATE rad2d::rad2d)
```

`rad2d::rad2d` carries its include path with it, so `#include "rad2d.hpp"`
just works. `cmake --install build` also installs the library, headers, and a
`find_package(rad2d)` config.

### Make

The Makefile builds the static library on Linux:

```bash
make        # build librad2d.a
make clean  # remove build artifacts
```

To use the library without CMake, add the repo root to your include path and
compile the `.cpp` files alongside your code:

```bash
g++ -std=c++17 your_app.cpp \
    animation/animation.cpp draw/draw.cpp assets/assets.cpp tiles/tiles.cpp \
    -I. $(pkg-config --cflags --libs raylib) -o your_app
```

---

## Project layout

```
rad2d.hpp       Umbrella header — include this to get everything
animation/      Frame, AnimRule, Animation, AnimationRegistry, AnimationState
assets/         TextureRegistry, FontRegistry
draw/           Drawable + AnimatedDrawable, Sprite, Background, Text, UI
tiles/          Tile, TileSet, Tilemap
CMakeLists.txt  Library build (packaging / install)
Makefile        Simple Linux build of the static library
DOCUMENTATION.md  Full usage guide and API reference
```

---

## Documentation

The full API walkthrough lives in **[DOCUMENTATION.md](DOCUMENTATION.md)**.
Version history is in **[CHANGELOG.md](CHANGELOG.md)**.

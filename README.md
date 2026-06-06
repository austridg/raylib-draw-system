# Animation

> ⚠️ **Beta.** The core animation + sprite pipeline works; `Text` and `Tile`
> drawables and other functionality are still on the way. See
> [Status & roadmap](#status--roadmap).

A **2D drawing and animation wrapper** for
[raylib](https://www.raylib.com/), written in C++17.

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
- **Drawables** — objects that represent anything you put on screen (sprites
  today; text and tiles planned). A drawable pulls from the animation registry,
  keeps a curated "usable" list of the animations it's allowed to play, and
  renders the current frame in a single `draw()` call.

You still call raylib directly for the window and frame loop (`InitWindow`,
`BeginDrawing`, …) — this library sits on top to remove the bookkeeping, not to
hide raylib from you.

---

## Quick start

```cpp
#include "raylib.h"
#include "assets/assets.h"
#include "draw/draw.h"
#include "animation/animation.h"

enum TextureKey { SHEET };
enum AnimId     { IDLE };

int main() {
    InitWindow(1080, 720, "Animation Library - Example");
    SetTargetFPS(60);

    // Load the sprite sheet. The registry owns it and unloads it for you.
    TextureRegistry textures;
    textures.load(SHEET, "test_anim.png");

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

This is essentially `main.cpp`, the bundled example. Build and run it with
`make run`.

---

## Concepts

### Assets — `assets/assets.h`

`TextureRegistry` and `FontRegistry` are keyed stores for raylib resources.
You load by id and retrieve a `shared_ptr`; the resource is unloaded
(`UnloadTexture` / `UnloadFont`) automatically when the last reference goes
away.

```cpp
TextureRegistry textures;
textures.load(SHEET, "player.png");
std::shared_ptr<Texture2D> tex = textures.get(SHEET);
```

Using plain `int` ids means you can back them with an `enum` for readable,
collision-free keys.

### Animations — `animation/animation.h`

An animation is an ordered list of **frames**. Each `Frame` is a duration plus
the source rectangle to crop from the sprite sheet:

```cpp
Frame(0.30f, Rectangle{ 0, 0, 64, 64 });  // show this 64x64 crop for 0.3s
```

Playback behaviour is controlled by an `AnimRule`:

| Field                | Effect                                                          |
| -------------------- | -------------------------------------------------------------- |
| `isRepeating`        | Loop forever instead of stopping after one pass.               |
| `returnToFirstFrame` | When a non-repeating animation finishes, snap back to frame 0. |
| `pingPong`           | Play forward to the end, then bounce back to the start.        |

```cpp
AnimRule(/*isRepeating=*/true, /*returnToFirstFrame=*/false, /*pingPong=*/false);
```

`Animation`s are immutable, shared data. Store them in an `AnimationRegistry`
(again keyed by id) so many sprites can share a single definition:

```cpp
AnimationRegistry anims;
anims.add(RUN, std::make_shared<Animation>("run", frames, rules));
```

### Drawables & sprites — `draw/draw.h`

`Drawable` is the base for anything rendered on screen — it owns position
(`x`, `y`, `z`-layer) and size, with getters/setters for each.

`Sprite` is a `Drawable` that pairs a texture with an animation:

```cpp
Sprite player("player", &anims, /*x*/100, /*y*/100, /*z*/0, /*w*/256, /*h*/256);
player.setTexture(textures.get(SHEET));

player.addAnimation(IDLE);          // declare which registry animations it may use
player.addAnimation(RUN);

player.setAnimation(RUN, true);     // make one active and start playing
player.stop();                      // pause (optionally reset to frame 0)
player.play();                      // resume
player.draw();                      // advance by elapsed time + render
```

A sprite holds a **non-owning** pointer to the `AnimationRegistry` and a set of
ids it is allowed to use. The actual `Animation` data lives in the registry and
is fetched on `setAnimation`, so definitions are shared rather than copied per
sprite.

This is the payoff of the central registry: define an animation once and let
many sprites reuse it. If your characters share a sprite-sheet layout, a single
`WALK` animation can drive all of them — you only describe the frames in one
place.

```cpp
// One shared definition...
anims.add(WALK, std::make_shared<Animation>("walk", walkFrames, AnimRule(true, false, false)));

// ...used by many sprites.
Sprite hero  ("hero",  &anims, 100, 100, 0, 64, 64);
Sprite goblin("goblin", &anims, 300, 100, 0, 64, 64);

for (Sprite* s : { &hero, &goblin }) {
    s->setTexture(textures.get(SHEET));
    s->addAnimation(WALK);
    s->setAnimation(WALK, /*play=*/true);
}
```

Timing is handled by an internal `AnimationState` that accumulates raylib's
frame delta and drains it frame-by-frame, so playback stays correct (and
catches up after a lag spike) regardless of render frame rate.

---

## Building

Requires a C++17 compiler, [raylib](https://www.raylib.com/) (found via
`pkg-config`), and [`rang.hpp`](https://github.com/agauniyal/rang) for coloured
log output (expected at `/usr/local/include`).

```bash
make        # build the example -> ./anim_example
make run    # build (if needed) and run it
make clean  # remove build artifacts
```

To use the library in your own project, add the module directories to your
include path and compile the `.cpp` files alongside your code:

```bash
g++ -std=c++17 your_app.cpp \
    animation/animation.cpp draw/draw.cpp assets/assets.cpp \
    -Ianimation -Idraw -Iassets -I/usr/local/include \
    $(pkg-config --libs raylib) -o your_app
```

> The Makefile is intentionally a simple single-platform (Linux) build. If the
> library grows to target Windows/macOS or ship as a package, the plan is to
> migrate to CMake.

---

## Project layout

```
animation/     Frame, AnimRule, Animation, AnimationRegistry, AnimationState
assets/        TextureRegistry, FontRegistry
draw/          Drawable base class + Sprite
main.cpp       Example / demo program
test_anim.png  Sample 64x64-frame sprite sheet used by the example
Makefile       Build the example
```

---

## Status & roadmap

**Beta.** The core animation and sprite pipeline is working. Still to come:

- **`Text` drawable** — render strings via the `FontRegistry`.
- **`Tile` drawable** — tile-map / grid rendering.
- Additional drawing/animation functionality and conveniences.

`Drawable` is designed to be stored polymorphically (e.g.
`std::vector<std::unique_ptr<Drawable>>`) so future drawable types slot into the
same render path as `Sprite`.

# RAD-2D — Documentation

A complete guide to **RAD-2D** (Raylib Animation & Drawing, 2D): what every type
does, how the pieces fit together, and how to use them.

This is a wrapper that sits *on top of* [raylib](https://www.raylib.com/). You
still own the window and the frame loop (`InitWindow`, `BeginDrawing`,
`EndDrawing`, `CloseWindow`); RAD-2D removes the per-frame bookkeeping of source
rectangles, frame timing, and resource lifetimes.

## Table of contents

- [Mental model](#mental-model)
- [Setup & includes](#setup--includes)
- [Assets — textures & fonts](#assets--textures--fonts)
- [Animations](#animations)
- [Drawables](#drawables)
  - [Drawable (base)](#drawable-base)
  - [AnimatedDrawable (base)](#animateddrawable-base)
  - [Sprite](#sprite)
  - [Background](#background)
  - [Text](#text)
  - [UI](#ui)
- [Tiles & tilemaps](#tiles--tilemaps)
- [Putting it together](#putting-it-together)
- [API quick reference](#api-quick-reference)

---

## Mental model

RAD-2D revolves around four kinds of object:

| Thing                | What it is                                                              |
| -------------------- | ---------------------------------------------------------------------- |
| **Registries**       | Keyed stores that own a resource (texture, font) or a definition (animation) and hand out shared references. |
| **`Animation`**      | An immutable list of frames (crop rectangle + duration) plus playback rules. Lives in the animation registry and is *shared*, not copied. |
| **`Drawable`**       | Anything you render: a sprite, a scrolling background, text, a HUD panel, a tile layer. All share one base and a single `draw()` call. |
| **`AnimationState`** | The per-drawable playback cursor (which frame, how much time has banked, playing or paused). You rarely touch it directly. |

The key idea: **definitions are shared, playback is per-object.** One `"walk"`
animation lives in the registry; ten sprites can each play it, each with its own
independent position in the cycle.

Everything lives in the `rad2d` namespace.

---

## Setup & includes

Include the umbrella header and you have the whole library:

```cpp
#include "raylib.h"
#include "rad2d.hpp"

using namespace rad2d;   // or prefix everything with rad2d::
```

Version macros are available if you need to gate on them:

```cpp
RAD2D_VERSION_MAJOR   // 0
RAD2D_VERSION_MINOR   // 3
RAD2D_VERSION_PATCH   // 0
```

A typical program looks like:

```cpp
int main() {
    InitWindow(1280, 720, "My Game");
    SetTargetFPS(60);

    // ... create registries and drawables here ...

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            // ... call draw() on your drawables ...
        EndDrawing();
    }

    CloseWindow();   // registries clean up their resources as they go out of scope
    return 0;
}
```

Tip: back your registry keys with `enum`s. They give you readable,
collision-free integer ids.

```cpp
enum TextureKey { SHEET, BACKDROP, TEXTBOX, PORTRAIT };
enum FontKey    { MAIN };
enum AnimId     { IDLE, WALK, WATER };
```

---

## Assets — textures & fonts

`assets/assets.h`

`TextureRegistry` and `FontRegistry` are keyed stores for raylib resources. You
load by id and retrieve a `shared_ptr`; the underlying resource is unloaded
(`UnloadTexture` / `UnloadFont`) automatically when the last reference goes away.

```cpp
TextureRegistry textures;
textures.load(SHEET, "player.png");                    // load once, keyed by id
std::shared_ptr<Texture2D> tex = textures.get(SHEET);  // retrieve anywhere

FontRegistry fonts;
fonts.load(MAIN, "font.ttf");
std::shared_ptr<Font> f = fonts.get(MAIN);
```

`get` on an **unknown id** logs a warning and returns `nullptr` rather than
throwing — so a missing asset degrades gracefully instead of crashing.

Because `get` returns a `shared_ptr`, you can hand the same texture to several
drawables; the registry keeps it alive for as long as any of them holds it.

| Method                                  | Effect                                              |
| --------------------------------------- | --------------------------------------------------- |
| `load(int id, const std::string& path)` | Load a texture/font from disk and store it under `id`. |
| `get(int id)`                           | Return the `shared_ptr` for `id`, or `nullptr` if unknown. |

---

## Animations

`animation/animation.h`

An animation is an ordered list of **frames**. Each `Frame` is a duration (in
seconds) plus the source rectangle to crop from the sprite sheet:

```cpp
Frame(0.30f, Rectangle{ 0, 0, 64, 64 });  // show this 64x64 crop for 0.3s
```

### Playback rules — `AnimRule`

How an animation plays back is controlled by an `AnimRule`:

| Field                | Effect                                                          |
| -------------------- | -------------------------------------------------------------- |
| `isRepeating`        | Loop forever instead of stopping after one pass.               |
| `returnToFirstFrame` | When a non-repeating animation finishes, snap back to frame 0. |
| `pingPong`           | Play forward to the end, then bounce back to the start.        |

```cpp
AnimRule(/*isRepeating=*/true, /*returnToFirstFrame=*/false, /*pingPong=*/false);
AnimRule();   // default-constructed: no repeat, no return, no ping-pong
```

### Defining an animation

`Animation`s are **immutable, shared data**. Construct one with a name, its
frames, and its rules, then store it in an `AnimationRegistry` (keyed by id) so
many drawables can share a single definition:

```cpp
std::vector<Frame> walkFrames {
    Frame(0.12f, Rectangle{   0, 0, 32, 32 }),
    Frame(0.12f, Rectangle{  32, 0, 32, 32 }),
    Frame(0.12f, Rectangle{  64, 0, 32, 32 }),
    Frame(0.12f, Rectangle{  96, 0, 32, 32 }),
};

AnimationRegistry anims;
anims.add(WALK, std::make_shared<Animation>("walk", walkFrames, AnimRule(true, false, false)));
```

This central registry is the payoff: define `"walk"` once and any number of
sprites (or UI panels, or tiles) can reuse it. If your characters share a
sprite-sheet layout, a single `WALK` animation can drive all of them.

| Type / method                          | Effect                                              |
| -------------------------------------- | --------------------------------------------------- |
| `Frame(float seconds, Rectangle src)`  | One frame: how long to show it, and what to crop.   |
| `AnimRule(bool repeat, bool returnFirst, bool pingPong)` | Playback behaviour.                |
| `Animation(name, frames, rules)`       | An immutable animation definition.                  |
| `AnimationRegistry::add(int id, shared_ptr<Animation>)` | Store a definition under `id`.     |
| `AnimationRegistry::get(int id)`       | Retrieve a definition by `id`.                      |

### Timing & `AnimationState`

`AnimationState` is the per-drawable playback cursor. It accumulates raylib's
frame delta (`GetFrameTime`) and drains it frame-by-frame, so playback stays
correct regardless of render frame rate — and catches up after a lag spike. You
normally never touch it directly: `Sprite`, `Background`, and `UI` own one
internally and expose `play()` / `stop()` / `setAnimation()` over it.

---

## Drawables

`draw/draw.h`

### Drawable (base)

`Drawable` is the base for anything rendered on screen. It owns position
(`x`, `y`, and a `z` layer) and size, with getters/setters for each, and a pure
virtual `draw()`.

```cpp
void setPositionX(int x);   int getPositionX() const;
void setPositionY(int y);   int getPositionY() const;
void setLayerZ(int z);      int getLayerZ()    const;
void setWidth(int w);       int getWidth()     const;
void setHeight(int h);      int getHeight()    const;
virtual void draw() = 0;    // every drawable knows how to render itself
```

Because `draw()` is pure virtual, you can store every kind of drawable behind
the base pointer and render them uniformly:

```cpp
std::vector<std::unique_ptr<Drawable>> scene;
scene.push_back(std::make_unique<Sprite>(/*...*/));
scene.push_back(std::make_unique<Background>(/*...*/));

for (auto& d : scene) d->draw();   // one render path for everything
```

(The `z` layer is yours to use for ordering — e.g. sort `scene` by `getLayerZ()`
before drawing so lower layers render first.)

### AnimatedDrawable (base)

`Sprite`, `Background`, and `UI` share an `AnimatedDrawable` base that holds the
registry pointer, the set of animation ids the drawable is allowed to use, and
the `AnimationState`. The animation plumbing lives here in one place:

```cpp
void addAnimation(int registryId);          // declare an animation this drawable MAY use
bool setAnimation(int registryId);          // make a declared animation active (false if not allowed/found)
void setAnimation(int registryId, bool play); // ...and optionally start playing immediately
void play();                                // resume playback
void stop();                                // pause
void stop(bool resetIdx);                   // pause, optionally snapping back to frame 0
```

The drawable holds a **non-owning** pointer to the `AnimationRegistry` and a set
of ids it may use. The actual `Animation` data lives in the registry and is
fetched on `setAnimation`, so definitions are shared rather than copied per
object. `addAnimation` is the gate: you must declare an id before you can
`setAnimation` to it.

### Sprite

A `Sprite` is an `AnimatedDrawable` that pairs a texture (the sprite sheet) with
an animation.

```cpp
Sprite player("player", &anims, /*x*/100, /*y*/100, /*z*/0, /*w*/256, /*h*/256);
player.setTexture(textures.get(SHEET));

player.addAnimation(IDLE);          // declare which registry animations it may use
player.addAnimation(WALK);

player.setAnimation(WALK, true);    // make one active and start playing
player.stop();                      // pause
player.play();                      // resume
player.draw();                      // advance by elapsed time + render the current frame
```

The width/height passed to the constructor are the **on-screen** draw size; the
crop rectangle comes from the active animation's current frame. So a 32×32 sheet
frame can be drawn at 256×256.

Sharing one definition across many sprites:

```cpp
Sprite hero  ("hero",   &anims, 100, 100, 0, 64, 64);
Sprite goblin("goblin", &anims, 300, 100, 0, 64, 64);

for (Sprite* s : { &hero, &goblin }) {
    s->setTexture(textures.get(SHEET));
    s->addAnimation(WALK);
    s->setAnimation(WALK, /*play=*/true);
}
```

### Background

`Background` fills an area with a texture in one of two modes:

- **Scroll** — a single tileable texture slid via `TEXTURE_WRAP_REPEAT` for a
  seamless infinite loop. Great for parallax: stack several at different scroll
  speeds and `z` layers.
- **Animated** — when an animation from the registry is active, its current
  frame is drawn across the area instead.

```cpp
Background stars("stars", nullptr, 0, 0, 0, 1280, 720); // nullptr = scroll only
stars.setTexture(textures.get(BACKDROP));
stars.setScrollSpeed(20.0f, 0.0f);                      // drift right at 20 px/s
// in the loop:
stars.draw();
```

Pass an `AnimationRegistry*` (instead of `nullptr`) if you want the animated
mode; then `addAnimation` / `setAnimation` as usual.

`setTexture` automatically flips the texture to `REPEAT` wrapping so the tiling
trick works. The two modes **don't combine**: GPU wrap tiles the whole texture,
not one frame of a sheet — so a background is either scrolling *or* playing a
sheet animation at a time.

| Method                          | Effect                                                |
| ------------------------------- | ----------------------------------------------------- |
| `setTexture(shared_ptr<Texture2D>)` | Set the image and enable REPEAT wrapping.         |
| `setScrollSpeed(float vx, float vy)` | Scroll velocity in px/sec; negatives reverse.    |

### Text

`Text` renders a string glyph-by-glyph, which is what enables a **typewriter**
reveal and per-glyph **effects** (shake, wave, fade). It is intentionally just
the *rendering* half of a text system — a markup/dialogue layer can sit on top
and drive these knobs.

```cpp
Text line("dialogue", 40, 40, 0);
line.setFont(fonts.get(MAIN));         // omit for raylib's default font
line.setText("* You feel determined.");
line.setFontSize(20.0f);
line.setColor(WHITE);

line.enableTypewriter(20.0f);          // reveal at 20 chars/sec
line.setEffect(TextEffect::WAVE);
line.setEffectParams(/*amplitude*/3.0f, /*speed*/6.0f);
line.setOnReveal([](int glyphIndex, int codepoint){ /* play a blip sound */ });

// in the loop:
line.draw();                           // advances the reveal + effect clocks, then draws
```

**Effects** (`TextEffect`):

| Value      | Effect                                            |
| ---------- | ------------------------------------------------- |
| `NONE`     | Plain text.                                        |
| `SHAKE`    | Each glyph jitters around its position.            |
| `WAVE`     | Glyphs ride a sine wave (phase offset per glyph).  |
| `FADE_IN`  | The whole string ramps from transparent to opaque. |
| `FADE_OUT` | The whole string ramps from opaque to transparent. |

**Typewriter & state controls:**

| Method                          | Effect                                                       |
| ------------------------------- | ------------------------------------------------------------ |
| `setText(const std::string&)`   | Replace the string and restart the reveal.                   |
| `enableTypewriter(float cps)`   | Reveal glyphs at `cps` characters/second.                    |
| `disableTypewriter()`           | Show the whole string at once.                               |
| `restart()`                     | Replay the reveal from the first glyph.                      |
| `revealAll()`                   | Skip straight to fully shown.                                |
| `isFinished()`                  | `true` once every glyph is visible.                          |
| `setFont` / `setFontSize` / `setSpacing` / `setColor` | Glyph atlas, size, extra px between glyphs, tint. |
| `setEffect` / `setEffectParams` / `setFadeDuration` | Choose and tune the active effect.            |
| `setOnReveal(cb)`               | Callback `(glyphIndex, codepoint)` fired the first time each glyph appears — wire a per-letter "blip" sound here. |

RAD-2D deliberately stays out of audio: the `setOnReveal` hook is how you connect
text to sound in your own game.

### UI

`UI` is a composite HUD/menu element: an (optionally animated) **background**
texture, an optional **icon**, and an optional attached **`Text`**. Any piece may
be left unset. Like `Sprite`, it animates its background by pulling from the
shared registry — so a UI panel and a sprite can play the very same animation.

```cpp
UI box("textbox", &anims, 40, 380, 10, 480, 96);
box.setBackground(textures.get(TEXTBOX));
box.setIcon(textures.get(PORTRAIT));
box.setIconOffset(8, 8);               // icon position relative to the element's origin
box.setIconSize(64, 64);
box.setText(&line);                    // attach an external Text (not owned by the UI)

// in the loop:
box.draw();                            // draws background -> icon -> text (whichever are set)
```

The attached `Text` is **non-owning** — the `UI` does not delete it, so keep it
alive yourself. Animation controls (`addAnimation` / `setAnimation` / `play` /
`stop`) come from `AnimatedDrawable` and animate the background surface.

| Method                          | Effect                                              |
| ------------------------------- | --------------------------------------------------- |
| `setBackground(shared_ptr<Texture2D>)` | The panel background (the animated surface). |
| `setIcon(shared_ptr<Texture2D>)`| Optional icon drawn over the background.            |
| `setIconOffset(int x, int y)`   | Icon position relative to the element's origin.     |
| `setIconSize(int w, int h)`     | Icon draw size.                                     |
| `setText(Text*)`                | Attach an external `Text` (not owned).              |

---

## Tiles & tilemaps

`tiles/tiles.h`

A `TileSet` maps **tile ids** (what your map data stores) to definitions — static
or animated. Animated tiles pull their `Animation` from the shared central
`AnimationRegistry`, so a tile can use the very same animation as a sprite.

A `Tilemap` is a single `z`-layer holding a flat, **row-major** grid of ids
(`0` = empty). Stack several `Tilemap`s with different `z` values for multi-layer
maps. The map *format* (JSON / CSV / …) lives in your game; you feed the parsed
ids in.

```cpp
TileSet tiles(&anims);
tiles.defineTile(1, textures.get(SHEET), { 0, 0, 16, 16 });   // static: crop a fixed rect
tiles.defineAnimatedTile(2, textures.get(SHEET), WATER);      // animated: plays WATER from the registry

Tilemap ground("ground", &tiles, /*x*/0, /*y*/0, /*z*/0, /*tileSize*/16);
ground.setTiles({ 1,1,2,0,
                  1,2,2,1 }, /*cols*/4, /*rows*/2);

// in the loop:
tiles.update();   // once per frame: advances every animated tile in sync
ground.draw();    // draws each non-empty cell at origin + (col,row) * tileSize
```

Call `TileSet::update()` **once per frame**. It advances each animated tile *type*
a single time, which keeps every cell of that type in sync and cheap (rather than
ticking thousands of cells individually).

| Type / method                                    | Effect                                              |
| ------------------------------------------------ | --------------------------------------------------- |
| `TileSet(AnimationRegistry* reg)`                | A tile-definition set sharing the central registry. |
| `defineTile(int id, tex, Rectangle source)`      | Define a static tile that crops `source`.           |
| `defineAnimatedTile(int id, tex, int animId)`    | Define an animated tile playing `animId`.           |
| `TileSet::update()`                              | Advance all animated tiles by one frame (call once/frame). |
| `TileSet::get(int id)`                           | Look up a tile; `nullptr` if never defined.         |
| `Tilemap(name, TileSet*, x, y, z, tileSize)`     | A single tile layer drawn through a shared set.     |
| `setTiles(const std::vector<int>& ids, cols, rows)` | Load a whole layer at once (row-major).          |
| `setTile(int col, int row, int id)`              | Poke a single cell; out-of-range is ignored.        |
| `getTile(int col, int row)`                      | Read a cell; `0` (empty) if out of range/unset.     |
| `setTileSize(int size)`                          | Pixels per tile (square).                           |

---

## Putting it together

A small but complete sketch wiring several drawable types into one loop:

```cpp
#include "raylib.h"
#include "rad2d.hpp"
using namespace rad2d;

enum Tex { SHEET, BACKDROP };
enum Anim { IDLE, WATER };

int main() {
    InitWindow(1280, 720, "RAD-2D demo");
    SetTargetFPS(60);

    TextureRegistry textures;
    textures.load(SHEET, "sheet.png");
    textures.load(BACKDROP, "stars.png");

    AnimationRegistry anims;
    anims.add(IDLE, std::make_shared<Animation>("idle",
        std::vector<Frame>{ Frame(0.3f, {0,0,32,32}), Frame(0.3f, {32,0,32,32}) },
        AnimRule(true, false, false)));
    anims.add(WATER, std::make_shared<Animation>("water",
        std::vector<Frame>{ Frame(0.2f, {0,16,16,16}), Frame(0.2f, {16,16,16,16}) },
        AnimRule(true, false, false)));

    Background bg("bg", nullptr, 0, 0, 0, 1280, 720);
    bg.setTexture(textures.get(BACKDROP));
    bg.setScrollSpeed(15.0f, 0.0f);

    Sprite hero("hero", &anims, 200, 300, 1, 128, 128);
    hero.setTexture(textures.get(SHEET));
    hero.addAnimation(IDLE);
    hero.setAnimation(IDLE, true);

    TileSet tiles(&anims);
    tiles.defineTile(1, textures.get(SHEET), { 0, 0, 16, 16 });
    tiles.defineAnimatedTile(2, textures.get(SHEET), WATER);
    Tilemap ground("ground", &tiles, 0, 600, 0, 16);
    ground.setTiles({ 1,2,2,1, 1,1,1,1 }, 4, 2);

    while (!WindowShouldClose()) {
        tiles.update();                     // advance animated tiles once

        BeginDrawing();
            ClearBackground(BLACK);
            bg.draw();                      // back layer
            ground.draw();
            hero.draw();                    // front layer
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

---

## API quick reference

**Assets** — `assets/assets.h`
- `TextureRegistry`: `load(id, path)`, `get(id) -> shared_ptr<Texture2D>`
- `FontRegistry`: `load(id, path)`, `get(id) -> shared_ptr<Font>`

**Animation** — `animation/animation.h`
- `Frame(float seconds, Rectangle source)`
- `AnimRule(bool isRepeating, bool returnToFirstFrame, bool pingPong)`
- `Animation(name, vector<Frame>, AnimRule)` — immutable; `getFrames()`
- `AnimationRegistry`: `add(id, shared_ptr<Animation>)`, `get(id)`
- `AnimationState` — per-drawable cursor; managed for you by animated drawables

**Drawables** — `draw/draw.h`
- `Drawable` (base): position/size get/set, pure-virtual `draw()`
- `AnimatedDrawable` (base): `addAnimation(id)`, `setAnimation(id[, play])`, `play()`, `stop([resetIdx])`
- `Sprite`: `setTexture(tex)`, `draw()`
- `Background`: `setTexture(tex)`, `setScrollSpeed(vx, vy)`, `draw()`
- `Text`: `setText`, `setFont`, `setFontSize`, `setSpacing`, `setColor`, `enableTypewriter`/`disableTypewriter`, `restart`, `revealAll`, `isFinished`, `setEffect`, `setEffectParams`, `setFadeDuration`, `setOnReveal`, `draw()`
- `TextEffect`: `NONE`, `SHAKE`, `WAVE`, `FADE_IN`, `FADE_OUT`
- `UI`: `setBackground`, `setIcon`, `setIconOffset`, `setIconSize`, `setText(Text*)`, `draw()`

**Tiles** — `tiles/tiles.h`
- `TileSet(reg)`: `defineTile(id, tex, source)`, `defineAnimatedTile(id, tex, animId)`, `update()`, `get(id)`
- `Tilemap(name, set, x, y, z, tileSize)`: `setTiles(ids, cols, rows)`, `setTile(col, row, id)`, `getTile(col, row)`, `setTileSize(size)`, `draw()`

See **[README.md](README.md)** for the overview and build instructions, and
**[CHANGELOG.md](CHANGELOG.md)** for version history.

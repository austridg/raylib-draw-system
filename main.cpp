#include "raylib.h"

#include "rad2d.hpp"

#include <cstdlib> // getenv
#include <vector>
#include <memory>

using namespace rad2d;

/*
=== === ===
EXAMPLE
A guided tour of every RAD-2D drawable on top of raylib, using the bundled
space-shooter art:

  - Background : a scrolling starfield          (space_background.png)
  - Sprite     : an animated ship + enemies     (pointer_shooter.png, bad_guy1.png)
  - Tilemap    : grass border + animated water  (example_tileset.png)
  - UI + Text  : an animated textbox with a typewriter line + a moon icon

Everything that animates pulls from ONE shared AnimationRegistry - the sprites,
the water tile, and the textbox's blinking arrow all live in the same place.
=== === ===
*/

enum Tex    { STARS, SHIP, ENEMY, TILESET, TEXTBOX, MOON };
enum FontId { FONT_MAIN };
enum Anim   { SHIP_FLY, ENEMY_FLY, WATER, BOX_IDLE, BOX_BLINK };
enum TileId { T_EMPTY = 0, T_GRASS = 1, T_WATER = 2 };

// build a horizontal strip of `count` frames (each w x h) starting at (x0,y0)
static std::vector<Frame> strip(int x0, int y0, int w, int h, int count, float t) {
    std::vector<Frame> frames;
    for (int i = 0; i < count; ++i) {
        frames.push_back(Frame(t, Rectangle{ (float)(x0 + i * w), (float)y0, (float)w, (float)h }));
    }
    return frames;
}

// The whole demo lives in its own function so every rad2d object (and the texture
// registry in particular) is destroyed *before* CloseWindow() in main(). The
// registry unloads its textures in its destructor via UnloadTexture, which needs a
// live GL context - so assets must outlive nothing past the window. This scoping is
// the standard raylib + RAII pattern.
static void runDemo() {
    // --- assets: one registry owns every texture and unloads them at shutdown ---
    TextureRegistry tex;
    tex.load(STARS,   "space_background.png");
    tex.load(SHIP,    "pointer_shooter.png");
    tex.load(ENEMY,   "bad_guy1.png");
    tex.load(TILESET, "example_tileset.png");
    tex.load(TEXTBOX, "textbox.png");
    tex.load(MOON,    "moon.png");

    // --- a pixel font for the Text drawable (the FontRegistry owns + unloads it) ---
    FontRegistry fonts;
    fonts.load(FONT_MAIN, "Early GameBoy.ttf");

    // --- the ONE shared animation registry (sprites, tiles, and UI all use it) ---
    AnimationRegistry anims;
    AnimRule loop(true, false, false); // isRepeating, returnToFirstFrame, pingPong
    anims.add(SHIP_FLY,  std::make_shared<Animation>("ship",  strip(0, 0, 16, 16, 5, 0.12f), loop));
    anims.add(ENEMY_FLY, std::make_shared<Animation>("enemy", strip(0, 0, 16, 16, 5, 0.15f), loop));
    anims.add(WATER,     std::make_shared<Animation>("water", strip(0, 32, 32, 32, 2, 0.35f), loop));
    // textbox: frame 0 is the plain box (idle), frames 0-2 bounce the "continue" arrow
    anims.add(BOX_IDLE,  std::make_shared<Animation>("box_idle", strip(0, 0, 96, 32, 1, 1.0f), loop));
    anims.add(BOX_BLINK, std::make_shared<Animation>("box",      strip(0, 0, 96, 32, 3, 0.40f), loop));

    // --- Background: tiling starfield that scrolls downward (scroll-only: no registry) ---
    Background space("space", nullptr, 0, 0, 0, 800, 480);
    space.setTexture(tex.get(STARS));
    space.setScrollSpeed(0.0f, 40.0f);

    // --- Sprite: the player ship, animated ---
    Sprite ship("ship", &anims, 360, 290, 0, 80, 80);
    ship.setTexture(tex.get(SHIP));
    ship.addAnimation(SHIP_FLY);
    ship.setAnimation(SHIP_FLY, true);

    // --- Sprites stored polymorphically as Drawable (the payoff of a virtual draw()) ---
    std::vector<std::unique_ptr<Drawable>> enemies;
    for (int i = 0; i < 3; ++i) {
        auto e = std::make_unique<Sprite>("enemy", &anims, 140 + i * 220, 50, 0, 64, 64);
        e->setTexture(tex.get(ENEMY));
        e->addAnimation(ENEMY_FLY);
        e->setAnimation(ENEMY_FLY, true);
        enemies.push_back(std::move(e)); // Sprite* -> Drawable*
    }

    // --- Tilemap: grass border with an animated-water interior ---
    TileSet tiles(&anims);
    tiles.defineTile(T_GRASS, tex.get(TILESET), Rectangle{ 0, 0, 32, 32 }); // 32x32 tiles
    tiles.defineAnimatedTile(T_WATER, tex.get(TILESET), WATER);

    Tilemap ground("ground", &tiles, 24, 150, 1, /*tileSize*/32);
    ground.setTiles({
        1, 1, 1, 1, 1, 1,
        1, 2, 2, 2, 2, 1,
        1, 2, 2, 2, 2, 1,
    }, /*cols*/6, /*rows*/3);

    // --- Text: a typewriter line; the reveal hook counts glyphs (where a blip would play) ---
    int revealed = 0;
    Text line("line", 300, 408, 10);
    line.setFont(fonts.get(FONT_MAIN));
    line.setColor(WHITE);
    line.setFontSize(20);
    line.setText("RAD-2D online.\nAll systems go.");
    line.enableTypewriter(22.0f);
    line.setEffect(TextEffect::WAVE);
    line.setEffectParams(2.0f, 9.0f);
    line.setOnReveal([&](int, int) { revealed++; });

    // --- UI: animated textbox (blinking arrow) + moon icon + the text line ---
    UI box("box", &anims, 160, 384, 10, 480, 80);
    box.setBackground(tex.get(TEXTBOX));
    box.addAnimation(BOX_IDLE);
    box.addAnimation(BOX_BLINK);
    box.setAnimation(BOX_IDLE, true); // plain box (no arrow) until the line finishes
    box.setIcon(tex.get(MOON));
    box.setIconOffset(20, 16);
    box.setIconSize(48, 48);
    box.setText(&line);

    // smoke-test hook: RAD2D_SMOKE=1 screenshots a couple of frames and exits
    const bool smoke = (std::getenv("RAD2D_SMOKE") != nullptr);
    int frame = 0;
    bool arrowShown = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) line.revealAll();          // skip the typewriter
        if (IsKeyPressed(KEY_R))     { line.restart(); revealed = 0; }

        // show the blinking "continue" arrow only once the line has fully typed out
        const bool done = line.isFinished();
        if (done && !arrowShown) { box.setAnimation(BOX_BLINK, true); arrowShown = true; }
        if (!done && arrowShown) { box.setAnimation(BOX_IDLE, true);  arrowShown = false; }

        tiles.update(); // advance animated tiles once per frame (shared + in sync)

        BeginDrawing();
            ClearBackground(BLACK);

            // draw order is back-to-front
            space.draw();                      // 1. scrolling background
            ground.draw();                     // 2. tile layer
            for (auto& d : enemies) d->draw();  // 3. sprites (via Drawable*)
            ship.draw();
            box.draw();                        // 4. UI: textbox -> icon -> text

            DrawText(TextFormat("glyphs revealed: %d", revealed), 12, 12, 16, RAYWHITE);
            DrawText("[SPACE] skip   [R] restart", 12, 458, 12, GRAY);
        EndDrawing();

        if (smoke) {
            if (frame == 50)  TakeScreenshot("rad2d_demo_a.png");
            if (frame == 130) { TakeScreenshot("rad2d_demo_b.png"); break; }
        }
        ++frame;
    }
}

int main() {
    InitWindow(800, 480, "RAD-2D - Drawable Tour");
    SetTargetFPS(60);

    runDemo(); // all assets/drawables are created and destroyed in here...

    CloseWindow(); // ...so textures are already unloaded before the GL context dies
    return 0;
}

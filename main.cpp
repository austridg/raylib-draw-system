#include "raylib.h"

#include "rad2d.hpp"

/*
=== === ===
EXAMPLE
A minimal demo of the animation library on top of raylib.

Loads one sprite sheet (test_anim.png, 64x64 frames) and plays a single
looping animation built from frames 0-2.
=== === ===
*/

enum TextureKey { SHEET };
enum AnimId { IDLE };

int main() {
    InitWindow(1080, 720, "Animation Library - Example");
    SetTargetFPS(60);

    // load the sprite sheet (the registry owns + unloads it for us)
    rad2d::TextureRegistry textures;
    textures.load(SHEET, "test_anim.png");

    // register a looping animation: frames 0-2, each shown for 0.3s
    // AnimRule(isRepeating, returnToFirstFrame, pingPong)
    rad2d::AnimationRegistry anims;
    anims.add(IDLE, std::make_shared<rad2d::Animation>(
        "idle",
        std::vector<rad2d::Frame>{
            rad2d::Frame(0.30f, Rectangle{   0, 0, 64, 64 }),
            rad2d::Frame(0.30f, Rectangle{  64, 0, 64, 64 }),
            rad2d::Frame(0.30f, Rectangle{ 128, 0, 64, 64 }),
        },
        rad2d::AnimRule(true, false, false)
    ));

    // build the sprite at (100,100), drawn at 256x256, and play the animation
    rad2d::Sprite player("player", &anims, 100, 100, 0, 256, 256);
    player.setTexture(textures.get(SHEET));
    player.addAnimation(IDLE);
    player.setAnimation(IDLE, true);

    // game loop: draw() advances the animation and renders the current frame
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            player.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
#include "raylib.h"

Texture2D sprite;

int main() {
  const int screenWidth = 1080;
  const int screenHeight = 720;

  InitWindow(screenWidth,screenHeight,"Animation Test");
  SetTargetFPS(60);

  sprite = LoadTexture("manrpg.png");

  while(!WindowShouldClose()) {
    BeginDrawing();

       DrawTexturePro(
        sprite,
        {16,0,16,16}, // cropping of image
        {10,10,500,500}, // position and size
        {0,0}, // origin
        0.0f, // rotation
        WHITE
      );

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
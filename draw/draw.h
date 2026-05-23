#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>

#include "sprite.h"

#include "raylib.h"

/*
=== Drawable ===
Drawables hold all information for drawing
elements on screen.

Each game element that's supposed to be displayed
on screen will hold a Drawable for defining how to
draw it and what animations it holds.
=== === === ===
*/
class Drawable {
private:
    std::string drawableName;
    int z; // layer variable (draw order)
    Sprite drawableSprite;
};

/*
=== Group ===

=== === ===
*/
struct Group {
    std::string groupName;
    int groupId;
    std::unordered_map<int,Drawable&> Drawables;

    Group(const std::string name);

    void addDrawable(const Drawable& drawable);
    void removeDrawable(int id);
};

/*
=== Draw System ===
Holds all Drawables and Drawable Groups
=== === === === ===
*/
class DrawSystem {
private:


public:

};
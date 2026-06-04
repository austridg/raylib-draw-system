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
    Texture2D texture;

    int positionX, positionY;
    int width, height;
    int z; // layer variable (draw order)

    Animations animationList;
public:


    void addAnimation(const Animation& animation);
};

/*
=== Group ===

=== === ===
*/
struct Group {
    std::string groupName;
    std::unordered_map<std::string,Drawable*> Drawables;

    Group(const std::string& name);

    void addDrawable(const Drawable& drawable);
    void removeDrawable(const std::string& name);
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
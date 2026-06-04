#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>
#include <memory>

#include "../animation/animation.h"

#include "raylib.h"

// TODO: Add type maps and asset registry
// std::unordered_map<std::string,std::shared_ptr<>>;

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

    int positionX, positionY, layerZ;
    int drawableWidth, drawableHeight;

    Animations animationList;
public:
    Drawable();
    Drawable(const std::string& name,int x,int y,int z,int width,int height);
    ~Drawable();

    void addAnimation(const Animation& animation);
};
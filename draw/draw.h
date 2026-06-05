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
public:
    Drawable(const std::string& name);
    Drawable(const std::string& name,int x,int y,int z,int width,int height);
    ~Drawable();
};

class Sprite : public Drawable {
private:
    std::unordered_map<int,std::shared_ptr<Animation>> animations;
    AnimationState animState;
public:
    Sprite(const std::string& name);
    Sprite(const std::string& name,int x,int y,int z,int width,int height);
    ~Sprite();

    void addAnimation(int localKey,std::shared_ptr<Animation> anim);

    void setAnimation(int localKey);
    void setAnimation(int localKey,bool play);
};

// TODO: Add mutational methods for coords and width/height
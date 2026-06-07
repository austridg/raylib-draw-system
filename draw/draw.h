#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>
#include <iostream>
#include <memory>

#include "../animation/animation.h"

#include "raylib.h"

namespace rad2d {

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
    virtual ~Drawable();

    void setPositionX(int x);
    void setPositionY(int y);
    void setLayerZ(int z);

    void setWidth(int width);
    void setHeight(int height);

    int getPositionX() const;
    int getPositionY() const;
    int getLayerZ() const;

    int getWidth() const;
    int getHeight() const;
};

class Sprite : public Drawable {
private:
    AnimationRegistry* registry;          // non-owning: central DB this sprite pulls animations from
    std::unordered_set<int> usableAnimations; // registry ids this sprite is allowed to use
    AnimationState animState;
    std::shared_ptr<Texture2D> texture;   // sprite sheet the animations crop from
public:
    Sprite(const std::string& name,AnimationRegistry* reg);
    Sprite(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height);
    ~Sprite();

    void setTexture(std::shared_ptr<Texture2D> tex);

    // declare that this sprite may use the animation stored under `registryId` in the registry
    void addAnimation(int registryId);

    // make a usable animation active (pulled from the registry). returns false if it
    // isn't in this sprite's usable list or isn't in the registry
    bool setAnimation(int registryId);
    void setAnimation(int registryId,bool play);

    // playback controls (pass-throughs to the internal AnimationState)
    void play();
    void stop();
    void stop(bool resetIdx);

    // advances the active animation and draws the current frame to the screen
    void draw();
};

} // namespace rad2d
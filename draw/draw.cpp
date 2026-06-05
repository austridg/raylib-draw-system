#include "draw.h"

/*
=== === ===
DRAWABLE
=== === ===
*/
Drawable::Drawable(const std::string& name)
    : drawableName(name),positionX(0), positionY(0), layerZ(0),drawableWidth(0), drawableHeight(0) {}

Drawable::Drawable(const std::string& name,int x,int y,int z,int width,int height)
    : drawableName(name),positionX(x), positionY(y), layerZ(z),drawableWidth(width), drawableHeight(height) {}

Drawable::~Drawable() {}

/*
=== === ===
SPRITE
=== === ===
*/
Sprite::Sprite(const std::string& name)
    : Drawable(name) {}

Sprite::Sprite(const std::string& name,int x,int y,int z,int width,int height)
    : Drawable(name,x,y,z,width,height) {}

Sprite::~Sprite() {}

void Sprite::addAnimation(int localKey,std::shared_ptr<Animation> anim) {
    animations[localKey] = std::move(anim);
}

void Sprite::setAnimation(int localKey) {
    auto it = animations.find(localKey);
    if(it == animations.end()) { printf("No valid animation found."); return; }

    animState.set(it->second);
}

void Sprite::setAnimation(int localKey,bool play) {
    auto it = animations.find(localKey);
    if(it == animations.end()) { printf("No valid animation found."); return; }

    animState.set(it->second);
    if(play) { animState.play(); }
}
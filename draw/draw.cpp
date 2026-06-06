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

void Drawable::setPositionX(int x) { positionX = x; }
void Drawable::setPositionY(int y) { positionY = y; }
void Drawable::setLayerZ(int z) { layerZ = z; }
void Drawable::setWidth(int width) { drawableWidth = width; }
void Drawable::setHeight(int height) { drawableHeight = height; }

int Drawable::getPositionX() const { return positionX; }
int Drawable::getPositionY() const { return positionY; }
int Drawable::getLayerZ() const { return layerZ; }
int Drawable::getWidth() const { return drawableWidth; }
int Drawable::getHeight() const { return drawableHeight; }

/*
=== === ===
SPRITE
=== === ===
*/
Sprite::Sprite(const std::string& name,AnimationRegistry* reg)
    : Drawable(name),registry(reg) {}

Sprite::Sprite(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height)
    : Drawable(name,x,y,z,width,height),registry(reg) {}

Sprite::~Sprite() {}

void Sprite::setTexture(std::shared_ptr<Texture2D> tex) {
    texture = std::move(tex);
}

void Sprite::addAnimation(int registryId) {
    // just record that this sprite is allowed to use this registry animation.
    // the actual Animation lives in the registry and is fetched on setAnimation()
    usableAnimations.insert(registryId);
}

bool Sprite::setAnimation(int registryId) {
    // the sprite must have declared this animation as usable
    if(usableAnimations.find(registryId) == usableAnimations.end()) {
        std::cout << rang::fg::red << "[ERROR] - Animation id " << registryId << " not in sprite's usable list" << rang::style::reset << std::endl;
        return false;
    }

    // need a registry to pull the actual animation from
    if(!registry) {
        std::cout << rang::fg::red << "[ERROR] - Sprite can't access the animation registry" << rang::style::reset << std::endl;
        return false;
    }

    // fetch the animation from the central registry (logs + returns null if missing)
    std::shared_ptr<Animation> anim = registry->get(registryId);
    if(!anim) { return false; }

    animState.set(anim);
    return true;
}

void Sprite::setAnimation(int registryId,bool play) {
    // only start playing if the animation was actually set
    if(setAnimation(registryId) && play) { animState.play(); }
}

void Sprite::play() { animState.play(); }
void Sprite::stop() { animState.stop(); }
void Sprite::stop(bool resetIdx) { animState.stop(resetIdx); }

void Sprite::draw() {
    // no texture means there's nothing to crop a frame out of
    if(!texture) {
        std::cout << rang::fg::red << "[ERROR] - No texture set on sprite" << rang::style::reset << std::endl;
        return;
    }

    // getSource() advances the animation (per elapsed time) and hands back the
    // current frame's crop rectangle into the sprite sheet
    Rectangle source = animState.getSource();

    // where on screen to draw, using this drawable's position and size
    Rectangle dest = {
        (float)getPositionX(),
        (float)getPositionY(),
        (float)getWidth(),
        (float)getHeight()
    };

    DrawTexturePro(*texture, source, dest, {0,0}, 0.0f, WHITE);
}
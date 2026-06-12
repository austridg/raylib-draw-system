#include "draw.h"

#include <cmath> // fmodf

namespace rad2d {

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
ANIMATED DRAWABLE
=== === ===
*/
AnimatedDrawable::AnimatedDrawable(const std::string& name,AnimationRegistry* reg)
    : Drawable(name),registry(reg) {}

AnimatedDrawable::AnimatedDrawable(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height)
    : Drawable(name,x,y,z,width,height),registry(reg) {}

void AnimatedDrawable::addAnimation(int registryId) {
    // just record that this drawable is allowed to use this registry animation.
    // the actual Animation lives in the registry and is fetched on setAnimation()
    usableAnimations.insert(registryId);
}

bool AnimatedDrawable::setAnimation(int registryId) {
    // the drawable must have declared this animation as usable
    if(usableAnimations.find(registryId) == usableAnimations.end()) {
        std::cerr << "[ERROR] - Animation id " << registryId << " not in drawable's usable list" << std::endl;
        return false;
    }

    // need a registry to pull the actual animation from
    if(!registry) {
        std::cerr << "[ERROR] - Drawable can't access the animation registry" << std::endl;
        return false;
    }

    // fetch the animation from the central registry (logs + returns null if missing)
    std::shared_ptr<Animation> anim = registry->get(registryId);
    if(!anim) { return false; }

    animState.set(anim);
    return true;
}

void AnimatedDrawable::setAnimation(int registryId,bool play) {
    // only start playing if the animation was actually set
    if(setAnimation(registryId) && play) { animState.play(); }
}

void AnimatedDrawable::play() { animState.play(); }
void AnimatedDrawable::stop() { animState.stop(); }
void AnimatedDrawable::stop(bool resetIdx) { animState.stop(resetIdx); }

/*
=== === ===
SPRITE
=== === ===
*/
Sprite::Sprite(const std::string& name,AnimationRegistry* reg)
    : AnimatedDrawable(name,reg) {}

Sprite::Sprite(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height)
    : AnimatedDrawable(name,reg,x,y,z,width,height) {}

Sprite::~Sprite() {}

void Sprite::setTexture(std::shared_ptr<Texture2D> tex) {
    texture = std::move(tex);
}

void Sprite::draw() {
    // no texture means there's nothing to crop a frame out of
    if(!texture) {
        std::cerr << "[ERROR] - No texture set on sprite" << std::endl;
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

/*
=== === ===
BACKGROUND
=== === ===
*/
Background::Background(const std::string& name,AnimationRegistry* reg)
    : AnimatedDrawable(name,reg),scrollX(0.f), scrollY(0.f), speedX(0.f), speedY(0.f) {}

Background::Background(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height)
    : AnimatedDrawable(name,reg,x,y,z,width,height),scrollX(0.f), scrollY(0.f), speedX(0.f), speedY(0.f) {}

Background::~Background() {}

void Background::setTexture(std::shared_ptr<Texture2D> tex) {
    texture = std::move(tex);

    // REPEAT wrapping is what lets a source rectangle larger than the texture tile
    // automatically - that's the whole trick behind the seamless looping scroll
    if(texture) { SetTextureWrap(*texture, TEXTURE_WRAP_REPEAT); }
}

void Background::setScrollSpeed(float vx, float vy) {
    speedX = vx;
    speedY = vy;
}

void Background::draw() {
    // nothing to draw without a texture
    if(!texture) {
        std::cerr << "[ERROR] - No texture set on background" << std::endl;
        return;
    }

    Rectangle dest = {
        (float)getPositionX(),
        (float)getPositionY(),
        (float)getWidth(),
        (float)getHeight()
    };

    // ANIMATED MODE: an active animation supplies the current frame, drawn across the
    // whole area. its frame rect stays inside the texture, so REPEAT never kicks in.
    if(animState.activeAnimation) {
        Rectangle frame = animState.getSource();
        DrawTexturePro(*texture, frame, dest, {0,0}, 0.0f, WHITE);
        return;
    }

    // SCROLL MODE: slide a screen-sized window across the texture; REPEAT wrapping
    // tiles it so the loop is seamless (one draw call, infinite scroll).
    scrollX += speedX * GetFrameTime();
    scrollY += speedY * GetFrameTime();

    // keep the offset bounded to one tile; the image repeats every texture width/height
    // so wrapping here is visually identical but stops the float from growing forever
    scrollX = fmodf(scrollX, (float)texture->width);
    scrollY = fmodf(scrollY, (float)texture->height);

    // a source rectangle the size of the on-screen area, offset by the scroll. because
    // the texture wraps, asking for a region past the edge tiles it to fill the dest.
    Rectangle source = {
        scrollX,
        scrollY,
        (float)getWidth(),
        (float)getHeight()
    };

    DrawTexturePro(*texture, source, dest, {0,0}, 0.0f, WHITE);
}

/*
=== === ===
TEXT
=== === ===
*/
namespace {
    // small clamp helper for the fade factors
    float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }
}

Text::Text(const std::string& name)
    : Drawable(name),
      fontSize(20.f), spacing(1.f), color(BLACK),
      typewriter(false), charsPerSecond(20.f), typeAccumulator(0.f), visibleChars(0),
      effect(TextEffect::NONE), effectTime(0.f), effectAmplitude(2.f), effectSpeed(8.f), fadeDuration(0.5f),
      announcedChars(0) {}

Text::Text(const std::string& name,int x,int y,int z)
    : Drawable(name,x,y,z,0,0),
      fontSize(20.f), spacing(1.f), color(BLACK),
      typewriter(false), charsPerSecond(20.f), typeAccumulator(0.f), visibleChars(0),
      effect(TextEffect::NONE), effectTime(0.f), effectAmplitude(2.f), effectSpeed(8.f), fadeDuration(0.5f),
      announcedChars(0) {}

void Text::setFont(std::shared_ptr<Font> f) { texFont = std::move(f); }

void Text::setText(const std::string& s) {
    text = s;
    restart(); // new string -> start the reveal over
}

void Text::setFontSize(float size) { fontSize = size; }
void Text::setSpacing(float s) { spacing = s; }
void Text::setColor(Color c) { color = c; }

void Text::enableTypewriter(float cps) {
    typewriter = true;
    charsPerSecond = cps;
    restart();
}

void Text::disableTypewriter() { typewriter = false; }

void Text::restart() {
    visibleChars = 0;
    typeAccumulator = 0.f;
    effectTime = 0.f;
    announcedChars = 0;
}

void Text::revealAll() {
    // jump to the end without firing a burst of reveal callbacks
    visibleChars = GetCodepointCount(text.c_str());
    announcedChars = visibleChars;
}

bool Text::isFinished() const {
    return visibleChars >= GetCodepointCount(text.c_str());
}

void Text::setEffect(TextEffect e) {
    effect = e;
    effectTime = 0.f; // restart the effect clock so fades begin from the top
}

void Text::setEffectParams(float amplitude,float speed) {
    effectAmplitude = amplitude;
    effectSpeed = speed;
}

void Text::setFadeDuration(float seconds) { fadeDuration = seconds; }

void Text::setOnReveal(std::function<void(int,int)> cb) { onReveal = std::move(cb); }

void Text::draw() {
    // fall back to raylib's built-in font when none was supplied
    Font useFont = texFont ? *texFont : GetFontDefault();

    const int totalGlyphs = GetCodepointCount(text.c_str());

    // advance both clocks by this frame's elapsed time
    const float dt = GetFrameTime();
    effectTime += dt;

    // reveal more glyphs if the typewriter is running, otherwise show everything
    if(typewriter) {
        if(visibleChars < totalGlyphs && charsPerSecond > 0.f) {
            typeAccumulator += dt;
            const float perChar = 1.f / charsPerSecond;

            // drain the accumulator so a lag spike can reveal several glyphs at once
            while(typeAccumulator >= perChar && visibleChars < totalGlyphs) {
                typeAccumulator -= perChar;
                visibleChars++;
            }
        }
    } else {
        visibleChars = totalGlyphs;
    }

    // whole-string alpha factor for the fade effects (1 = fully opaque)
    float fade = 1.f;
    if(effect == TextEffect::FADE_IN) {
        fade = fadeDuration > 0.f ? clamp01(effectTime / fadeDuration) : 1.f;
    } else if(effect == TextEffect::FADE_OUT) {
        fade = fadeDuration > 0.f ? clamp01(1.f - effectTime / fadeDuration) : 0.f;
    }

    // per-glyph layout, mirroring how raylib's DrawTextEx walks a string
    const float scaleFactor = fontSize / useFont.baseSize;
    const Vector2 origin = { (float)getPositionX(), (float)getPositionY() };
    float penX = 0.f; // horizontal advance within the current line
    float penY = 0.f; // vertical offset for line breaks

    const char* t = text.c_str();
    const int byteLen = (int)text.length();
    int byteOfs = 0;
    int g = 0; // glyph counter (matches GetCodepointCount, newlines included)

    while(byteOfs < byteLen) {
        int cpBytes = 0;
        const int codepoint = GetCodepointNext(&t[byteOfs], &cpBytes);
        const int idx = GetGlyphIndex(useFont, codepoint);
        byteOfs += cpBytes;

        // stop once we've drawn every glyph the typewriter has revealed so far
        if(g >= visibleChars) { break; }

        // announce each glyph the first time it becomes visible (e.g. blip sound)
        if(onReveal && g >= announcedChars) { onReveal(g, codepoint); }

        if(codepoint == '\n') {
            penY += fontSize + 2.f; // simple single-line leading
            penX = 0.f;
            g++;
            continue;
        }

        // this glyph's pen position before any effect displacement
        float gx = origin.x + penX;
        float gy = origin.y + penY;
        Color gc = color;

        // per-glyph effect displacement / tint
        if(effect == TextEffect::SHAKE) {
            gx += (float)GetRandomValue(-100,100) / 100.f * effectAmplitude;
            gy += (float)GetRandomValue(-100,100) / 100.f * effectAmplitude;
        } else if(effect == TextEffect::WAVE) {
            gy += sinf(effectTime * effectSpeed + (float)g * 0.5f) * effectAmplitude;
        } else if(effect == TextEffect::FADE_IN || effect == TextEffect::FADE_OUT) {
            gc.a = (unsigned char)((float)color.a * fade);
        }

        DrawTextCodepoint(useFont, codepoint, { gx, gy }, fontSize, gc);

        // advance the pen by this glyph's width (some fonts use width, some advanceX)
        if(useFont.glyphs[idx].advanceX == 0) {
            penX += (float)useFont.recs[idx].width * scaleFactor + spacing;
        } else {
            penX += (float)useFont.glyphs[idx].advanceX * scaleFactor + spacing;
        }

        g++;
    }

    // every glyph up through visibleChars has now been announced
    announcedChars = visibleChars;
}

/*
=== === ===
UI
=== === ===
*/
UI::UI(const std::string& name,AnimationRegistry* reg)
    : AnimatedDrawable(name,reg),iconX(0), iconY(0), iconW(0), iconH(0),text(nullptr) {}

UI::UI(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height)
    : AnimatedDrawable(name,reg,x,y,z,width,height),iconX(0), iconY(0), iconW(0), iconH(0),text(nullptr) {}

UI::~UI() {}

void UI::setBackground(std::shared_ptr<Texture2D> tex) { background = std::move(tex); }

void UI::setIcon(std::shared_ptr<Texture2D> tex) {
    icon = std::move(tex);

    // default the icon's draw size to its native size if the caller hasn't set one
    if(icon && iconW == 0 && iconH == 0) {
        iconW = icon->width;
        iconH = icon->height;
    }
}

void UI::setIconOffset(int x,int y) { iconX = x; iconY = y; }
void UI::setIconSize(int w,int h) { iconW = w; iconH = h; }

void UI::setText(Text* t) { text = t; }

// addAnimation / setAnimation / play / stop are inherited from AnimatedDrawable.

void UI::draw() {
    // --- background ---
    if(background) {
        // crop the current animation frame if one is active, else use the whole texture
        Rectangle src;
        if(animState.activeAnimation) {
            src = animState.getSource();
        } else {
            src = { 0.f, 0.f, (float)background->width, (float)background->height };
        }

        Rectangle dest = {
            (float)getPositionX(),
            (float)getPositionY(),
            (float)getWidth(),
            (float)getHeight()
        };
        DrawTexturePro(*background, src, dest, {0,0}, 0.0f, WHITE);
    }

    // --- icon (positioned relative to the element's origin) ---
    if(icon) {
        Rectangle src = { 0.f, 0.f, (float)icon->width, (float)icon->height };
        Rectangle dest = {
            (float)(getPositionX() + iconX),
            (float)(getPositionY() + iconY),
            (float)iconW,
            (float)iconH
        };
        DrawTexturePro(*icon, src, dest, {0,0}, 0.0f, WHITE);
    }

    // --- text (external drawable; it positions and advances itself) ---
    if(text) { text->draw(); }
}

} // namespace rad2d
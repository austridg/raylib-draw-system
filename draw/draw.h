#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>
#include <iostream>
#include <memory>
#include <functional>

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

    // every drawable knows how to render itself. pure virtual so Drawables can be
    // stored and drawn polymorphically, e.g. std::vector<std::unique_ptr<Drawable>>.
    virtual void draw() = 0;

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

/*
=== AnimatedDrawable ===
Shared base for any Drawable that plays animations out of the central
AnimationRegistry: it owns the registry pointer, the set of animation ids it's
allowed to use, and the AnimationState that tracks playback. Sprite, UI, and
Background all inherit it so this plumbing lives in exactly one place.
=== === === ===
*/
class AnimatedDrawable : public Drawable {
protected:
    AnimationRegistry* registry;              // non-owning: shared central animation DB
    std::unordered_set<int> usableAnimations; // registry ids this drawable may use
    AnimationState animState;                 // current playback state
public:
    AnimatedDrawable(const std::string& name,AnimationRegistry* reg);
    AnimatedDrawable(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height);

    // declare that this drawable may use the animation stored under `registryId`
    void addAnimation(int registryId);

    // make a usable animation active (pulled from the registry). returns false if it
    // isn't in this drawable's usable list or isn't in the registry
    bool setAnimation(int registryId);
    void setAnimation(int registryId,bool play);

    // playback controls (pass-throughs to the internal AnimationState)
    void play();
    void stop();
    void stop(bool resetIdx);
};

class Sprite : public AnimatedDrawable {
private:
    std::shared_ptr<Texture2D> texture;   // sprite sheet the animations crop from
public:
    Sprite(const std::string& name,AnimationRegistry* reg);
    Sprite(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height);
    ~Sprite();

    void setTexture(std::shared_ptr<Texture2D> tex);

    // advances the active animation and draws the current frame to the screen
    void draw() override;
};

/*
=== Background ===
A full-area Drawable with two alternative modes:
  - SCROLL: a single tileable texture slid via REPEAT wrapping for a seamless loop.
  - ANIMATED: when an animation from the registry is active, its current frame is
    drawn across the area instead (frame animation and wrap-scroll don't compose,
    since GPU wrap tiles the whole texture, not a single sheet frame).
Pass a registry to animate; pass nullptr if you only ever scroll.
=== === === ===
*/
class Background : public AnimatedDrawable {
private:
    std::shared_ptr<Texture2D> texture; // tileable image to scroll, or sheet to animate
    float scrollX, scrollY;             // current scroll offset into the texture (px)
    float speedX, speedY;               // scroll velocity (px per second)
public:
    Background(const std::string& name,AnimationRegistry* reg);
    Background(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height);
    ~Background();

    // setting the texture also flips it to REPEAT wrapping so the tiling trick works
    void setTexture(std::shared_ptr<Texture2D> tex);

    // how fast (px/sec) the background scrolls; negative reverses direction
    void setScrollSpeed(float vx, float vy);

    // draws the current animation frame if one is active, else the tiled scrolling image
    void draw() override;
};

/*
=== Text ===
A Drawable that renders a string glyph-by-glyph, which is what lets it support a
typewriter reveal and per-glyph effects (shake / wave / fade). It is intentionally
just the *rendering* half of a text system: a higher-level dialogue engine can sit
on top and drive these knobs (markup, voice blips, scripted pauses, box advance).
=== === === ===
*/
enum class TextEffect {
    NONE,
    SHAKE,    // each glyph jitters around its position
    WAVE,     // glyphs ride a sine wave (phase offset per glyph)
    FADE_IN,  // whole string ramps from transparent to opaque
    FADE_OUT  // whole string ramps from opaque to transparent
};

class Text : public Drawable {
private:
    std::shared_ptr<Font> texFont; // glyph atlas; null falls back to raylib's default font
    std::string text;
    float fontSize;
    float spacing;                 // extra px between glyphs
    Color color;

    // --- typewriter ---
    bool typewriter;               // when true, glyphs are revealed over time
    float charsPerSecond;          // reveal speed
    float typeAccumulator;         // time banked toward revealing the next glyph
    int visibleChars;              // how many glyphs are currently shown

    // --- effects ---
    TextEffect effect;
    float effectTime;              // self-contained clock driving wave/fade
    float effectAmplitude;         // px of displacement for shake / wave
    float effectSpeed;             // wave angular speed (rad/sec)
    float fadeDuration;            // seconds a fade in/out takes

    // optional per-glyph reveal hook (e.g. play a blip sound); audio lives in your game
    std::function<void(int,int)> onReveal;
    int announcedChars;            // how many glyphs onReveal has already fired for
public:
    Text(const std::string& name);
    Text(const std::string& name,int x,int y,int z);

    void setFont(std::shared_ptr<Font> f);
    void setText(const std::string& s); // replaces the string and restarts the reveal
    void setFontSize(float size);
    void setSpacing(float s);
    void setColor(Color c);

    // --- typewriter controls ---
    void enableTypewriter(float cps);   // reveal glyphs at `cps` characters/second
    void disableTypewriter();           // show the whole string at once
    void restart();                     // replay the reveal from the first glyph
    void revealAll();                   // skip straight to fully shown
    bool isFinished() const;            // true once every glyph is visible

    // --- effect controls ---
    void setEffect(TextEffect e);
    void setEffectParams(float amplitude,float speed); // tune shake/wave intensity
    void setFadeDuration(float seconds);

    // fired the first time each glyph appears (glyph index, codepoint). wire a
    // per-letter "blip" sound here - rad2d itself stays out of audio.
    void setOnReveal(std::function<void(int glyphIndex,int codepoint)> cb);

    // advances the typewriter + effect clocks and draws the currently-visible glyphs
    void draw() override;
};

/*
=== UI ===
A composite Drawable for HUD / menu elements: an (optionally animated) background
texture, an optional icon, and an optional Text drawable. Any piece may be left
unset. Like Sprite, it animates its background by pulling from the shared central
AnimationRegistry, so a UI element and a sprite can play the very same animation.
=== === === ===
*/
class UI : public AnimatedDrawable {
private:
    std::shared_ptr<Texture2D> background;    // the panel background (the animated surface)
    std::shared_ptr<Texture2D> icon;          // optional icon, drawn over the background
    int iconX, iconY, iconW, iconH;           // icon placement/size, relative to this element

    Text* text;                               // optional, non-owning; positions itself
public:
    UI(const std::string& name,AnimationRegistry* reg);
    UI(const std::string& name,AnimationRegistry* reg,int x,int y,int z,int width,int height);
    ~UI();

    void setBackground(std::shared_ptr<Texture2D> tex);

    void setIcon(std::shared_ptr<Texture2D> tex);
    void setIconOffset(int x,int y);          // icon position relative to the element's origin
    void setIconSize(int w,int h);

    void setText(Text* t);                    // attach an external Text (not owned by the UI)

    // animation controls (addAnimation / setAnimation / play / stop) come from
    // AnimatedDrawable and animate the background surface.

    // draws background, then icon, then the attached text (whichever are set)
    void draw() override;
};

} // namespace rad2d
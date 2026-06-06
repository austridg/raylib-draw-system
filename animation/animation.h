#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>
#include <memory>
#include "rang.hpp"

#include "raylib.h"

struct Frame {
    float time; // 1 = one second
    Rectangle source;

    Frame(float t,Rectangle s);
};

// config for animation rules
struct AnimRule {
  bool isRepeating;
  bool returnToFirstFrame;
  bool pingPong;

  AnimRule();
  AnimRule(bool repeat,bool returnFirst,bool pp);
};

class Animation {
private:
    std::string name;
    std::vector<Frame> frames;
public:
    AnimRule rules;

    Animation(std::string anim_name,const std::vector<Frame>& f,const AnimRule& anim_rule);
    const std::vector<Frame>& getFrames() const;
};

class AnimationRegistry {
private:
    std::unordered_map<int,std::shared_ptr<Animation>> registry;
public:
    void add(int id,std::shared_ptr<Animation> anim);
    std::shared_ptr<Animation> get(int id);
};

/*
=== === ===
Holds information about a drawable's active animation
=== === ===
*/
struct AnimationState {
    std::shared_ptr<Animation> activeAnimation;
    int frameIdx;
    float accumulator;
    bool isPlaying;
    int step; // direction of animation

    AnimationState();

    void play();

    void stop();
    void stop(bool resetIdx);

    void set(std::shared_ptr<Animation> anim);
    void reset();

    Rectangle getSource();
};
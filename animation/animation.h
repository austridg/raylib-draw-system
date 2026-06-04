#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>

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

  AnimRule(bool repeat);
};

class Animation; // forward declaration because they're cool

// holds all animations
struct Animations {
    int total;
    std::unordered_map<std::string,Animation*> index;

    void addAnimation(Animation& animation);
    Animation* getAnimation(const std::string& anim_name); // NOTE: find way to use ints instead of string

    // TODO: Add constructor
};

class Animation {
private:
    std::string name;
    int id; // may not keep
    std::vector<Frame> frames;
    bool isPlaying;

    int currentFrameIdx;

    float animAccumulator;
    AnimRule rules;
public:

    Animation(std::string anim_name,const std::vector<Frame> f,AnimRule& anim_rule);

    const std::string& getName();
    void createId(int anim_id);

    void addFrame(Frame f);
    Rectangle getSource();
};
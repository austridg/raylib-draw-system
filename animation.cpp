#include "animation.h"

Frame::Frame(float t,Rectangle s)
    : time(t), source(s) {}

AnimRule::AnimRule(bool repeat)
    : isRepeating(repeat) {}

void Animations::addAnimation(Animation& animation) {
    animation.createId(index.size()+1);
    index.emplace(animation.getName(),animation);
}

Animation* Animations::getAnimation(const std::string& anim_name) {
    if(!index.count(anim_name)) { printf("Animation not found."); }
    return index.at(anim_name);
}

Animation::Animation(std::string anim_name,const std::vector<Frame> f,const std::string& texture_path,AnimRule& anim_rule)
    : name(anim_name),frames(f),id(-1),texturePath(texture_path),rules(anim_rule), isPlaying(true),currentFrameIdx(0),animAccumulator(0.f) {}

const std::string& Animation::getName() { return name; }

void Animation::createId(int anim_id) { id = anim_id; }

void Animation::addFrame(Frame f) { frames.push_back(f); }

Rectangle Animation::getSource() {
    if(!isPlaying) { return frames.at(currentFrameIdx).source; }

    animAccumulator += GetFrameTime();

    if(animAccumulator >= frames.at(currentFrameIdx).time) {
        animAccumulator -= frames.at(currentFrameIdx).time;
        if(currentFrameIdx == frames.size() - 1) {
            if(rules.isRepeating) { currentFrameIdx = 0; }
            else {
                isPlaying = false;
                if(rules.returnToFirstFrame) { currentFrameIdx = 0; }
            }
        }
        else { currentFrameIdx++; }
    }

    return frames.at(currentFrameIdx).source;
}
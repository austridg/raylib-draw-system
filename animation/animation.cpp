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
    : name(anim_name),frames(f),id(-1),texturePath(texture_path),rules(anim_rule), isPlaying(false),currentFrameIdx(0),animAccumulator(0.f) {}

const std::string& Animation::getName() { return name; }

void Animation::createId(int anim_id) { id = anim_id; } // may not keep

void Animation::addFrame(Frame f) { frames.push_back(f); }

Rectangle Animation::getSource() {
    // if animation is not playing then just return current frame with no incrementation
    if(!isPlaying) { return frames.at(currentFrameIdx).source; }

    // accumulate delta time
    animAccumulator += GetFrameTime();

    // check if accumulator is greater than or equal to frame time
    if(animAccumulator >= frames.at(currentFrameIdx).time) {
        
        // reset accumulator
        animAccumulator -= frames.at(currentFrameIdx).time;

        // check if on last frame in the animation
        if(currentFrameIdx == frames.size() - 1) {
            
            // check if animation is supposed to repeat - if so then go back to first frame
            if(rules.isRepeating) { currentFrameIdx = 0; }

            // if animation isn't repeating, stop playing animation
            else {
                isPlaying = false;

                // check if animation is supposed to return to first frame when finished
                if(rules.returnToFirstFrame) { currentFrameIdx = 0; }
            }
        }
        // increment frame index
        else { currentFrameIdx++; }
    }

    // return current frame
    return frames.at(currentFrameIdx).source;
}
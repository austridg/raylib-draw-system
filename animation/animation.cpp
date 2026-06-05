#include "animation.h"

/*
=== === ===
FRAME
=== === ===
*/

Frame::Frame(float t,Rectangle s)
    : time(t), source(s) {}

/*
=== === ===
ANIMRULE
=== === ===
*/
AnimRule::AnimRule(bool repeat)
    : isRepeating(repeat) {}

/*
=== === ===
ANIMATION
=== === ===
*/
Animation::Animation(std::string anim_name,const std::vector<Frame>& f,AnimRule& anim_rule)
    : name(anim_name),frames(f),rules(anim_rule) {}

/*
=== === ===
ANIMATION REGISTRY
=== === ===
*/
// TODO: Add function

std::shared_ptr<Animation> AnimationRegistry::get(int id) {
    return registry.at(id);
}

/*
=== === ===
ANIMATION STATE
=== === ===
*/

AnimationState::AnimationState()
    : activeAnimation(nullptr),frameIdx(0),accumulator(0.f),isPlaying(false) {}

void AnimationState::play() { isPlaying = true; }

void AnimationState::stop() { 
    isPlaying = false; 
}

void AnimationState::stop(bool resetIdx) {
    if(resetIdx) { frameIdx = 0; }
    isPlaying = false;
}

void AnimationState::set(std::shared_ptr<Animation> anim) { 
    if(anim == activeAnimation) { printf("Animation already set."); return; }
    activeAnimation = std::move(anim);
    frameIdx = 0;
    accumulator = 0.f;
}

void AnimationState::reset() { activeAnimation = nullptr; }

Rectangle AnimationState::getSource() {
    const std::vector<Frame>& frames = activeAnimation->getFrames();

    // if animation is not playing then just return current frame with no incrementation
    if(!isPlaying) { return frames.at(frameIdx).source; }

    // accumulate delta time
    accumulator += GetFrameTime();

    // check if accumulator is greater than or equal to frame time
    if(accumulator >= frames.at(frameIdx).time) {
        
        // reset accumulator
        accumulator -= frames.at(frameIdx).time;
        
        // check if on last frame in the animation
        if(frameIdx == frames.size() - 1) {
            
            // check if animation is supposed to repeat - if so then go back to first frame
            if(activeAnimation->rules.isRepeating) { frameIdx = 0; }

            // if animation isn't repeating, stop playing animation
            else {
                isPlaying = false;

                // check if animation is supposed to return to first frame when finished
                if(activeAnimation->rules.returnToFirstFrame) { frameIdx = 0; }
            }
        }
        // increment frame index
        else { frameIdx++; }
    }

    // return current frame
    return frames.at(frameIdx).source;
}
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
AnimRule::AnimRule() : isRepeating(false), returnToFirstFrame(false) {}

AnimRule::AnimRule(bool repeat,bool returnFirst,bool inAndDe)
    : isRepeating(repeat), returnToFirstFrame(returnFirst), incrementAndDecrement(inAndDe) {}

/*
=== === ===
ANIMATION
=== === ===
*/
Animation::Animation(std::string anim_name,const std::vector<Frame>& f,const AnimRule& anim_rule)
    : name(anim_name),frames(f),rules(anim_rule) {}

const std::vector<Frame>& Animation::getFrames() const { return frames; }
/*
=== === ===
ANIMATION REGISTRY
=== === ===
*/
// TODO: Add function
void AnimationRegistry::add(int id,std::shared_ptr<Animation> anim) {
    registry[id] = std::move(anim);
}

std::shared_ptr<Animation> AnimationRegistry::get(int id) {
    return registry.at(id);
}

/*
=== === ===
ANIMATION STATE
=== === ===
*/

AnimationState::AnimationState()
    : activeAnimation(nullptr),frameIdx(0),accumulator(0.f),isPlaying(false), reachedEndOfFrames(false) {}

void AnimationState::play() { isPlaying = true; }

void AnimationState::stop() { 
    isPlaying = false;
}

void AnimationState::stop(bool resetIdx) {
    if(resetIdx) { frameIdx = 0; }
    isPlaying = false;
}

void AnimationState::set(std::shared_ptr<Animation> anim) { 
    if(anim == activeAnimation) {
        std::cout << rang::fg::yellow << "[WARNING] - Animation is already active animation" << rang::style::reset << std::endl;
        return; 
    }
    activeAnimation = std::move(anim);
    frameIdx = 0;
    accumulator = 0.f;
}

void AnimationState::reset() { 
    activeAnimation = nullptr;
    frameIdx = 0;
    accumulator = 0.f;
    isPlaying = false;
}

// TODO: Needs refactoring for incrementAndDecrement bool
Rectangle AnimationState::getSource() {
    if(!activeAnimation) { 
        std::cout << rang::fg::red << "[ERROR] - No animation set" << rang::style::reset << std::endl; 
        return Rectangle{0,0,0,0};
    }

    const std::vector<Frame>& frames = activeAnimation->getFrames();

    // if animation is not playing then just return current frame with no incrementation
    if(!isPlaying) { return frames.at(frameIdx).source; }

    // accumulate delta time
    accumulator += GetFrameTime();

    // check if accumulator is greater than or equal to frame time
    if(accumulator >= frames.at(frameIdx).time) {
        
        // reset accumulator
        accumulator -= frames.at(frameIdx).time;

        // check if incrementAndDecrement is true
        if(activeAnimation->rules.incrementAndDecrement) {}
        
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
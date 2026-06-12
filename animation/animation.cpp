#include "animation.h"

namespace rad2d {

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
AnimRule::AnimRule() : isRepeating(false), returnToFirstFrame(false), pingPong(false) {}

AnimRule::AnimRule(bool repeat,bool returnFirst,bool pp)
    : isRepeating(repeat), returnToFirstFrame(returnFirst), pingPong(pp) {}

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
void AnimationRegistry::add(int id,std::shared_ptr<Animation> anim) {
    registry[id] = std::move(anim);
}

std::shared_ptr<Animation> AnimationRegistry::get(int id) {
    auto it = registry.find(id);

    // not finding an animation isn't fatal - log it and let the caller decide
    if(it == registry.end()) {
        std::cerr << "[ERROR] - No animation with id " << id << " in registry" << std::endl;
        return nullptr;
    }

    return it->second;
}

/*
=== === ===
ANIMATION STATE
=== === ===
*/

AnimationState::AnimationState()
    : activeAnimation(nullptr),frameIdx(0),accumulator(0.f),isPlaying(false), step(1) {}

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
        std::cerr << "[WARNING] - Animation is already active animation" << std::endl;
        return;
    }
    activeAnimation = std::move(anim);
    frameIdx = 0;
    accumulator = 0.f;
    step = 1; // always start playing forward
}

void AnimationState::reset() { 
    activeAnimation = nullptr;
    frameIdx = 0;
    accumulator = 0.f;
    isPlaying = false;
    step = 1; // reset direction back to forward
}

Rectangle AnimationState::getSource() {
    if(!activeAnimation) { 
        std::cerr << "[ERROR] - No animation set" << std::endl;
        return Rectangle{0,0,0,0};
    }

    const std::vector<Frame>& frames = activeAnimation->getFrames();

    // an animation with no frames has nothing to index into - guard before frames.at()
    if(frames.empty()) {
        std::cerr << "[ERROR] - Active animation has no frames" << std::endl;
        return Rectangle{0,0,0,0};
    }

    // if animation is not playing then just return current frame with no incrementation
    if(!isPlaying) { return frames.at(frameIdx).source; }

    // accumulate delta time
    accumulator += GetFrameTime();

    // drain the accumulator, advancing as many frames as the elapsed time covers.
    // using a while (instead of if) lets the animation "catch up" after a lag spike
    // or when a frame's time is shorter than the real frametime.
    while(isPlaying && frames.at(frameIdx).time > 0.f && accumulator >= frames.at(frameIdx).time) {

        // consume one frame's worth of time from the accumulator
        accumulator -= frames.at(frameIdx).time;

        // check if pingPong is true (play forward, then bounce back)
        if(activeAnimation->rules.pingPong) {

            // are we on the last frame moving forward, or the first frame moving backward
            bool atEnd = (step > 0 && frameIdx == (int)frames.size() - 1);
            bool atStart = (step < 0 && frameIdx == 0);

            // check if at end or start of vector (a boundary where we need to decide what to do)
            if(atEnd || atStart) {

                // back at the first frame moving backward = one full there and back is done
                if(atStart && !activeAnimation->rules.isRepeating) {

                    // not repeating, so stop playing
                    isPlaying = false;

                    // check if animation is supposed to return to first frame when finished
                    // (already at 0 here, but reset step so a replay starts forward)
                    if(activeAnimation->rules.returnToFirstFrame) { frameIdx = 0; step = 1; }
                }

                // otherwise bounce: flip direction, then step off the boundary frame
                // (stepping after the flip means the boundary frame plays once, not twice)
                else {
                    step = -step;
                    frameIdx += step;
                }
            }

            // not at a boundary, keep moving in the current direction
            else {
                frameIdx += step;
            }
        }

        // normal one directional playback
        else {

            // check if on last frame in the animation
            if(frameIdx == (int)frames.size() - 1) {

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
            else { frameIdx += step; }
        }
    }

    // return current frame
    return frames.at(frameIdx).source;
}

} // namespace rad2d
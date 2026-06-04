#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>

#include "../animation/animation.h"

#include "raylib.h"

class Sprite {
private:
    std::string name;
    Texture2D texture; // !!!: Maybe just do texture path and load texture in higher draw class

    // int positionX, positionY
    int width, height;

    Animations animationList;
public:
    Sprite(const std::string& n,const std::string& texturePath,int w,int h);
    Sprite(const std::string& n,const std::string& texturePath,int w,int h,Animations& list);

    void addAnimation(const Animation& animation);
};
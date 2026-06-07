#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <utility>
#include <stdio.h>
#include <memory>

#include "raylib.h"

namespace rad2d {

class TextureRegistry {
private:
    std::unordered_map<int,std::shared_ptr<Texture2D>> registry;
public:
    std::shared_ptr<Texture2D> get(int id);
    void load(int id, const std::string& texturePath);
};

class FontRegistry {
private:
    std::unordered_map<int,std::shared_ptr<Font>> registry;
public:
    std::shared_ptr<Font> get(int id);
    void load(int id, const std::string& fontPath);
};

} // namespace rad2d
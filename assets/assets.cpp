#include "assets.h"

#include <iostream>

namespace rad2d {

std::shared_ptr<Texture2D> TextureRegistry::get(int id) {
    auto it = registry.find(id);

    // a missing asset shouldn't crash the consumer - log it and return null
    if(it == registry.end()) {
        std::cerr << "[ERROR] - No texture with id " << id << " in registry" << std::endl;
        return nullptr;
    }

    return it->second;
}

void TextureRegistry::load(int id,const std::string& texturePath) {
    registry.emplace(id,std::shared_ptr<Texture2D>(
        new Texture2D(LoadTexture(texturePath.c_str())),
        [](Texture2D* t){ UnloadTexture(*t); delete t; } // cleanup instructions
    ));
}

std::shared_ptr<Font> FontRegistry::get(int id) {
    auto it = registry.find(id);

    // a missing asset shouldn't crash the consumer - log it and return null
    if(it == registry.end()) {
        std::cerr << "[ERROR] - No font with id " << id << " in registry" << std::endl;
        return nullptr;
    }

    return it->second;
}

void FontRegistry::load(int id,const std::string& fontPath) {
    registry.emplace(id,std::shared_ptr<Font>(
        new Font(LoadFont(fontPath.c_str())),
        [](Font* f){ UnloadFont(*f); delete f; }
    ));
}

} // namespace rad2d
#include "assets.h"

std::shared_ptr<Texture2D> TextureRegistry::get(int id) {
    return registry.at(id);
}

void TextureRegistry::load(int id,const std::string& texturePath) {
    registry.emplace(id,std::shared_ptr<Texture2D>(
        new Texture2D(LoadTexture(texturePath.c_str())),
        [](Texture2D* t){ UnloadTexture(*t); delete t; } // cleanup instructions
    ));
}

std::shared_ptr<Font> FontRegistry::get(int id) {
    return registry.at(id);
}

void FontRegistry::load(int id,const std::string& fontPath) {
    registry.emplace(id,std::shared_ptr<Font>(
        new Font(LoadFont(fontPath.c_str())),
        [](Font* f){ UnloadFont(*f); delete f; }
    ));
}
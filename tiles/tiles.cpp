#include "tiles.h"

#include <iostream>

namespace rad2d {

/*
=== === ===
TILE
=== === ===
*/
Tile::Tile()
    : animated(false), currentSource({0,0,0,0}) {}

/*
=== === ===
TILESET
=== === ===
*/
TileSet::TileSet(AnimationRegistry* reg)
    : registry(reg) {}

void TileSet::defineTile(int tileId,std::shared_ptr<Texture2D> tex,Rectangle source) {
    Tile t;
    t.texture = std::move(tex);
    t.animated = false;
    t.currentSource = source;
    tiles[tileId] = std::move(t);
}

void TileSet::defineAnimatedTile(int tileId,std::shared_ptr<Texture2D> tex,int animationId) {
    // animated tiles need the central registry to resolve the animation id
    if(!registry) {
        std::cerr << "[ERROR] - TileSet has no animation registry; can't define animated tile " << tileId << std::endl;
        return;
    }

    // pull the shared Animation (registry logs + returns null if the id is missing)
    std::shared_ptr<Animation> anim = registry->get(animationId);
    if(!anim) { return; }

    Tile t;
    t.texture = std::move(tex);
    t.animated = true;
    t.animState.set(anim);
    t.animState.play();
    t.currentSource = t.animState.getSource(); // seed with the first frame
    tiles[tileId] = std::move(t);
}

void TileSet::update() {
    // advance each animated tile once so every cell that uses it stays in sync
    for(auto& pair : tiles) {
        Tile& t = pair.second;
        if(t.animated) {
            t.currentSource = t.animState.getSource();
        }
    }
}

const Tile* TileSet::get(int tileId) const {
    auto it = tiles.find(tileId);
    if(it == tiles.end()) { return nullptr; }
    return &it->second;
}

/*
=== === ===
TILEMAP
=== === ===
*/
Tilemap::Tilemap(const std::string& name,TileSet* set)
    : Drawable(name),tileset(set),cols(0), rows(0),tileSize(0) {}

Tilemap::Tilemap(const std::string& name,TileSet* set,int x,int y,int z,int tileSize)
    : Drawable(name,x,y,z,0,0),tileset(set),cols(0), rows(0),tileSize(tileSize) {}

Tilemap::~Tilemap() {}

void Tilemap::setTileSize(int size) {
    tileSize = size;
    // keep the Drawable's reported size in sync with the grid's pixel extent
    setWidth(cols * tileSize);
    setHeight(rows * tileSize);
}

void Tilemap::setTiles(const std::vector<int>& ids,int c,int r) {
    // the flat array has to match the declared grid dimensions
    if((int)ids.size() != c * r) {
        std::cerr << "[ERROR] - Tilemap tile count " << ids.size()
                  << " doesn't match " << c << "x" << r << std::endl;
        return;
    }

    tiles = ids;
    cols = c;
    rows = r;
    setWidth(cols * tileSize);
    setHeight(rows * tileSize);
}

void Tilemap::setTile(int col,int row,int id) {
    if(col < 0 || col >= cols || row < 0 || row >= rows) {
        std::cerr << "[ERROR] - Tilemap cell (" << col << "," << row << ") out of range" << std::endl;
        return;
    }
    tiles[row * cols + col] = id;
}

int Tilemap::getTile(int col,int row) const {
    if(col < 0 || col >= cols || row < 0 || row >= rows) { return 0; }
    return tiles[row * cols + col];
}

void Tilemap::draw() {
    if(!tileset) {
        std::cerr << "[ERROR] - Tilemap has no tileset" << std::endl;
        return;
    }

    const int originX = getPositionX();
    const int originY = getPositionY();

    for(int r = 0; r < rows; ++r) {
        for(int c = 0; c < cols; ++c) {
            const int id = tiles[r * cols + c];
            if(id == 0) { continue; } // 0 = empty cell, draw nothing

            const Tile* tile = tileset->get(id);
            if(!tile || !tile->texture) { continue; } // undefined id -> skip silently

            Rectangle dest = {
                (float)(originX + c * tileSize),
                (float)(originY + r * tileSize),
                (float)tileSize,
                (float)tileSize
            };

            DrawTexturePro(*tile->texture, tile->currentSource, dest, {0,0}, 0.0f, WHITE);
        }
    }
}

} // namespace rad2d

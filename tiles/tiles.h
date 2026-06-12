#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "../animation/animation.h"
#include "../draw/draw.h"

#include "raylib.h"

namespace rad2d {

/*
=== Tile ===
One tile definition: a texture plus either a fixed source rectangle (static) or a
shared AnimationState (animated). `currentSource` is the rect to crop this frame -
TileSet::update() refreshes it for animated tiles; static tiles never change.
=== === === ===
*/
struct Tile {
    std::shared_ptr<Texture2D> texture;
    bool animated;
    Rectangle currentSource;  // the crop rectangle to draw this frame
    AnimationState animState; // only used when animated

    Tile();
};

/*
=== TileSet ===
Maps tile ids (what your map data stores) to Tile definitions. Animated tiles pull
their Animation from the shared central AnimationRegistry, so a tile can use the
very same animation as a sprite. Call update() once per frame to advance them - one
advance per tile type keeps every cell of that type in sync and cheap.
=== === === ===
*/
class TileSet {
private:
    AnimationRegistry* registry;        // non-owning: shared central animation registry
    std::unordered_map<int,Tile> tiles; // tile id -> definition
public:
    TileSet(AnimationRegistry* reg);

    // define a static tile: crops `source` from `tex`
    void defineTile(int tileId,std::shared_ptr<Texture2D> tex,Rectangle source);

    // define an animated tile: plays animation `animationId` from the registry
    void defineAnimatedTile(int tileId,std::shared_ptr<Texture2D> tex,int animationId);

    // advance every animated tile by one frame's worth of time (call once per frame)
    void update();

    // look up a tile for drawing; nullptr if `tileId` was never defined
    const Tile* get(int tileId) const;
};

/*
=== Tilemap ===
A single z-layer of tiles. Holds a flat, row-major grid of tile ids (0 = empty) and
draws each non-empty cell through a shared TileSet. Stack several Tilemaps with
different z values for multi-layer maps. The map *format* (JSON / CSV / ...) lives
in your game; feed the parsed ids in via setTiles().
=== === === ===
*/
class Tilemap : public Drawable {
private:
    TileSet* tileset;       // non-owning: shared tile definitions
    std::vector<int> tiles; // row-major tile ids, length = cols*rows (0 = empty)
    int cols, rows;         // grid dimensions in tiles
    int tileSize;           // px per tile (square)
public:
    Tilemap(const std::string& name,TileSet* set);
    Tilemap(const std::string& name,TileSet* set,int x,int y,int z,int tileSize);
    ~Tilemap();

    void setTileSize(int size);

    // load a whole layer at once (e.g. from the data your engine parsed)
    void setTiles(const std::vector<int>& ids,int cols,int rows);

    // poke a single cell; out-of-range is ignored
    void setTile(int col,int row,int id);
    int getTile(int col,int row) const; // 0 (empty) if out of range or unset

    // draws every non-empty cell at origin + (col,row) * tileSize
    void draw() override;
};

} // namespace rad2d

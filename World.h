#pragma once

#include "Tile.h"

constexpr unsigned int map_width = 256;
constexpr unsigned int map_depth = 16;


class LevelLayer
{
public:
    explicit LevelLayer(int heightOffset);

    glm::ivec2 getSize() const { return size; }
    std::span<const Tile> getData() const { return tiles; }

    Tile &getTile(glm::ivec2 pos) { return tiles[pos.y * size.x + pos.x]; }
    const Tile &getTile(glm::ivec2 pos) const { return tiles[pos.y * size.x + pos.x]; }

    void visit(const std::function<void(glm::ivec2, Tile &)> &visitor);
    void visit(const std::function<void(glm::ivec2, const Tile &)> &visitor) const;

private:
    int heightOffset = 0;
    glm::ivec2 size {map_width, map_width};
    std::vector<Tile> tiles;
};

class World
{
public:
    explicit World();

    LevelLayer *getLayer(int depth)
    {
        const int index = depth - firstLayerDepth;
        if (index < 0 || index >= layers.size())
            return nullptr;

        return &layers[index];
    }

    std::span<const TileClass> getClasses() const { return tileClasses; }

private:
    std::mt19937 random;

    int firstLayerDepth = 0;
    std::deque<LevelLayer> layers;

    std::vector<TileClass> tileClasses;
};


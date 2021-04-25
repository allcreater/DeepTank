#pragma once

#include "Tile.h"

class WorldGenerator;

class LevelLayer
{
public:
    explicit LevelLayer(glm::ivec2 horizontalDimensions, int heightOffset);

    int getDepth() const { return depth; }
    glm::ivec2 getSize() const { return size; }
    size_t getRevision() const { return revision; }

        //    std::span<const Tile> getData() const { return tiles; }

    void beginIntialize(std::shared_ptr<WorldGenerator> generator);

    Tile &getTile(glm::ivec2 pos);
    const Tile &getTile(glm::ivec2 pos) const;

    void visit(const std::function<void(glm::ivec2, Tile &)> &visitor);
    void visit(const std::function<void(glm::ivec2, const Tile &)> &visitor) const;

private:
    Tile &getTileUnsafe(glm::ivec2 pos);
    const Tile &getTileUnsafe(glm::ivec2 pos)const;

private:
    int depth = 0;
    glm::ivec2 size;

    size_t revision = 0;

    std::vector<Tile> tiles;
    std::future<void> loadTask;
    mutable std::recursive_mutex tilesMutex;
};

class World
{
public:
    explicit World();

    LevelLayer *getLayer(int depth);

    void Update(float dt);

    void setGenerator(std::shared_ptr<WorldGenerator> _generator) { generator = std::move(_generator); }
    std::span<const TileClass> getClasses() const { return tileClasses; }

private:
    std::shared_ptr<WorldGenerator> generator;

    size_t maxLoadedLayers = 32;
    int firstLayerDepth = 0;
    std::deque<LevelLayer> layers;

    std::vector<TileClass> tileClasses;

};


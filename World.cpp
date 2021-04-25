#include "stdafx.h"

#include "World.h"
#include "WorldGenerator.h"

LevelLayer::LevelLayer(glm::ivec2 horizontalDimensions, int depth) :
    size{horizontalDimensions}, depth{depth}
{
    
}

void LevelLayer::beginIntialize(std::shared_ptr<WorldGenerator> generator)
{
    if (loadTask.valid())
        return;

    loadTask = std::async(std::launch::async, [this, generator]() mutable
    {
        std::lock_guard tilesLock{tilesMutex};

        tiles.resize(size.x * size.y);
        generator->generateLevelLayer(*this);
    });

    //loadTask.get();
}

Tile &LevelLayer::getTileUnsafe(glm::ivec2 pos)
{
    return tiles[pos.y * size.x + pos.x];
}

const Tile &LevelLayer::getTileUnsafe(glm::ivec2 pos) const
{
    return tiles[pos.y * size.x + pos.x];
}

Tile &LevelLayer::getTile(glm::ivec2 pos)
{
    std::lock_guard tilesLock{tilesMutex};
    if (!tiles.empty())
        return getTileUnsafe(pos);

    revision++;

    throw std::logic_error{"trying to modify empty layer"};
}

const Tile &LevelLayer::getTile(glm::ivec2 pos) const
{
    std::lock_guard tilesLock{tilesMutex};
    if (!tiles.empty())
        return getTileUnsafe(pos);

    return Tile::Empty();
}

void LevelLayer::visit(const std::function<void(glm::ivec2, const Tile &)>& visitor) const
{
    std::lock_guard tilesLock{tilesMutex};
    using scalar = decltype(size)::value_type;
    for (scalar y = 0; y < size.y; ++y)
    for (scalar x = 0; x < size.x; ++x)
    {
        visitor({x, y}, getTileUnsafe({x, y}));
    }
}

void LevelLayer::visit(const std::function<void(glm::ivec2, Tile &)>& visitor)
{
    std::lock_guard tilesLock{tilesMutex};
    revision++;

    using scalar = decltype(size)::value_type;
    for (scalar y = 0; y < size.y; ++y)
    for (scalar x = 0; x < size.x; ++x)
    {
        visitor({x, y}, getTileUnsafe({x, y}));
    }
}

World::World()
{
    tileClasses.emplace_back( "empty"s );
    tileClasses.emplace_back( "crystal"s );
    tileClasses.emplace_back( "ground"s );
    tileClasses.emplace_back( "rock"s );
    tileClasses.emplace_back( "cuprum_ore"s );
    tileClasses.emplace_back( "gold_ore"s );
}

LevelLayer *World::getLayer(int depth)
{
    const int index = depth - firstLayerDepth;
    if (index < 0 || index >= layers.size())
        return nullptr;

    return &layers[index];
}

void World::Update(float dt)
{
    // Удаляем ненужные слои
    while (!layers.empty() && layers.front().getDepth() < firstLayerDepth)
        layers.pop_front();

    for (int i = layers.size(); i < maxLoadedLayers; ++i)
    {
        auto& newLayer = layers.emplace_back(generator->getLayerDimensions(), firstLayerDepth + i);
        newLayer.beginIntialize(generator);
    }
}

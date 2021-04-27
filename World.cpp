#include "stdafx.h"

#include "Actor.h"
#include "World.h"
#include "WorldGenerator.h"

LevelLayer::LevelLayer(glm::ivec2 horizontalDimensions, int depth) :
    size{horizontalDimensions}, depth{depth}
{    
}

Tile &LevelLayer::getTileUnsafe(glm::ivec2 pos)
{
    return tiles[pos.y * size.x + pos.x];
}

const Tile &LevelLayer::getTileUnsafe(glm::ivec2 pos) const
{
    return tiles[pos.y * size.x + pos.x];
}

void LevelLayer::swap(LevelLayer &other)
{
    std::swap(depth, other.depth);
    std::swap(size, other.size);
    std::swap(revision, other.revision);
    std::swap(tiles, other.tiles);
}

Tile &LevelLayer::getTile(glm::ivec2 pos)
{
    revision++;

    if (!tiles.empty() && pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y)
        return getTileUnsafe(pos);

    throw std::logic_error{"trying to modify empty layer"};
}

const Tile &LevelLayer::getTile(glm::ivec2 pos) const
{
    if (!tiles.empty() && pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y)
        return getTileUnsafe(pos);

    return Tile::Empty();
}

void LevelLayer::visit(const std::function<void(glm::ivec2, Tile &)> &visitor)
{
    visit(visitor, {0, 0}, size);
}

void LevelLayer::visit(const std::function<void(glm::ivec2, const Tile &)> &visitor) const
{
    visit(visitor, {0, 0}, size);
}

void LevelLayer::visit(const std::function<void(glm::ivec2, const Tile &)> &visitor, glm::ivec2 from,
                       glm::ivec2 to) const
{
    from = max({0, 0}, from);
    to = min(size, to);

    for (auto y = from.y; y < to.y; ++y)
    for (auto x = from.x; x < to.x; ++x)
    {
        visitor({x, y}, getTileUnsafe({x, y}));
    }
}

void LevelLayer::setData(std::vector<Tile> &&data)
{
    if (data.size() != (size.x * size.y))
        throw std::logic_error{"level layer data have invalid length"};

    tiles = std::move( data );
    revision = 0;
}

void LevelLayer::visit(const std::function<void(glm::ivec2, Tile &)> &visitor, glm::ivec2 from, glm::ivec2 to)
{
    from = max({0, 0}, from);
    to = min(size, to);

    for (auto y = from.y; y < to.y; ++y)
    for (auto x = from.x; x < to.x; ++x)
    {
        visitor({x, y}, getTileUnsafe({x, y}));
    }

    revision++;
}

World::World()
{
}

World::~World()
{
    //should be automatic
    std::ranges::for_each(actors, std::bind_front(&World::callOnDestroyForActor, this));
}

LevelLayer *World::getLayer(int depth)
{
    const int index = depth - firstLayerDepth;
    if (index < 0 || index >= layers.size())
        return nullptr;

    if (auto* layer = std::get_if<LevelLayer>(&layers[index]))
        return layer;

    return nullptr;
}

const LevelLayer * World::getLayer(int depth) const
{
    return const_cast<World *>(this)->getLayer(depth);
}

World::CellType World::categorizeTile(glm::ivec3 point) const
{
    const auto *wallLayer = getLayer(point.z);
    if (!wallLayer || !wallLayer->isLoaded())
        return CellType::Unloaded;

    const auto horizontalPoint = glm::ivec2{point.x, point.y};
    const auto& wallClass = generator->getClasses()[wallLayer->getTile(horizontalPoint).classId];
    if (wallClass.isSolid)
        return World::CellType::Wall;

    const auto *floorLayer = getLayer(point.z + 1);
    if (!floorLayer || !floorLayer->isLoaded())
        return CellType::Unloaded;

    const auto &floorClass = generator->getClasses()[floorLayer->getTile(horizontalPoint).classId];
    if (floorClass.isSolid)
        return CellType::Floor;

    return CellType::Empty;
}

Actor &World::addActor(std::shared_ptr<Actor> actor)
{
    actor->setWorld(this);
    actors.push_back(std::move(actor));
    const auto& actorRef = actors.back();

    // if immediately initialization is not possible, will be initialized with layer
    if (const auto *layer = getLayer(actorRef->getPosition().z))
        callOnReadyForActor(actorRef, *layer);

    return *actorRef;
}

void World::Update(float dt)
{

    // включаем загруженные слои
    for (auto& layer : layers)
    {
        std::visit(overloaded{[this, &layer](std::future<LevelLayer> &future) {
                                  if (future.wait_until(std::chrono::system_clock::time_point::min()) !=
                                      std::future_status::ready)
                                      return;

                                  layer = future.get();
                                  onLayerLoaded(std::get<LevelLayer>(layer));
                                  
                              },
                              [](const auto&) {}},
                   layer);
    }

    for (int i = layers.size(); i < maxLoadedLayers; ++i)
    {
        layers.push_back(generator->generateLevelLayerAsync(firstLayerDepth + i));
    }

    for (auto &actor : actors)
    {
        if (const auto layer = getLayer(actor->getPosition().z))
            actor->update(dt, *this);
    }

    auto tailRange = std::ranges::remove_if(actors, [this](std::shared_ptr<Actor> &actor)
    {
        bool isDying = !actor->isAlive();
        if (isDying) // crutch
            callOnDestroyForActor(actor);
        return isDying;
    });

    actors.erase(tailRange.begin(), tailRange.end());

}

void World::trimLevelsAbove(int minimalInterestingDepth)
{
    //// Удаляем ненужные слои. Осторожно, костыли
    while ((!layers.empty() && std::holds_alternative<LevelLayer>(layers.front()) &&
            std::get<LevelLayer>(layers.front()).getDepth() < minimalInterestingDepth))
    {
        layers.erase(layers.begin());
        firstLayerDepth++;
    }

}

const std::vector<Actor *> &World::queryPoint(glm::vec3 point)
{
    static std::vector<Actor *> container;
    container.clear();

    for (auto* actor : collideableActors)
    {
        if (floor(actor->getPosition().z) == floor(point.z) &&
            glm::length(xy(actor->getPosition()) - xy(point)) <= static_cast<float>(actor->getSize()))
            container.push_back(actor);
    }

    return container;
}

void World::onLayerLoaded(const LevelLayer &layer)
{
    for (const auto &actor : actors)
        callOnReadyForActor(actor, layer);
}

void World::callOnReadyForActor(const std::shared_ptr<Actor> &actor, const LevelLayer &layer)
{
    if (static_cast<int>(actor->getPosition().z) == layer.getDepth())
        actor->onReady(*this);
}

void World::callOnDestroyForActor(const std::shared_ptr<Actor> &actor)
{
    unregisterForCollision(actor.get());
    actor->onDestroy(*this);
    actor->setWorld(nullptr);
}
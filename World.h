#pragma once

#include "Tile.h"

class Actor;
class WorldGenerator;

class LevelLayer
{
public:
    LevelLayer() { std::puts("hello"); } // ??????-?? ???????????? ??? ???????? future, ???????? ???? ????????
    explicit LevelLayer(glm::ivec2 horizontalDimensions, int heightOffset);

    int getDepth() const { return depth; }
    glm::ivec2 getSize() const { return size; }
    size_t getRevision() const { return revision; }
    const bool isLoaded() const { return !tiles.empty(); }

        //    std::span<const Tile> getData() const { return tiles; }

    Tile &getTile(glm::ivec2 pos);
    const Tile &getTile(glm::ivec2 pos) const;

    void visit(const std::function<void(glm::ivec2, Tile &)> &visitor);
    void visit(const std::function<void(glm::ivec2, const Tile &)> &visitor) const;
    void visit(const std::function<void(glm::ivec2, Tile &)> &visitor, glm::ivec2 from, glm::ivec2 to);
    void visit(const std::function<void(glm::ivec2, const Tile &)> &visitor, glm::ivec2 from, glm::ivec2 to) const;

    void setData(std::vector<Tile> &&data);

private:
    Tile &getTileUnsafe(glm::ivec2 pos);
    const Tile &getTileUnsafe(glm::ivec2 pos)const;

    void swap(LevelLayer &other);

private:
    int depth = 0;
    glm::ivec2 size;

    size_t revision = 0;

    std::vector<Tile> tiles;
};

class World
{
public:
    enum class CellType
    {
        Unloaded,
        Empty,
        Floor,
        Wall,
        Ramp
    };

    using ActorsList = std::list<std::shared_ptr<Actor>>;

public:
    explicit World();
    ~World();

    LevelLayer *getLayer(int depth);
    const LevelLayer *getLayer(int depth) const ;
    CellType categorizeTile(glm::ivec3 point) const;


    Actor &addActor(std::shared_ptr<Actor> actor);
    const ActorsList &getActors() const { return actors; }
    
    void Update(float dt);

    std::shared_ptr<WorldGenerator> getGenerator() const { return generator; }
    void setGenerator(std::shared_ptr<WorldGenerator> _generator) { generator = std::move(_generator); }

    void trimLevelsAbove(int minimalInterestingDepth);

    // simple collision detection
    void registerForCollision(Actor *actor) { collideableActors.insert(actor); }

    void unregisterForCollision(Actor *actor) { collideableActors.erase(actor); }

    const std::vector<Actor *> &queryPoint(glm::vec3 point);

    size_t getFrameStamp() const { return frameStamp; }

private:
    void onLayerLoaded(const LevelLayer &layer);
    void callOnReadyForActor(const std::shared_ptr<Actor> &actor, const LevelLayer &layer);
    void callOnDestroyForActor(const std::shared_ptr<Actor> &actor);

private:
    struct UnavailableLevel{};
    using Layer = std::variant<LevelLayer, std::future<LevelLayer>, UnavailableLevel>;

    std::shared_ptr<WorldGenerator> generator;

    size_t maxLoadedLayers = 32;
    int firstLayerDepth = 0;

    std::deque<Layer> layers;

    size_t frameStamp = 0;
    ActorsList actors;
    std::unordered_set<Actor*> collideableActors;
};


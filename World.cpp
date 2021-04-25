#include "stdafx.h"

#include "World.h"

LevelLayer::LevelLayer(int heightOffset)
: tiles{size.x * size.y}
, heightOffset{heightOffset} {}

void LevelLayer::visit(const std::function<void(glm::ivec2, const Tile &)>& visitor) const
{
    for (int y = 0; y < size.y; ++y)
        for (int x = 0; x < size.x; ++x)
        {
            visitor({x, y}, getTile({x, y}));
        }
}

void LevelLayer::visit(const std::function<void(glm::ivec2, Tile &)>& visitor)
{
    for (int y = 0; y < size.y; ++y)
    for (int x = 0; x < size.x; ++x)
    {
        visitor({x, y}, getTile({x, y}));
    }
}


#include <noise/noise.h>

World::World()
    : random{std::random_device{}()}
{
    tileClasses.emplace_back( "empty"s );
    tileClasses.emplace_back( "crystal"s );
    tileClasses.emplace_back( "ground"s );
    tileClasses.emplace_back( "rock"s );
    tileClasses.emplace_back( "cuprum_ore"s );
    tileClasses.emplace_back( "gold_ore"s );


    noise::module::Perlin noise;

    // world generation
    for (size_t h = 0; h < map_depth; ++h)
    {
        auto &currentLayer = layers.emplace_back(h);

        currentLayer.visit([&,this](glm::ivec2 pos, Tile & tile) {
            //std::uniform_int_distribution<int> distr_tileClass{0, 5};

            auto val = noise.GetValue(pos.x * 0.1, pos.y * 0.1, static_cast<double>(h) * 0.1);

            tile.classId = 0;
            if (val < -0.3)
                tile.classId = 3;
            else if (val < 0.4)
                tile.classId = 2;
        });
    }
}

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

void LayerRenderer::update(const LevelLayer &layer, const World &world)
{
    const size_t bufferLength = layer.getSize().x * layer.getSize().y * 4;

    vertexArray.resize(0);

    layer.visit([&](glm::ivec2 pos, const Tile & tile) {
        const auto &tileClass = world.getClasses()[tile.classId];

        const auto &[a, b] = tileClass.texCoords;

        const glm::vec2 lt = glm::vec2{pos} - 3.0f / 12, bd = glm::vec2{pos} + 15.0f / 12;

        vertexArray.append(sf::Vertex{{lt.x, lt.y}, a});
        vertexArray.append(sf::Vertex{{bd.x, lt.y}, sf::Vector2f{b.x, a.y}});
        vertexArray.append(sf::Vertex{{bd.x, bd.y}, b});
        vertexArray.append(sf::Vertex{{lt.x, bd.y}, sf::Vector2f{a.x, b.y}});
    });
}

void LayerRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::Transform a;
    //a.scale(8.0f, 8.0f);

    states.texture = texture;
    states.transform *= a;
    target.draw(vertexArray, states);
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

World::World(sf::Texture* tilesTexture)
    : random{std::random_device{}()}
    , tilesTexture{tilesTexture}
{
    constexpr float tileSize = 12.0f;

    auto getTileCoords = [](int i) {
        return std::pair<sf::Vector2f, sf::Vector2f>{{(tileSize) * i, 0},
                                                     {(tileSize) * (i + 1), tileSize}};
    };


    tileClasses.emplace_back( "empty"s, getTileCoords(0) );
    tileClasses.emplace_back( "crystal"s, getTileCoords(1) );
    tileClasses.emplace_back( "ground"s, getTileCoords(2) );
    tileClasses.emplace_back( "rock"s, getTileCoords(3) );
    tileClasses.emplace_back( "cuprum_ore"s, getTileCoords(4) );
    tileClasses.emplace_back( "gold_ore"s, getTileCoords(5) );


    noise::module::Perlin noise;

    // world generation
    for (size_t h = 0; h < map_depth; ++h)
    {
        auto &currentLayer = layers.emplace_back(h);

        currentLayer.visit([&,this](glm::ivec2 pos, Tile & tile) {
            //std::uniform_int_distribution<int> distr_tileClass{0, 5};

            auto val = noise.GetValue(pos.x * 0.1 + 1.25, pos.y * 0.1 + 0.75, static_cast<double>(h) * 0.1 + 0.50);

            tile.classId = 0;
            if (val < -0.3)
                tile.classId = 3;
            else if (val < 0.4)
                tile.classId = 2;
        });
    }
}

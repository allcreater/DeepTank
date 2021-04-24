#include "stdafx.h"

#include "World.h"

LevelLayer::LevelLayer(int heightOffset)
: tiles{size.x * size.y}
, heightOffset{heightOffset} {}

void LevelLayer::visit( std::function<void(glm::ivec2, const Tile &)> visitor) const
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

        glm::vec2 fpos = pos;

        vertexArray.append(sf::Vertex{{fpos.x, fpos.y}, a});
        vertexArray.append(sf::Vertex{{fpos.x + 1, fpos.y}, sf::Vector2f{b.x, a.x}});
        vertexArray.append(sf::Vertex{{fpos.x + 1, fpos.y + 1}, b});
        vertexArray.append(sf::Vertex{{fpos.x, fpos.y + 1}, sf::Vector2f{a.x, b.x}});
    });
}

void LayerRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::Transform a;
    a.scale(10.0f, 10.0f);

    states.texture = texture;
    states.transform *= a;
    target.draw(vertexArray, states);
}

void LevelLayer::visit(std::function<void(glm::ivec2, Tile &)> visitor)
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
    constexpr float tileSize = 8.0f;

    auto getTileCoords = [texSize = tilesTexture->getSize()](int i) {
        return std::pair<sf::Vector2f, sf::Vector2f>{{tileSize * i, 0.0f},
                                                     {tileSize * (i+1), tileSize}};
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

            auto val = noise.GetValue(pos.x, pos.y, static_cast<double>(h));

            //tile.classId = distr_tileClass(random);
        });
    }
}

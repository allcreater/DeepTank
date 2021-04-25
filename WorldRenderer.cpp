#include "stdafx.h"
#include "WorldRenderer.h"

#include "World.h"

void LayerRenderer::update(const LevelLayer &layer, TextureAtlas &tileAtlas)
{
    const size_t bufferLength = layer.getSize().x * layer.getSize().y * 4;
    vertexArray.resize(bufferLength);

    texture = &tileAtlas.texture;
    
    size_t baseVertexIndex = 0;
    layer.visit([&](glm::ivec2 pos, const Tile &tile) {
        const auto& region = tileAtlas.regions[tile.classId];
        const auto bottom = region.top + region.height;
        const auto right = region.left + region.width;

        //const glm::vec2 lt = glm::vec2{pos} - 3.0f / 12, bd = glm::vec2{pos} + 15.0f / 12;
        pos *= 8;
        const glm::vec2 lt = glm::vec2{pos} - 6.0f, bd = glm::vec2{pos} + 6.0f;

        vertexArray[baseVertexIndex+0] = sf::Vertex{{lt.x, lt.y}, baseColor, sf::Vector2f{region.left, region.top}};
        vertexArray[baseVertexIndex+1] = sf::Vertex{{bd.x, lt.y}, baseColor, sf::Vector2f{right, region.top}};
        vertexArray[baseVertexIndex+2] = sf::Vertex{{bd.x, bd.y}, baseColor, sf::Vector2f{right,bottom}};
        vertexArray[baseVertexIndex+3] = sf::Vertex{{lt.x, bd.y}, baseColor, sf::Vector2f{region.left, bottom}};

        baseVertexIndex += 4;
    });

    if (vertexBuffer.getVertexCount() != vertexArray.getVertexCount())
        vertexBuffer.create(vertexArray.getVertexCount());
    vertexBuffer.update(&vertexArray[0], vertexArray.getVertexCount(), 0);
}

void LayerRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.texture = texture;
    target.draw(vertexBuffer, states);

}

WorldRenderer::WorldRenderer(World &world, TextureAtlas &tilesAtlas) : world{world}, tilesAtlas{tilesAtlas}
{
    for (size_t i = 0; i < 5; ++i)
    {
        auto &renderer = renderers.emplace_back();
        const uint8_t intensity = 255 / (i + 1);
        renderer.setBaseColor(sf::Color{intensity, intensity, intensity, 255});
        renderer.update(*world.getLayer(i), tilesAtlas);
    }
}

void WorldRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    for (int depth = static_cast<int>(renderers.size()) - 1; depth >= 0; --depth)
    {
        auto &renderer = renderers[depth];

        const auto scaleFactor = 1.0f / static_cast<float>(depth * 0.02f + 1);

        sf::RenderStates rs;
        rs.transform.translate(cameraPosition);
        rs.transform.scale(scaleFactor, scaleFactor);
        rs.transform.translate(-cameraPosition);

        target.draw(renderer, rs);
    }

    //target.draw(tankSprite);
}

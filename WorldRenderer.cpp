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
}

void LayerRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.texture = texture;
    target.draw(vertexArray, states);
}

void WorldRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

}

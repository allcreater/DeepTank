#include "stdafx.h"
#include "WorldRenderer.h"

#include "World.h"

const TextureAtlas::Region& TextureAtlas::getRegion(size_t index) const 
{
    static Region empty{};
    if (index >= regions.size())
        return empty;

    return regions[index];
}

TextureAtlas TextureAtlas::MakeFromRegularGrid(const std::string &textureName, glm::uvec2 cellSize, size_t numOfCells)
{
    TextureAtlas atlas;
    if (!atlas.texture.loadFromFile(textureName))
        throw std::runtime_error{"texture "s + textureName + " can't be loaded"s};

    const auto textureSize = atlas.texture.getSize();
    const auto numOfPossibleCells = (textureSize.x / cellSize.x) * (textureSize.y / cellSize.y);

    if (numOfPossibleCells < numOfCells)
        throw std::logic_error{"texture atlas "s + textureName + " could contain only "s +
            std::to_string(numOfPossibleCells) + " cells, not " + std::to_string(numOfCells)};

    atlas.regions.resize(numOfCells);

    sf::FloatRect currentRegion{0, 0, static_cast<float>(cellSize.x), static_cast<float>(cellSize.y)};
    for (int i = 0; i < numOfCells; ++i)
    {
        atlas.regions[i] = currentRegion;
        if (currentRegion.left + currentRegion.width >= textureSize.x)
        {
            currentRegion.left = 0;
            currentRegion.top += currentRegion.height;
        }
        else
            currentRegion.left += currentRegion.width;
    }

    return atlas;
}


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
    sf::Transform a;
    // a.scale(8.0f, 8.0f);

    states.texture = texture;
    states.transform *= a;
    target.draw(vertexArray, states);
}
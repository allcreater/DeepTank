#include "stdafx.h"
#include "TextureAtlas.h"

const TextureAtlas::Region &TextureAtlas::getRegion(size_t index) const
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
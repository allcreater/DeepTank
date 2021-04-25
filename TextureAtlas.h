#pragma once

#include <SFML/Graphics.hpp>

struct TextureAtlas
{
    using Region = sf::FloatRect;

    sf::Texture texture;
    std::vector<Region> regions;

    const Region &getRegion(size_t index) const;

    static TextureAtlas MakeFromRegularGrid(const std::string &textureName, glm::uvec2 cellSize, size_t numOfCells);
};
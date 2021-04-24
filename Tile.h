#pragma once

#include <SFML/Graphics.hpp>

//
//struct Resource
//{
//};

struct TileClass
{
    std::string name;
    std::pair<sf::Vector2f, sf::Vector2f> texCoords;
    int16_t initialStrength = 1;
};

struct Tile
{
    int classId = 0;
};
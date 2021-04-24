#pragma once

#include <SFML/Graphics.hpp>

//
//struct Resource
//{
//};

struct TileClass
{
    std::string name;
    int16_t initialStrength = 1;
};

struct Tile
{
    int classId = 0;
};
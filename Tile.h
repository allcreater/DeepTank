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
    bool isSolid = true;
};

struct Tile
{
    int classId = 0;

    const static Tile& Empty()
    {
        static Tile empty;
        return empty;
    }
};
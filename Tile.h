#pragma once

//
//struct Resource
//{
//};

using TileClassId = int;

struct TileClass
{
    const TileClassId id;
    std::string name;
    int16_t initialStrength = 1;
    int16_t value = 0;
    bool isSolid = true;
};


struct Tile
{
    Tile() = default;
    Tile(const TileClass& prototype)
    : classId{prototype.id}
    , actualStrength{prototype.initialStrength}
    {}

    int classId = 0;
    int16_t actualStrength = 0;

    const static Tile& Empty()
    {
        static Tile empty;
        return empty;
    }
};
#include "stdafx.h"
#include "WorldGenerator.h"

WorldGenerator::WorldGenerator(glm::uvec2 horizontalDimensions):
    horizontalDimensions{horizontalDimensions}
{
    tileClasses.emplace_back(0, "empty"s, 0, 0, false);
    tileClasses.emplace_back(1, "dirt"s, 5);
    tileClasses.emplace_back(2, "rock"s, 20);
    tileClasses.emplace_back(3, "cuprum_ore"s, 3, 1);
    tileClasses.emplace_back(4, "mineral_sapphire"s, 4, 4);
    tileClasses.emplace_back(5, "mineral_emerald"s, 4, 5);
    tileClasses.emplace_back(6, "mineral_amethyst"s, 4, 3);
    tileClasses.emplace_back(7, "mineral_hz"s, 4, 6);
    tileClasses.emplace_back(8, "gold_ore"s, 3, 2);
    tileClasses.emplace_back(9, "mineral_ruby"s, 10, 10);
    tileClasses.emplace_back(10, "fuel"s, 1);
}

void WorldGenerator::generateLevelLayer(LevelLayer& currentLayer)
{
    static noise::module::Perlin noise; 
    static noise::module::RidgedMulti rmf;
    static noise::module::Perlin smallNoise;

    static noise::module::Add add;

    [&]() {
        rmf.SetOctaveCount(4);
        rmf.SetFrequency(0.2);

        add.SetSourceModule(0, noise);
        add.SetSourceModule(1, rmf);

        smallNoise.SetFrequency(0.7);
        smallNoise.SetOctaveCount(2);
    }();

    const auto depth = static_cast<double>(currentLayer.getDepth()) * 1.2;

    currentLayer.visit([&, this](glm::ivec2 pos, Tile &tile) {
        // std::uniform_int_distribution<int> distr_tileClass{0, 5};

        auto val = add.GetValue(pos.x * 0.1, pos.y * 0.1, depth);

        tile = Tile::Empty();
        if (val < -0.3) 
            tile = Tile{tileClasses[2]};
        else if (val < 0.4)
            tile = Tile{tileClasses[1]};

        if (tileClasses[tile.classId].isSolid)
        {

            for (TileClassId i = 3; i <= 10; ++i)
            {
                const auto frequency = i * 0.02f;
                const auto x = smallNoise.GetValue(pos.x * frequency, pos.y * frequency, depth + 1000 * i);
                if (x < -0.98)
                {
                    tile = tileClasses[i];
                    break;
                }
            }
        }

        /*
        if (val < -0.6)
            tile.classId = 5;
        else if (val < -0.3)
            tile.classId = 3;
        else if (val < 0.4)
            tile.classId = 2;
            */
    });
}

std::future<LevelLayer> WorldGenerator::generateLevelLayerAsync(int depth)
{
    return std::async(std::launch::async, [generator = shared_from_this(), depth]() mutable {
        LevelLayer replacementLayer{generator->horizontalDimensions, depth};
        replacementLayer.setData(std::vector<Tile>{generator->horizontalDimensions.x * generator->horizontalDimensions.y});

        generator->generateLevelLayer(replacementLayer);

        return replacementLayer;
    });
}



void FillRoundArea(LevelLayer &layer, glm::ivec2 center, int radius, Tile fillingTile)
{
    layer.visit([=](glm::ivec2 pos, Tile &tile)
    {
        if (auto dir = center - pos; sqrt(dir.x * dir.x + dir.y * dir.y) <= radius)
            tile = fillingTile;

    }, center - glm::ivec2{radius}, center + glm::ivec2{radius});
}

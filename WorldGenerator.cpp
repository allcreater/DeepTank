#include "stdafx.h"
#include "WorldGenerator.h"

WorldGenerator::WorldGenerator(glm::uvec2 horizontalDimensions):
    horizontalDimensions{horizontalDimensions}
{
}

void WorldGenerator::generateLevelLayer(LevelLayer& currentLayer)
{
    static noise::module::Perlin noise;
    static noise::module::RidgedMulti rmf;
    rmf.SetFrequency(0.2);

    const auto depth = static_cast<double>(currentLayer.getDepth()) * 1.2;

    currentLayer.visit([&, this](glm::ivec2 pos, Tile &tile) {
        // std::uniform_int_distribution<int> distr_tileClass{0, 5};

        auto val = rmf.GetValue(pos.x * 0.1, pos.y * 0.1, depth);

        tile.classId = 0;
        if (val < -0.3)
            tile.classId = 3;
        else if (val < 0.4)
            tile.classId = 2;
    });
}

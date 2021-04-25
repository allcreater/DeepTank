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
        if (val < -0.6)
            tile.classId = 5;
        else if (val < -0.3)
            tile.classId = 3;
        else if (val < 0.4)
            tile.classId = 2;
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

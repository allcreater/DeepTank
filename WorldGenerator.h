#pragma once

#include "World.h"
#include <noise/noise.h>

class WorldGenerator : public std::enable_shared_from_this<WorldGenerator>
{
public:
    WorldGenerator(glm::uvec2 horizontalDimensions);

    glm::uvec2 getLayerDimensions() const { return horizontalDimensions; }
    void generateLevelLayer(LevelLayer &currentLayer);
    std::future<LevelLayer> generateLevelLayerAsync(int depth);

 private:
    glm::uvec2 horizontalDimensions;

};

// Drawing
void FillRoundArea(LevelLayer &layer, glm::ivec2 center, int radius, Tile tile = Tile::Empty());
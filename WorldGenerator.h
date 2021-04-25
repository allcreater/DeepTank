#pragma once

#include "World.h"
#include <noise/noise.h>

class WorldGenerator
{
public:
    WorldGenerator(glm::uvec2 horizontalDimensions);

    glm::uvec2 getLayerDimensions() const { return horizontalDimensions; }
    void generateLevelLayer(LevelLayer &currentLayer);

 private:
    glm::uvec2 horizontalDimensions;
};

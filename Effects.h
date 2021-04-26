#pragma once

#include "Actor.h"
#include "World.h"

//[](Effect& effect, World &world)
//                                         {
//    if (auto *layer = world.getLayer(effect.getPosition().z))
//        FillRoundArea(*layer, glm::ivec2{effect.getPositionOnLayer()}, 4);
//                                         })
//
//namespace Actions
//{
//    std::function<void(Effect & effect, World & world)> Make
//}

#include "stdafx.h"
#include "Actor.h"
#include "World.h"

void Character::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(sprite, states);
}

void Character::update(float dt, World &world)
{
    sprite.setPosition(position.x, position.y);

    const auto tileBeneath = world.categorizeTile(glm::ivec3{position});
    if (tileBeneath != World::CellType::Floor)
        position.z += 1.0f * dt;

    const auto newPos = glm::vec3{getPositionOnLayer() + getVelocity() * dt, getPosition().z};
    const auto tileAhead = world.categorizeTile(glm::ivec3{newPos});
    if (tileAhead == World::CellType::Floor)
        setPosition(newPos);
    else
        velocity = {};

}

void Tank::update(float dt, World &world)
{
    Character::update(dt, world);
}

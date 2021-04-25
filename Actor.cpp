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
    sprite.setOrigin(sprite.getTexture()->getSize().x / 2, sprite.getTexture()->getSize().y / 2);

    const auto maxDimension = std::max(sprite.getTexture()->getSize().x, sprite.getTexture()->getSize().y);
    const auto scale = static_cast<float>(getSize()) / maxDimension;
    sprite.setScale(scale, scale);

    const auto tileBeneath = world.categorizeTile(glm::ivec3{position});
    if (tileBeneath == World::CellType::Unloaded)
        return;

    if (tileBeneath == World::CellType::Empty)
        position.z += 1.0f * dt;

    const auto newPos = glm::vec3{getPositionOnLayer() + getVelocity() * dt, getPosition().z};
    const auto tileAhead = world.categorizeTile(glm::ivec3{newPos});
    if (tileAhead == World::CellType::Floor)
        setPosition(newPos);
    else
        velocity = {};

}

void Tank::onReady(World &world)
{
    auto *layer = world.getLayer(getPosition().z);
    assert(layer);

    glm::ivec2 intPos{getPosition()};
    layer->visit([](glm::ivec2 pos, Tile &tile) {
        tile = Tile::Empty();
    }, intPos - glm::ivec2{getSize() * 4}, intPos + glm::ivec2{getSize() * 4});
}

void Tank::update(float dt, World &world)
{
    Character::update(dt, world);
}

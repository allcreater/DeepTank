#include "stdafx.h"
#include "Actor.h"
#include "World.h"

#include "WorldGenerator.h" // For drawing on level

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

    if (length(velocity) >= 1.0f)
    {
        const auto angle = glm::degrees(atan2f(velocity.y, velocity.x));
        sprite.setRotation(angle + 90.0f);
    }

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

void Tank::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(sprite, states);
    target.draw(drillSprite, states);
    target.draw(towerSprite, states);
}

void Tank::onReady(World &world)
{
    auto *layer = world.getLayer(getPosition().z);
    assert(layer);

    FillRoundArea(*layer, getPositionOnLayer(), getSize() * 4);
}

void Tank::update(float dt, World &world)
{
    Character::update(dt, world);

    const auto *towerTexture = towerSprite.getTexture();
    const auto *drillTexture = drillSprite.getTexture();

    towerSprite = sprite;
    towerSprite.setTexture(*towerTexture);

    const auto angle = glm::radians(sprite.getRotation() - 90.0f);

    drillSprite = sprite;
    drillSprite.setPosition(drillSprite.getPosition() + sf::Vector2f{cos(angle), sin(angle)} * 2.0f );
    drillSprite.setTexture(*drillTexture);
}

void Effect::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.blendMode = sf::BlendMode{sf::BlendMode::SrcAlpha, sf::BlendMode::One};
    target.draw(sprite, states);
}

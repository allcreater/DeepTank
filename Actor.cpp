#include "stdafx.h"
#include "Actor.h"
#include "World.h"

#include "WorldGenerator.h" // For drawing on level

void Character::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(sprite, states);
}

float Character::directionToAngle(glm::vec2 dir)
{
    if (length(dir) > 0.001f)
    {
        return glm::degrees(atan2f(dir.y, dir.x)) + 90.0f;
    }

    return 0;
}

void Character::updateWeapon(float dt, World &world)
{
    if (shootTrigger && activeWeapon < weaponList.size())
    {
        auto &weapon = weaponList[activeWeapon];
        if (weapon.reloadTimer <= 0.01f && weapon.amunition > 0)
        {
            if (auto effect = weapon.effectFactory(*this, shootDirection))
            {
                weapon.amunition--;
                weapon.reloadTimer = weapon.reloadTime;

                world.addActor(std::move(effect));
            }
        }
    }

    for (auto& weapon : weaponList)
    {
        weapon.reloadTimer = std::max(0.0f, weapon.reloadTimer - dt);
    }

    shootTrigger = false;
}

void Character::update(float dt, World &world)
{
    sprite.setPosition(position.x, position.y);
    sprite.setOrigin(sprite.getTexture()->getSize().x / 2, sprite.getTexture()->getSize().y / 2);

    const auto maxDimension = std::max(sprite.getTexture()->getSize().x, sprite.getTexture()->getSize().y);
    const auto scale = static_cast<float>(getSize()) / maxDimension;
    sprite.setScale(scale, scale);
    sprite.setRotation(glm::degrees(rotation) + 90.0f);

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

    updateWeapon(dt, world);

}

void Tank::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    target.draw(sprite, states);
    target.draw(drillSprite, states);
    target.draw(towerSprite, states);
}

void Base::onReady(World &world)
{
    auto *layer = world.getLayer(getPosition().z);
    assert(layer);

    FillRoundArea(*layer, getPositionOnLayer(), getSize());
}

void Effect::update(float dt, World &world)
{
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    size += sizeVelocity * dt;
    lifetime -= dt;

    sprite.setRotation(sprite.getRotation() + angularVelocity * dt);
    sprite.setPosition(position.x, position.y);
    sprite.setOrigin(sprite.getTexture()->getSize().x / 2, sprite.getTexture()->getSize().y / 2);
    sprite.setColor(sf::Color{255, 255, 255, static_cast<sf::Uint8>(255 * lifetime / initialLifetime)});

    const auto maxDimension = std::max(sprite.getTexture()->getSize().x, sprite.getTexture()->getSize().y);
    const auto scale = static_cast<float>(size) / maxDimension;
    sprite.setScale(scale, scale);
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

    towerSprite.setOrigin(sprite.getOrigin() + sf::Vector2f{0.0, 50.0f});
    towerSprite.setPosition(sprite.getPosition());
    towerSprite.setScale(sprite.getScale());
    towerSprite.setRotation(directionToAngle(shootDirection));

    const auto angle = glm::radians(sprite.getRotation() - 90.0f);

    drillSprite.setOrigin(sprite.getOrigin());
    drillSprite.setScale(sprite.getScale());
    drillSprite.setRotation(sprite.getRotation());
    drillSprite.setPosition(sprite.getPosition() + sf::Vector2f{cos(angle), sin(angle)} * 2.0f );
}

void Effect::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.blendMode = sf::BlendMode{sf::BlendMode::SrcAlpha, sf::BlendMode::One};
    target.draw(sprite, states);
}

void Bullet::update(float dt, World &world)
{
    Effect::update(dt, world);

    const auto currentTileType = world.categorizeTile(getPosition());
    if (currentTileType == World::CellType::Wall)
    {
        lifetime = 0.0f;
        if (payload)
        {
            payload->setPosition(getPosition());
            world.addActor(std::move(payload));
        }
    }
}

#pragma once

#include <SFML/Graphics.hpp>

class World;

class Actor : public sf::Drawable // TODO: ActorRenderer, remove shared_from_this
{
public:
    virtual void update(float dt, World &world) = 0;

    virtual void onReady(World& world){}
    virtual void onDestroy(World &world) {}

    virtual void setSize(uint8_t size) = 0;
    virtual uint8_t getSize() const = 0;

    virtual void setPosition(glm::vec3 position) = 0;
    virtual glm::vec3 getPosition() const = 0;

    bool isAlive() const { return world && isAliveImpl(); }

    void setWorld(const World *_world) { world = _world; }
    const World *getWorld() const { return world; }

    virtual ~Actor() = default;

protected:

    virtual bool isAliveImpl() const = 0;
private:
    const World *world = nullptr;

    //virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override {}
};

struct Weapon
{
    std::function<std::unique_ptr<class Effect>(class Character& instigator, glm::vec2 direction)> effectFactory;

    float reloadTime = 0.2f;
    int amunition = 100;
    int fullAmunition = 100;
    float reloadTimer = 0.0f;
};

class Character : public Actor
{
public:
    void setMaxSpeed(float speed) { maxSpeed = speed; }
    float getMaxSpeed() const { return maxSpeed; }


    void setVelocity(glm::vec2 _velocity) { velocity = _velocity; }
    glm::vec2 getVelocity() const { return velocity; }

    void setRotation(float angle) { rotation = angle; }
    float getRotation() const { return rotation; }
    glm::vec2 getFrontDirection() const { return {cos(rotation), sin(rotation)}; }

    void update(float dt, World &world) override;

    void setSize(uint8_t _size) override { size = _size; }
    uint8_t getSize() const override { return size; }

    void setPosition(glm::vec3 _position) override { position = _position; }
    glm::vec3 getPosition() const override { return position; }
    glm::vec2 getPositionOnLayer() const { return {position.x, position.y}; }

    size_t getActiveWeapon() const { return activeWeapon; }
    void setActiveWeapon(size_t weaponIndex) { activeWeapon = weaponIndex; }
    void triggerShoot() { shootTrigger = true; }
    void setShootDirection(glm::vec2 _dir) { shootDirection = _dir; }

    std::vector<Weapon> &getWeaponList() { return weaponList; }

    void damage(float amount) { hp -= amount; }
    void setHP(float _hp) { hp = _hp; }
    float getHP() const { return hp; }

    bool isAliveImpl() const override { return hp > 0.0f; }

    void setTexture(const sf::Texture &texture) { sprite.setTexture(texture); }

    void onReady(World &world) override;

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    static float directionToAngle(glm::vec2 dir);

protected:
    sf::Sprite sprite;
    float rotation = 0.0f;

    size_t activeWeapon = 0;
    bool shootTrigger = false;
    std::vector<Weapon> weaponList;
    glm::vec2 shootDirection;

private:
    void updateWeapon(float dt, World &world);

private:
    glm::vec3 position = {};
    glm::vec2 velocity = {};
    float hp = 1.0f;
    uint8_t size = 1;
    float maxSpeed = 1.0;
};


class Tank : public Character
{
public:
    struct Inventory
    {
        int amountMinerals = 0;
        int amountOil = 0;
    } inventory;

public:
    void onReady(World &world) override;
    void update(float dt, World &world) override;

    void setAdditionalTextures(const sf::Texture &towerTexture, const sf::Texture &drillTexture)
    {
        towerSprite.setTexture(towerTexture);
        drillSprite.setTexture(drillTexture);
    }

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Sprite towerSprite;
    sf::Sprite drillSprite;
};

class Base : public Character
{
public:
    void onReady(World &world) override;
    void update(float dt, World &world) override;
};


//very dumb and inefficient particles, but we must hurry
//probably will do some game logic on appear
class Effect : public Actor
{
public:
    /*
     *     glm::vec3 position = {};
    glm::vec2 velocity = {};
    sf::Sprite sprite;
    float lifetime = 1.0f;

    float angularVelocity = 0.0f;
    float sizeVelocity = 0.0f;
    float size = 1.0f;

    std::function<void(World &)> onAppear;
     */
    Effect(glm::vec3 position, glm::vec2 velocity, float lifetime, float angularVelocity = 0.0f, float sizeVelocity = 0.0f, float initialSize = 1.0f, std::function<void(Effect&, World &)> onAppear = {}) :
        position{position},
        velocity{velocity}, lifetime{lifetime}, angularVelocity{angularVelocity},
        sizeVelocity{sizeVelocity}, size{initialSize}, onAppear {std::move(onAppear)} {}
    Effect() = default;

    void setVelocity(glm::vec2 _velocity) { velocity = _velocity; }
    glm::vec2 getVelocity() const { return velocity; }

    void update(float dt, World &world) override;

    void setSize(uint8_t _size) override { size = _size; }
    void setSize(float _size) { size = _size; }
    uint8_t getSize() const override { return size; }

    void setPosition(glm::vec3 _position) override { position = _position; }
    glm::vec3 getPosition() const override { return position; }
    glm::vec2 getPositionOnLayer() const { return {position.x, position.y}; }

    bool isAliveImpl() const override { return lifetime > 0.0 && size > 0.0f; }

    void setTexture(const sf::Texture &texture) { sprite.setTexture(texture); }

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    void onReady(World &world) override { if (onAppear) onAppear(*this, world); }

protected:
    glm::vec3 position = {};
    glm::vec2 velocity = {};
    sf::Sprite sprite;
    float lifetime = 1.0f;
    float initialLifetime = lifetime;

    float angularVelocity = 0.0f;
    float sizeVelocity = 0.0f;
    float size = 1.0f;

    std::function<void(Effect&,World &)> onAppear;
};

class Bullet : public Effect
{
public:
    Bullet() = default;
    Bullet(glm::vec3 position, glm::vec2 velocity)
    {
        setPosition(position);
        setVelocity(velocity);
    }
    void update(float dt, World &world) override;

    void setPayload(std::unique_ptr<Effect> effect) { payload = std::move(effect); }

private:
    std::unique_ptr<Effect> payload;
};

class Enemy : public Character
{
public:
    void update(float dt, World &world) override;

    float getNearDamage() const { return nearDamage; }
    void setNearDamage(float damage) { nearDamage = damage; }

    int buildingRange = 0;
    std::weak_ptr<Character> chasingActor;

private:
    float nearDamage = 0.1;
};


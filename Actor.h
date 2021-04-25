#pragma once

#include <SFML/Graphics.hpp>

class World;

class Actor : public sf::Drawable //TODO: ActorRenderer
{
public:
    virtual void update(float dt, World &world) = 0;

    virtual void onReady(World& world){}
    virtual void onDestroy(World &world) {}

    virtual bool isAlive() const = 0;

    virtual void setSize(uint8_t size) = 0;
    virtual uint8_t getSize() const = 0;

    virtual void setPosition(glm::vec3 position) = 0;
    virtual glm::vec3 getPosition() const = 0;

    virtual ~Actor() = default;

    //virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override {}
};


class Character : public Actor
{
public:
    void setVelocity(glm::vec2 _velocity) { velocity = _velocity; }
    glm::vec2 getVelocity() const { return velocity; } 

    void update(float dt, World &world) override;

    void setSize(uint8_t _size) override { size = _size; }
    uint8_t getSize() const override { return size; }

    void setPosition(glm::vec3 _position) override { position = _position; }
    glm::vec3 getPosition() const override { return position; }
    glm::vec2 getPositionOnLayer() const { return {position.x, position.y}; }

    bool isAlive() const override { return hp > 0.0f; }

    void setTexture(const sf::Texture &texture) { sprite.setTexture(texture); }

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    glm::vec3 position = {};
    glm::vec2 velocity = {};
    sf::Sprite sprite;
    float hp = 1.0f;
    uint8_t size = 1;
};

class Tank : public Character
{
public:
    virtual void onReady(World &world);
    void update(float dt, World &world) override;

private:
};

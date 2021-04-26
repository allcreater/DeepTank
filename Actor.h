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

    void damage(float amount) { hp -= amount; }
    void setHP(float _hp) { hp = _hp; }
    float getHP() const { return hp; }

    bool isAlive() const override { return hp > 0.0f; }

    void setTexture(const sf::Texture &texture) { sprite.setTexture(texture); }

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

protected:
    sf::Sprite sprite;

private:
    glm::vec3 position = {};
    glm::vec2 velocity = {};
    float hp = 1.0f;
    uint8_t size = 1;
};

class Tank : public Character
{
public:
    virtual void onReady(World &world);
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

    void setVelocity(glm::vec2 _velocity) { velocity = _velocity; }
    glm::vec2 getVelocity() const { return velocity; }

    void update(float dt, World &world) override
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

    void setSize(uint8_t _size) override { size = _size; }
    void setSize(float _size) { size = _size; }
    uint8_t getSize() const override { return size; }

    void setPosition(glm::vec3 _position) override { position = _position; }
    glm::vec3 getPosition() const override { return position; }
    glm::vec2 getPositionOnLayer() const { return {position.x, position.y}; }

    bool isAlive() const override { return lifetime > 0.0 && size > 0.0f; }

    void setTexture(const sf::Texture &texture) { sprite.setTexture(texture); }

protected:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    void onReady(World &world) override { onAppear(*this, world); }

private:
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

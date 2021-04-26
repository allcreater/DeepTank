#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "SfmlEventHelper.h"
#include "WorldRenderer.h"
#include "World.h"
#include "Actor.h"

#include "WorldGenerator.h"

namespace
{
    void GatherResourcesAtRadius(LevelLayer &layer, glm::ivec2 center, int radius, int16_t gatherForce,
                                 std::function<void(glm::ivec2, TileClassId)> onGather)
    {
        layer.visit(
            [=](glm::ivec2 pos, Tile &tile) {
                if (auto dir = center - pos; sqrt(dir.x * dir.x + dir.y * dir.y) <= radius)
                {
                    tile.actualStrength = std::max(0, tile.actualStrength - gatherForce);
                    if (tile.actualStrength == 0)
                    {
                        onGather(pos, tile.classId);
                        tile = Tile::Empty();
                    }
                }
            },
            center - glm::ivec2{radius}, center + glm::ivec2{radius});
    }

    std::unordered_map<TileClassId, int> HarvestResources(World &world, glm::ivec3 pos, int radius, int16_t gatherForce)
    {
        std::unordered_map<TileClassId, int> harvest;

        auto *layer = world.getLayer(pos.z);
        if (layer)
            GatherResourcesAtRadius(*layer, glm::xy(pos), radius, gatherForce, [&harvest](glm::ivec2, TileClassId tile) { harvest[tile]++;});

        return harvest;
    }

    std::unique_ptr<Effect> MakeExplosionEffect(glm::vec3 position, int radius, const sf::Texture& texture, float gatherForce = 1.0f)
    {
        auto effect = std::make_unique<Effect>(position, glm::vec2{}, 0.1f, 0.0f, 200.0, 4.0f, [=](Effect &effect, World &world) {
           if (auto *layer = world.getLayer(effect.getPosition().z))
           {
               auto harvest = HarvestResources(world, effect.getPosition(), radius, gatherForce);
           }
       });
        effect->setTexture(texture);
        return effect;
    }

} // namespace

const static auto title = "Deep.Drill.Tank. "s; 

class App
{
public:
    App ()
    {
        
    }

    void Run()
    {
        window.create(sf::VideoMode(1280, 800), title);

        Init();

        auto prevTime = std::chrono::high_resolution_clock::now();

        sf::Clock performanceCounterClock;
        size_t fps = 0;
        while (window.isOpen())
        {
            // Process events
            for (sf::Event event; window.pollEvent(event); )
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                else
                    OnWindowEvent(sf::Utils::MakeTypeSafeEvent(event));
            }

            const auto newTime = std::chrono::high_resolution_clock::now();
            const float dt = std::chrono::duration_cast<std::chrono::microseconds>(newTime - prevTime).count() / 1000000.0f;
            prevTime = newTime;

            Update(dt);
            Render();

            if (performanceCounterClock.getElapsedTime().asSeconds() >= 1.0f)
            {
                window.setTitle(title + std::to_string(fps) + " fps");

                fps = 0;
                performanceCounterClock.restart();
            }
            else
                fps++;
        }
    }

private:
    static void loadTextureOrThrow(sf::Texture& texture, const std::string& name)
    {
        if (!texture.loadFromFile(name))
            throw std::runtime_error{"texture "s + name + " can't be loaded"s};
    }

    void OnWindowEvent(sf::Utils::SfmlEvent event)
    {
        using namespace sf::Utils;
        std::visit(overloaded{
            [&](KeyPressed key)
            {
              if (key.code == sf::Keyboard::E)
                  visibleLayer++;
              else if (key.code == sf::Keyboard::Q)
                  visibleLayer--;
            },
            [&](MouseButtonPressed button)
            {
                //if (button.button == sf::Mouse::Left)
            },
            [](auto){}
        }, event);
    }

    void Init()
    {
        tilesAtlas = TextureAtlas::MakeFromRegularGrid("Resources/tiles2.png", {16, 16}, 11);

        loadTextureOrThrow(tankTexture, "Resources/only_tank.png");
        loadTextureOrThrow(tankTowerTexture, "Resources/only_tower.png");
        loadTextureOrThrow(tankDrillTexture, "Resources/drill.png");

        loadTextureOrThrow(flameTexture, "Resources/effect_flame.png");
        loadTextureOrThrow(glowTexture, "Resources/effect_glow.png");

        world = std::make_unique<World>();
        world->setGenerator(std::make_shared<WorldGenerator>(glm::uvec2{256, 256}));


        {
            auto tankActor = std::make_unique<Tank>();
            tankActor->setTexture(tankTexture);
            tankActor->setSize(4);
            tankActor->setPosition({128, 128, 0.0});
            tankActor->setAdditionalTextures(tankTowerTexture, tankDrillTexture);

            //cannon
            tankActor->getWeaponList().emplace_back([&](Actor *instigator, glm::vec2 direction)
            {
                auto bullet = std::make_unique<Bullet>();
                bullet->setVelocity(direction * 100.0f);
                bullet->setPayload(MakeExplosionEffect(glm::vec3{}, 4, flameTexture));
                bullet->setTexture(glowTexture);
                return bullet;
            });

            //drill
            tankActor->getWeaponList().emplace_back([&](Actor *instigator, glm::vec2 direction) {
                return MakeExplosionEffect(glm::vec3{}, 2, flameTexture, 3.0f);
            }, 0.1f, std::numeric_limits<int>::max());

            playerActor = tankActor.get();
            world->addActor(std::move(tankActor));
        }

        worldRenderer = std::make_unique<WorldRenderer>(*world, tilesAtlas);
    }

    void Update(float dt)
    {
        //const auto cameraSpeed = 1000.0f * dt;
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        //    cameraPosition.y -= cameraSpeed;
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        //    cameraPosition.y += cameraSpeed;
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        //    cameraPosition.x -= cameraSpeed;
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        //    cameraPosition.x += cameraSpeed;

        glm::vec2 velocity{};
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        //    velocity += glm::vec2{0.0f, -40.0f};
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        //    velocity += glm::vec2{0.0f, 40.0f};
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        //    velocity += glm::vec2{-40.0f, 0.0f};
        //if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        //    velocity += glm::vec2{40.0f, 0.0f};




        sf::View view{{cameraPosition.x, cameraPosition.y},
                      {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)}};
        window.setView(view);

        {
            const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            const auto pos = worldRenderer->getInverseTransform().transformPoint(mousePos);

            const auto directionToMouse = [&]()
            {
                const auto dir = to_glm(pos) - playerActor->getPositionOnLayer();
                return length(dir) > 0 ? dir / length(dir) : glm::vec2{};
            }();

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                velocity = directionToMouse * 10.0f;
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
            {
                playerActor->requestShootTo(1);
                //auto bullet = std::make_unique<Bullet>(playerActor->getPosition(), directionToMouse * 90.0f);
                //bullet->setPayload(MakeExplosionEffect(glm::vec3{to_glm(pos), visibleLayer}, 4, flameTexture));
                //bullet->setTexture(glowTexture);
                //world->addActor(std::move(bullet));
            }
            playerActor->setShootDirection(directionToMouse);
        }

        cameraPosition = worldRenderer->getTransform().transformPoint(playerActor->getPosition().x,
                                                                   playerActor->getPosition().y);

        playerActor->setVelocity(velocity);


        world->Update(dt);

        worldRenderer->setCameraPosition(cameraPosition);
        worldRenderer->setScale(12.0, 12.0);
        worldRenderer->setVisibleLayers(visibleLayer, 16);
        worldRenderer->update();
    }

    void Render()
    {
        // Clear screen
        window.clear(sf::Color::Magenta);

        window.draw(*worldRenderer);

        window.display();
    }

private:
    sf::RenderWindow window;
    TextureAtlas tilesAtlas;
    sf::Texture tankTexture, tankTowerTexture, tankDrillTexture;
    sf::Texture flameTexture, glowTexture;

    std::unique_ptr<World> world;

    std::unique_ptr<WorldRenderer> worldRenderer;
    sf::Vector2f cameraPosition = {128, 128};
    int visibleLayer = 0;

    Character *playerActor = nullptr;
};

int main()
{
    App app;
    app.Run();

    return EXIT_SUCCESS;
}
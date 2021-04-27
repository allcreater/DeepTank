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

    using ResourceSet = std::unordered_map<TileClassId, int>;
    ResourceSet HarvestResources(World &world, glm::ivec3 pos, int radius, int16_t gatherForce)
    {
        std::unordered_map<TileClassId, int> harvest;

        auto *layer = world.getLayer(pos.z);
        if (layer)
            GatherResourcesAtRadius(*layer, glm::xy(pos), radius, gatherForce, [&harvest](glm::ivec2, TileClassId tile) { harvest[tile]++;});

        return harvest;
    }

    void HarvestToInventory(ResourceSet &&harvest, Tank::Inventory &inventory, std::span<const TileClass> classes)
    {
        for (auto [tileClassId, amount] : harvest)
        {
            inventory.amountMinerals += classes[tileClassId].value;
        }

        inventory.amountOil += harvest[10];
    }

    std::unique_ptr<Effect> MakeExplosionEffect(glm::vec3 position, int radius, const sf::Texture& texture, float gatherForce = 1.0f, float damage = 0.5f, std::shared_ptr<Tank> gatherer = {})
    {
        auto effect = std::make_unique<Effect>(position, glm::vec2{}, 0.1f, 0.0f, 200.0, 4.0f, [=](Effect &effect, World &world) {
           if (auto *layer = world.getLayer(effect.getPosition().z))
           {
               auto harvest = HarvestResources(world, effect.getPosition(), radius, gatherForce);
               if (gatherer)
                HarvestToInventory(std::move(harvest), gatherer->inventory, world.getGenerator()->getClasses());
           }

            auto &collisions = world.queryPoint(effect.getPosition());
            for (auto *object : collisions | std::ranges::views::filter([](auto *x) { return dynamic_cast<Character *>(x); }))
            {
                if (object != gatherer.get())
                    static_cast<Character *>(object)->damage(damage);
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
        window.setVerticalSyncEnabled(true);

        Init();

        auto prevTime = std::chrono::high_resolution_clock::now();

        float averageDt = 0.0f;

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

            averageDt = glm::mix(averageDt, dt, 0.01f);
            Update(averageDt);
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
                if (key.code == sf::Keyboard::F10)
                    StartNewGame();
                else if (key.code == sf::Keyboard::F1)
                {
                    auto* layer = world->getLayer(playerActor->getPosition().z + 1);
                    FillRoundArea(*layer, playerActor->getPositionOnLayer(), 20);
                }
                else if (key.code == sf::Keyboard::Q)
                {
                    if (playerActor)
                        playerActor->setActiveWeapon((playerActor->getActiveWeapon() + 1) % 2);
                }
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
        loadTextureOrThrow(baseTexture, "Resources/basa.png");
        loadTextureOrThrow(smallEnemyTexture, "Resources/40leggs.png");
        loadTextureOrThrow(bigEnemyTexture, "Resources/scorp.png");

        loadTextureOrThrow(flameTexture, "Resources/effect_flame.png");
        loadTextureOrThrow(glowTexture, "Resources/effect_glow.png");

        if (!font.loadFromFile("Resources/third-party/Nasa21-l23X.ttf"))
            throw std::runtime_error{"font could'nt be loaded"s};

        StartNewGame();
    }

    void StartNewGame()
    {
        world = std::make_unique<World>();
        world->setGenerator(std::make_shared<WorldGenerator>(glm::uvec2{256, 256}));

        {
            baseActor = std::make_unique<Base>();
            baseActor->setTexture(baseTexture);
            baseActor->setSize(15);
            baseActor->setPosition({128, 128, 0.0});
            baseActor->setHP(100.0);

            world->addActor(baseActor);
        }

        {
            playerActor = std::make_unique<Tank>();
            playerActor->setTexture(tankTexture);
            playerActor->setMaxSpeed(10.0f);
            playerActor->setSize(2);
            playerActor->setPosition({128, 128, 0.0});
            playerActor->setAdditionalTextures(tankTowerTexture, tankDrillTexture);

            //cannon
            playerActor->getWeaponList().emplace_back(
                [&](Character &instigator, glm::vec2 direction)
            {
                auto bullet = std::make_unique<Bullet>(instigator.getPosition(), instigator.getVelocity() + direction * 100.0f);
                bullet->setPosition(instigator.getPosition() +
                    glm::vec3{direction * static_cast<float>(instigator.getSize() * 2.0f), 0.0f});
                bullet->setPayload(MakeExplosionEffect(glm::vec3{}, 6, flameTexture, 6, 0.5f));
                bullet->setTexture(glowTexture);
                return bullet;
            }, 0.6f);

            //drill
            playerActor->getWeaponList().emplace_back(
                [&](Character &instigator, glm::vec2 direction) {
                    return MakeExplosionEffect(instigator.getPosition() +
                            glm::vec3{instigator.getFrontDirection() * static_cast<float>(instigator.getSize() * 1.0f), 0.0f},
                                               2, flameTexture, 3.0f,0.1f, playerActor);
            }, 0.1f, std::numeric_limits<int>::max());

            world->addActor(playerActor);
        }


        auto generateSafePos = [this]()
        {
            std::uniform_real_distribution<float> posDistribution{-32, 256 - 32};

            glm::vec2 pos{};
            do
            {
                pos = {posDistribution(random), posDistribution(random)};
            } while (length(pos - glm::vec2{128, 128}) <= 30.0);

            return pos;
        };

        {
            for (int i = 0; i < 100; ++i)
            {
                auto actor = std::make_shared<Enemy>();
                actor->setTexture(smallEnemyTexture);
                actor->setSize(2);
                actor->setPosition({generateSafePos(), 0.0});
                //actor->setPosition({120,120, 0.0});
                actor->setHP(0.5f);
                actor->setMaxSpeed(2.0);
                actor->chasingActor = playerActor;

                world->addActor(std::move(actor));
            }

            for (int i = 0; i < 10; ++i)
            {
                auto actor = std::make_shared<Enemy>();
                actor->setTexture(bigEnemyTexture);
                actor->setMaxSpeed(0.5);
                actor->setSize(4);
                actor->setPosition({generateSafePos(), 0.0});
                // actor->setPosition({120,120, 0.0});
                actor->setHP(20.0f);
                actor->chasingActor = playerActor;
                actor->buildingRange = 2;

                world->addActor(std::move(actor));
            }
        }


        worldRenderer = std::make_unique<WorldRenderer>(*world, tilesAtlas);
    }

    void Update(float dt)
    {
        sf::View view{{cameraPosition.x, cameraPosition.y},
                      {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)}};
        window.setView(view);

        // player input
        if (playerActor && playerActor->isAlive())
        {
            constexpr auto rotateSpeed = 3.0f;

            glm::vec2 velocity{};
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                velocity = playerActor->getFrontDirection() * playerActor->getMaxSpeed();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                velocity = playerActor->getFrontDirection() * playerActor->getMaxSpeed() * -0.5f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                playerActor->setRotation(playerActor->getRotation() - rotateSpeed * dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                playerActor->setRotation(playerActor->getRotation() + rotateSpeed * dt);

            playerActor->setVelocity(velocity);

            {
                const auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                const auto pos = worldRenderer->getInverseTransform().transformPoint(mousePos);

                const auto directionToMouse = [&]() {
                    const auto dir = to_glm(pos) - playerActor->getPositionOnLayer();
                    return length(dir) > 0 ? dir / length(dir) : glm::vec2{};
                }();

                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    //velocity = directionToMouse * 10.0f;
                }

                if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                {
                    //playerActor->setActiveWeapon(0);
                    playerActor->triggerShoot();
                    // auto bullet = std::make_unique<Bullet>(playerActor->getPosition(), directionToMouse * 90.0f);
                    // bullet->setPayload(MakeExplosionEffect(glm::vec3{to_glm(pos), visibleLayer}, 4, flameTexture));
                    // bullet->setTexture(glowTexture);
                    // world->addActor(std::move(bullet));
                }
                playerActor->setShootDirection(directionToMouse);
            }

            cameraPosition = worldRenderer->getTransform().transformPoint(playerActor->getPosition().x,
                                                                          playerActor->getPosition().y);
            visibleLayer = playerActor->getPosition().z;

            playerActor->setVelocity(velocity);

            world->trimLevelsAbove(playerActor->getPosition().z-1);
        }

        if (!world)
            return;

        world->Update(dt);

        worldRenderer->setCameraPosition(cameraPosition);
        worldRenderer->setScale(12.0, 12.0);
        worldRenderer->setVisibleLayers(visibleLayer, 16);
        worldRenderer->update();
    }

    void Render()
    {
        // Clear screen
        window.clear(sf::Color::Black);

        if (playerActor->isAlive() && baseActor->isAlive())
        {
            window.draw(*worldRenderer);

            window.setView(window.getDefaultView());

            sf::Text text{"Resources:\n "s + std::to_string(playerActor->inventory.amountMinerals) + " minerals\n "s + 
                std::to_string(playerActor->inventory.amountOil) + " oil.\n\n Base structure: "s + std::to_string(baseActor->getHP())
                , font, 30};

            window.draw(text);
        }
        else
        {
            window.setView(window.getDefaultView());

            sf::Text text{"Game over", font, 80};
            sf::Vector2u position = window.getSize() / 2u - sf::Vector2u{static_cast<unsigned>(text.getGlobalBounds().width), static_cast<unsigned>(text.getGlobalBounds().height)};
            text.setPosition(position.x, position.y);
            window.draw(text);
        }

        window.display();
    }

private:
    sf::RenderWindow window;
    TextureAtlas tilesAtlas;
    sf::Texture tankTexture, tankTowerTexture, tankDrillTexture;
    sf::Texture baseTexture;
    sf::Texture flameTexture, glowTexture;
    sf::Texture smallEnemyTexture, bigEnemyTexture;

    sf::Font font;

    std::mt19937 random;
    std::unique_ptr<World> world;

    std::unique_ptr<WorldRenderer> worldRenderer;
    sf::Vector2f cameraPosition = {128, 128};
    int visibleLayer = 0;

    std::shared_ptr<Tank> playerActor = nullptr;
    std::shared_ptr<Base> baseActor = nullptr;
};

int main()
{
    App app;
    app.Run();

    return EXIT_SUCCESS;
}
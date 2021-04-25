#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "SfmlEventHelper.h"
#include "WorldRenderer.h"
#include "World.h"
#include "Actor.h"

#include <chrono>

#include "WorldGenerator.h"

class App
{
public:
    App ()
    {
        
    }

    void Run()
    {
        window.create(sf::VideoMode(1280, 800), "");

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
                window.setTitle("Deep Tank "s + std::to_string(fps) + " fps");

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
            [](auto){}
                   }, event);
    }

    void Init()
    {
        tilesAtlas = TextureAtlas::MakeFromRegularGrid("Resources/tiles.png", {12, 12}, 6);

        loadTextureOrThrow(tankTexture, "Resources/tank.png");

        world = std::make_unique<World>();
        world->setGenerator(std::make_shared<WorldGenerator>(glm::uvec2{256, 256}));


        {
            auto tankActor = std::make_unique<Tank>();
            tankActor->setTexture(tankTexture);
            tankActor->setSize(4);
            tankActor->setPosition({128, 128, 0.0});

            playerActor = tankActor.get();
            world->addActor(std::move(tankActor));
        }

        worldRenderer = std::make_unique<WorldRenderer>(*world, tilesAtlas);
    }

    void Update(float dt)
    {
        const auto cameraSpeed = 1000.0f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            cameraPosition.y -= cameraSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cameraPosition.y += cameraSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cameraPosition.x -= cameraSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cameraPosition.x += cameraSpeed;

        glm::vec2 velocity{};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            velocity += glm::vec2{0.0f, -4.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            velocity += glm::vec2{0.0f, 4.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            velocity += glm::vec2{-4.0f, 0.0f};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            velocity += glm::vec2{4.0f, 0.0f};

        playerActor->setVelocity(velocity);


        sf::View view{{cameraPosition.x, cameraPosition.y},
                      {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)}};
        window.setView(view);


        world->Update(dt);

        worldRenderer->setCameraPosition(cameraPosition);
        worldRenderer->setScale(8.0, 8.0);
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
    sf::Texture tankTexture;

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
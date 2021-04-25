#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "SfmlEventHelper.h"
#include "WorldRenderer.h"
#include "World.h"

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

        tankSprite.setTexture(tankTexture);
        tankSprite.setScale(1 / 16.0, 1 / 16.0);
        tankSprite.setPosition(128, 128);

        world = std::make_unique<World>();
        world->setGenerator(std::make_unique<WorldGenerator>(glm::uvec2{256, 256}));

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


        sf::View view{{cameraPosition.x, cameraPosition.y},
                      {static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)}};
        window.setView(view);


        world->Update(dt);

        worldRenderer->setCameraPosition(cameraPosition);
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

    sf::Sprite tankSprite;

    std::unique_ptr<World> world;

    std::unique_ptr<WorldRenderer> worldRenderer;
    sf::Vector2f cameraPosition = {128, 128};
    int visibleLayer = 0;
};

int main()
{
    App app;
    app.Run();

    return EXIT_SUCCESS;
}
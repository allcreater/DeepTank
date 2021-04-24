#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "World.h"

class App
{
public:
    App ()
    {
        
    }

    void Run()
    {
        window.create(sf::VideoMode(1280, 800), "Deep Tank");

        Init();

        sf::Clock clock;
        while (window.isOpen())
        {
            // Process events
            for (sf::Event event; window.pollEvent(event); )
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            Update(clock.getElapsedTime().asSeconds());
            Render();

            clock.restart();
        }
    }

private:
    void Init()
    {
        if (!tilesTexture.loadFromFile("Resources/tiles.png"))
            return;

        if (!tankTexture.loadFromFile("Resources/tank.png"))
            return;

        tankSprite.setTexture(tankTexture);
        tankSprite.setScale(1 / 16.0, 1 / 16.0);
        tankSprite.setPosition(128, 128);

        world = std::make_unique<World>(&tilesTexture);

        for (size_t i = 3; i > 0; --i)
        {
            auto &renderer = renderers.emplace_back(&tilesTexture);
            renderer.update(*world->getLayer(i), *world);
        }
    }

    void Update(float dt)
    {
        const auto aspectRatio = static_cast<float>(window.getSize().y) / window.getSize().x;

        const float range = 64;
        sf::View view{{128, 128}, {range, range * aspectRatio}};
        window.setView(view);
    }

    void Render()
    {
        // Clear screen
        window.clear(sf::Color::Magenta);

        sf::RenderStates states;
        for (const auto &renderer : renderers)
        {
            //states.transform.scale(1.1, 1.1);
            window.draw(renderer);
        }

        window.draw(tankSprite);

        window.display();
    }

private:
    sf::RenderWindow window;
    sf::Texture tilesTexture, tankTexture;

    sf::Sprite tankSprite;

    std::unique_ptr<World> world;
    std::vector<LayerRenderer> renderers;
};

int main()
{
    App app;
    app.Run();

    return EXIT_SUCCESS;
}
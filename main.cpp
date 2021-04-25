#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "SfmlEventHelper.h"
#include "WorldRenderer.h"
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
                else
                    OnWindowEvent(sf::Utils::MakeTypeSafeEvent(event));
            }

            Update(clock.getElapsedTime().asSeconds());
            Render();

            clock.restart();
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
            [this](ResizedEvent event)
            {
                  sf::View view{
                      {128, 128},
                      {static_cast<float>(event.width), static_cast<float>(event.height)}};
                  window.setView(view);
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

        for (size_t i = 3; i > 0; --i)
        {
            auto &renderer = renderers.emplace_back();
            renderer.update(*world->getLayer(i), tilesAtlas);
        }
    }

    void Update(float dt)
    {
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
    TextureAtlas tilesAtlas;
    sf::Texture tankTexture;

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
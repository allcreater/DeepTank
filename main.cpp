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

        for (size_t i = 0; i < 5; ++i)
        {
            auto &renderer = renderers.emplace_back();
            const uint8_t intensity = 255 / (i + 1);
            renderer.setBaseColor(sf::Color{intensity, intensity, intensity, 255});
            renderer.update(*world->getLayer(i), tilesAtlas);
        }
    }

    void Update(float dt)
    {
        const auto cameraSpeed = 1;
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
    }

    void Render()
    {
        // Clear screen
        window.clear(sf::Color::Magenta);

        for (int depth = static_cast<int>(renderers.size()) - 1; depth >= 0; --depth)
        {
            auto &renderer = renderers[depth];

            const auto scaleFactor = 1.0f / static_cast<float>(depth * 0.02f + 1);

            sf::RenderStates rs;
            rs.transform.translate(cameraPosition);
            rs.transform.scale(scaleFactor, scaleFactor);
            rs.transform.translate(-cameraPosition);

            window.draw(renderer, rs);

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

    sf::Vector2f cameraPosition = {128, 128};
};

int main()
{
    App app;
    app.Run();

    return EXIT_SUCCESS;
}
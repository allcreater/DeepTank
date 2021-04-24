#include "stdafx.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "World.h"

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(1280, 800), "SFML window");

    // Load a sprite to display

    sf::Texture texture;
    if (!texture.loadFromFile("Resources/tiles.png"))
        return EXIT_FAILURE;

    sf::Texture tankTexture;
    if (!tankTexture.loadFromFile("Resources/tank.png"))
        return EXIT_FAILURE;

    sf::Sprite sprite(tankTexture);
    sprite.setScale(1 / 16.0, 1 / 16.0);
    sprite.setPosition(128, 128);

    // Create a graphical text to display
    sf::Font font;

    World world {&texture};


    std::vector<LayerRenderer> renderers;
    for (size_t i = 3; i > 0; --i)
    {
        auto& renderer = renderers.emplace_back(&texture);
        renderer.update(*world.getLayer(i), world);
    }
    

    sf::Clock clock;
    int layer = 0;

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        const auto aspectRatio = static_cast<float>(window.getSize().y) / window.getSize().x;

        const float range = 64;
        sf::View view{{128, 128}, {range, range * aspectRatio}};
        window.setView(view);

        if (clock.getElapsedTime().asSeconds() > 1.0f)
        {
            //renderer.update(*world.getLayer(layer % 16), world);
            clock.restart();
        }

        // Clear screen
        window.clear(sf::Color::Magenta);

        sf::RenderStates states;
        for (const auto &renderer : renderers)
        {
            states.transform.scale(1.1, 1.1);
            window.draw(renderer);
        }

        window.draw(sprite);

        window.display();
    }
    return EXIT_SUCCESS;
}
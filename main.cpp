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

    sf::Sprite sprite(texture);
    // Create a graphical text to display
    sf::Font font;

    World world {&texture};

    LayerRenderer renderer{ world.tilesTexture };
    renderer.update(*world.getLayer(0), world);

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
        // Clear screen
        window.clear(sf::Color::Magenta);

        window.draw(renderer);

        window.draw(sprite);

        window.display();
    }
    return EXIT_SUCCESS;
}
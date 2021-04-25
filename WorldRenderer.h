#pragma once

#include "TextureAtlas.h"

class World;
class LevelLayer;

class LayerRenderer final : public sf::Drawable
{
public:
    void setBaseColor(sf::Color color) { baseColor = color; }
    void update(const LevelLayer &layer, TextureAtlas& tileAtlas);

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Texture *texture = nullptr;
    sf::Color baseColor = sf::Color::White;
    sf::VertexArray vertexArray{sf::Quads};
    sf::VertexBuffer vertexBuffer { sf::Quads};
};

class WorldRenderer final : public sf::Drawable, public sf::Transformable
{
public:
    WorldRenderer(World &world, TextureAtlas &tilesAtlas);

    void setCameraPosition(sf::Vector2f pos) { cameraPosition = pos; }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    World &world;
    TextureAtlas &tilesAtlas;
    sf::Vector2f cameraPosition;
    std::vector<LayerRenderer> renderers;
};
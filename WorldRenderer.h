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
};

class WorldRenderer final : public sf::Drawable, public sf::Transformable
{
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};
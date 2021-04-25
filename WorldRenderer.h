#pragma once

#include "TextureAtlas.h"

class World;
class LevelLayer;

class LayerRenderer final : public sf::Drawable
{
public:
    void setLayer(const LevelLayer *layer);

    void setAtlas(const TextureAtlas *atlas);

    void setBaseColor(sf::Color color);
    void update(bool force = false);

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    size_t lastKnownRevision = 0;
    const TextureAtlas *textureAtlas = nullptr;
    const LevelLayer *currentLayer = nullptr;

    sf::Color baseColor = sf::Color::White;
    sf::VertexArray vertexArray{sf::Quads};
    sf::VertexBuffer vertexBuffer { sf::Quads};
};

class WorldRenderer final : public sf::Drawable, public sf::Transformable
{
public:
    WorldRenderer(World &world, TextureAtlas &tilesAtlas);

    void setCameraPosition(sf::Vector2f pos) { cameraPosition = pos; }
    void setVisibleLayers(int _topLayer, int _numLayers = 16);

    void update();
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    int topLayer = 0, numVisibleLayers = 0;

    World &world;
    TextureAtlas &tilesAtlas;
    sf::Vector2f cameraPosition;
    std::vector<LayerRenderer> renderers;
};
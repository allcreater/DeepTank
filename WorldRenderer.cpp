#include "stdafx.h"
#include "WorldRenderer.h"
#include "Actor.h"
#include "World.h"

void LayerRenderer::setLayer(const LevelLayer *layer)
{
    if (currentLayer == layer)
        return;

    currentLayer = layer;
    lastKnownRevision = -1;
}

void LayerRenderer::setAtlas(const TextureAtlas *atlas)
{
    if (textureAtlas == atlas)
        return;

    textureAtlas = atlas;
    lastKnownRevision = -1;
}

void LayerRenderer::setBaseColor(sf::Color color)
{
    if (baseColor == color)
        return;

    baseColor = color;
    lastKnownRevision = -1;
}

void LayerRenderer::update(bool force)
{
    if (!currentLayer || !textureAtlas)
        return;

    if (!force && lastKnownRevision == currentLayer->getRevision() && lastKnownRevision != -1)
        return;

    lastKnownRevision = currentLayer->getRevision();

    const size_t bufferLength = currentLayer->getSize().x * currentLayer->getSize().y * 4;
    vertexArray.resize(bufferLength);
    
    size_t baseVertexIndex = 0;
    currentLayer->visit([&](glm::ivec2 pos, const Tile &tile) {
        const auto& region = textureAtlas->regions[tile.classId];
        const auto bottom = region.top + region.height;
        const auto right = region.left + region.width;

        const glm::vec2 lt = glm::vec2{pos} - 8.0f/12, bd = glm::vec2{pos} + 8.0f / 12;

        vertexArray[baseVertexIndex+0] = sf::Vertex{{lt.x, lt.y}, baseColor, sf::Vector2f{region.left, region.top}};
        vertexArray[baseVertexIndex+1] = sf::Vertex{{bd.x, lt.y}, baseColor, sf::Vector2f{right, region.top}};
        vertexArray[baseVertexIndex+2] = sf::Vertex{{bd.x, bd.y}, baseColor, sf::Vector2f{right,bottom}};
        vertexArray[baseVertexIndex+3] = sf::Vertex{{lt.x, bd.y}, baseColor, sf::Vector2f{region.left, bottom}};

        baseVertexIndex += 4;
    });

    if (vertexBuffer.getVertexCount() != vertexArray.getVertexCount())
        vertexBuffer.create(vertexArray.getVertexCount());
    vertexBuffer.update(&vertexArray[0], vertexArray.getVertexCount(), 0);
}

void LayerRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!currentLayer)
        return;

    states.texture = &textureAtlas->texture;
    target.draw(vertexBuffer, states);
}

WorldRenderer::WorldRenderer(World &world, TextureAtlas &tilesAtlas) : world{world}, tilesAtlas{tilesAtlas}
{
}

void WorldRenderer::setVisibleLayers(int _topLayer, int _numLayers)
{
    topLayer = _topLayer;
    numVisibleLayers = _numLayers;

    renderers.resize(_numLayers);

    for (size_t layerIndex = topLayer; layerIndex < topLayer + numVisibleLayers; ++layerIndex)
    {
        const auto *layer = world.getLayer(layerIndex);
        if (!layer)
            continue;

        const auto i = layerIndex - topLayer;
        auto &renderer = renderers[i];
        const uint8_t intensity = 255 / (i + 1);
        renderer.setBaseColor(sf::Color{intensity, intensity, intensity, 255});
        renderer.setAtlas(&tilesAtlas);
        renderer.setLayer(layer);
    }
}

void WorldRenderer::update()
{
    for (auto& renderer : renderers)
    {
        renderer.update();
    }
}

void WorldRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    const auto originalTransform = states.transform *= getTransform();

    for (int depth = static_cast<int>(renderers.size()) - 1; depth >= 0; --depth)
    {
        auto &renderer = renderers[depth];

        const auto scaleFactor = 1.0f / static_cast<float>(depth * 0.02f + 1);

        sf::Transform transform;
        transform.translate(cameraPosition);
        transform.scale(scaleFactor, scaleFactor);
        transform.translate(-cameraPosition);
        states.transform =  transform * originalTransform;

        target.draw(renderer, states);
    }

    for (const auto& actor : world.getActors())
    {
        if (actor->isAlive())
            target.draw(*actor, states);
    }
}

#pragma once

#include <SFML/Graphics.hpp>

class World;
class LevelLayer;

struct TextureAtlas
{
    using Region = sf::FloatRect;

    sf::Texture texture;
    std::vector<Region> regions;

    const Region &getRegion(size_t index) const;

    static TextureAtlas MakeFromRegularGrid(const std::string &textureName, glm::uvec2 cellSize, size_t numOfCells);
};

class LayerRenderer final : public sf::Drawable
{
public:
    void update(const LevelLayer &layer, TextureAtlas& tileAtlas);

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
    sf::Texture *texture = nullptr;
    sf::VertexArray vertexArray{sf::Quads};
};
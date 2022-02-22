#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H
#include <SFML/Graphics.hpp>

namespace sn
{
    class VirtualScreen : public sf::Drawable
    {
    public:
        void create (unsigned int width, unsigned int height, float pixel_size, sf::Color color);
        void setPixel (std::size_t x, std::size_t y, sf::Color color);

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        sf::Vector2u m_screenSize;
        float m_pixelSize; //virtual pixel size in real pixels
        sf::VertexArray m_vertices;
    };
};
#endif // VIRTUALSCREEN_H

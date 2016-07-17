#ifndef VIRTUALSCREEN_H
#define VIRTUALSCREEN_H
#include <SFML/Graphics.hpp>

namespace sn
{
    using uint = unsigned int;

    class VirtualScreen : public sf::Drawable
    {
    public:
        void create (uint width, uint height, float pixel_size, sf::Color default_color);
        void setPixel (std::size_t x, std::size_t y, sf::Color color);

    private:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;

        sf::Vector2<uint> m_screenSize;
        float m_pixelSize; //virtual pixel size in real pixels
        sf::VertexArray m_vertices;
    };
};
#endif // VIRTUALSCREEN_H

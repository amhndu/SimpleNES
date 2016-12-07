#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <SFML/Window.hpp>
#include <cstdint>
#include <vector>

namespace sn
{
    using Byte = std::uint8_t;
    class Controller
    {
    public:
        Controller();
        enum Buttons
        {
            A,
            B,
            Select,
            Start,
            Up,
            Down,
            Left,
            Right,
            TotalButtons,
        };

        void strobe(Byte b);
        Byte read();
        void setKeyBindings(const std::vector<sf::Keyboard::Key>& keys);
    private:
        bool m_strobe;
        unsigned int m_keyStates;

        std::vector<sf::Keyboard::Key> m_keyBindings;
//         sf::Keyboard::Key m_keyBindings[TotalButtons];
    };
}

#endif // CONTROLLER_H

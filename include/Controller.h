#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <SFML/Window.hpp>
#include <list>
#include <map>
#include <cstdint>

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
    private:
        bool m_strobe;
        unsigned int m_keyStates;

        sf::Keyboard::Key m_keyBindings[TotalButtons];
    };
}

#endif // CONTROLLER_H

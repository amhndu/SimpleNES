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
        };

        void strobe(Byte b);
        Byte read();
    private:
        bool m_strobe;
        unsigned int m_keyStates;

        std::map<Buttons, sf::Keyboard::Key> m_keyBindings;
    };
}

#endif // CONTROLLER_H

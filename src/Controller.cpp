#include "Controller.h"

namespace sn
{
    Controller::Controller() :
        m_keyStates(0),
        m_keyBindings({
            {A, sf::Keyboard::Z},
            {B, sf::Keyboard::X},
            {Select, sf::Keyboard::RShift},
            {Start, sf::Keyboard::Return},
            {Up, sf::Keyboard::Up},
            {Down, sf::Keyboard::Down},
            {Left, sf::Keyboard::Left},
            {Right, sf::Keyboard::Right},
        })
    {
        //TODO Make key bindings user-configurable (or load from a config file)
    }

    void Controller::strobe(Byte b)
    {
        m_strobe = (b & 1);
        if (!m_strobe)
        {
            m_keyStates = 0;
            int shift = 0;
            for (auto i : m_keyBindings)
            {
                m_keyStates |= (sf::Keyboard::isKeyPressed(i.second) << shift);
                ++shift;
            }
        }
    }

    Byte Controller::read()
    {
        Byte ret;
        if (m_strobe)
            ret = sf::Keyboard::isKeyPressed(m_keyBindings[A]);
        else
        {
            ret = m_keyStates & 1;
            m_keyStates >>= 1;
        }
        return ret;
    }

}
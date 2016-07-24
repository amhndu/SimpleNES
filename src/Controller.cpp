#include "Controller.h"

namespace sn
{
    Controller::Controller() :
        m_keyStates(0)
    {
        //TODO Make key bindings user-configurable (or load from a config file)
        m_keyBindings[A] = sf::Keyboard::Z;
        m_keyBindings[B] = sf::Keyboard::X;
        m_keyBindings[Select] = sf::Keyboard::RShift;
        m_keyBindings[Start] = sf::Keyboard::Return;
        m_keyBindings[Up] = sf::Keyboard::Up;
        m_keyBindings[Down] = sf::Keyboard::Down;
        m_keyBindings[Left] = sf::Keyboard::Left;
        m_keyBindings[Right] = sf::Keyboard::Right;
    }

    void Controller::strobe(Byte b)
    {
        m_strobe = (b & 1);
        if (!m_strobe)
        {
            m_keyStates = 0;
            int shift = 0;
            for (int button = A; button < TotalButtons; ++button)
            {
                m_keyStates |= (sf::Keyboard::isKeyPressed(m_keyBindings[static_cast<Buttons>(button)]) << shift);
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
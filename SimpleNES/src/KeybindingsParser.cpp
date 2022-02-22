#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>

#include "Controller.h"
#include "Log.h"

namespace sn
{
    // trim from start (construct new string)
    inline std::string ltrim(const std::string &str)
    {
        std::string s(str);
        s.erase(s.begin(), std::find_if_not<decltype(s.begin()), int(int)>(s.begin(), s.end(),
                std::isspace));
        return s;
    }

    // trim from end (construct new string)
    inline std::string rtrim(const std::string &str)
    {
        std::string s(str);
        s.erase(std::find_if_not<decltype(s.rbegin()), int(int)>(s.rbegin(), s.rend(),
                std::isspace).base(), s.end());
        return s;
    }

    void parseControllerConf(std::string filepath,
                            std::vector<sf::Keyboard::Key>& p1,
                            std::vector<sf::Keyboard::Key>& p2)
    {
        const std::string buttonStrings[] = { "A",
                                            "B",
                                            "Select",
                                            "Start",
                                            "Up",
                                            "Down",
                                            "Left",
                                            "Right" };
        const std::string keys[] = {"A",
                                    "B",
                                    "C",
                                    "D",
                                    "E",
                                    "F",
                                    "G",
                                    "H",
                                    "I",
                                    "J",
                                    "K",
                                    "L",
                                    "M",
                                    "N",
                                    "O",
                                    "P",
                                    "Q",
                                    "R",
                                    "S",
                                    "T",
                                    "U",
                                    "V",
                                    "W",
                                    "X",
                                    "Y",
                                    "Z",
                                    "Num0",
                                    "Num1",
                                    "Num2",
                                    "Num3",
                                    "Num4",
                                    "Num5",
                                    "Num6",
                                    "Num7",
                                    "Num8",
                                    "Num9",
                                    "Escape",
                                    "LControl",
                                    "LShift",
                                    "LAlt",
                                    "LSystem",
                                    "RControl",
                                    "RShift",
                                    "RAlt",
                                    "RSystem",
                                    "Menu",
                                    "LBracket",
                                    "RBracket",
                                    "SemiColon",
                                    "Comma",
                                    "Period",
                                    "Quote",
                                    "Slash",
                                    "BackSlash",
                                    "Tilde",
                                    "Equal",
                                    "Dash",
                                    "Space",
                                    "Return",
                                    "BackSpace",
                                    "Tab",
                                    "PageUp",
                                    "PageDown",
                                    "End",
                                    "Home",
                                    "Insert",
                                    "Delete",
                                    "Add",
                                    "Subtract",
                                    "Multiply",
                                    "Divide",
                                    "Left",
                                    "Right",
                                    "Up",
                                    "Down",
                                    "Numpad0",
                                    "Numpad1",
                                    "Numpad2",
                                    "Numpad3",
                                    "Numpad4",
                                    "Numpad5",
                                    "Numpad6",
                                    "Numpad7",
                                    "Numpad8",
                                    "Numpad9",
                                    "F1",
                                    "F2",
                                    "F3",
                                    "F4",
                                    "F5",
                                    "F6",
                                    "F7",
                                    "F8",
                                    "F9",
                                    "F10",
                                    "F11",
                                    "F12",
                                    "F13",
                                    "F14",
                                    "F15",
                                    "Pause"
                                };

        std::ifstream file(filepath);
        std::string line;
        enum { Player1, Player2, None } state;
        unsigned int line_no = 0;
        while (std::getline(file, line))
        {
            line = rtrim(ltrim(line));
            if (line[0] == '#' || line.empty())
                continue;
            else if (line == "[Player1]")
            {
                state = Player1;
            }
            else if (line == "[Player2]")
            {
                state = Player2;
            }
            else if (state == Player1 || state == Player2)
            {
                auto divider = line.find("=");
                auto it  = std::find(std::begin(buttonStrings), std::end(buttonStrings), ltrim(rtrim(line.substr(0, divider)))),
                    it2 = std::find(std::begin(keys), std::end(keys), ltrim(rtrim(line.substr(divider + 1))));
                if (it == std::end(buttonStrings) || it2 == std::end(keys))
                {
                    LOG(Error) << "Invalid key in configuration file at Line " << line_no << std::endl;
                    continue;
                }
                auto i = std::distance(std::begin(buttonStrings), it);
                auto key = std::distance(std::begin(keys), it2);
                (state == Player1 ? p1 : p2)[i] = static_cast<sf::Keyboard::Key>(key);
            }
            else
                LOG(Error) << "Invalid line in key configuration at Line " << line_no << std::endl;

            ++line_no;
        }

    }
}


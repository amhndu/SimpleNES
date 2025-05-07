#ifndef EMULATOR_H
#define EMULATOR_H
#include <SFML/Graphics.hpp>
#include <chrono>

#include "APU/APU.h"
#include "AudioPlayer.h"
#include "CPU.h"
#include "Controller.h"
#include "MainBus.h"
#include "PPU.h"
#include "PictureBus.h"

namespace sn
{
using TimePoint          = std::chrono::high_resolution_clock::time_point;

const int NESVideoWidth  = ScanlineVisibleDots;
const int NESVideoHeight = VisibleScanlines;

class Emulator
{
public:
    Emulator();
    void run(std::string rom_path);
    void setVideoWidth(int width);
    void setVideoHeight(int height);
    void setVideoScale(float scale);
    void setKeys(std::vector<sf::Keyboard::Key>& p1, std::vector<sf::Keyboard::Key>& p2);

private:
    void                                         DMA(Byte page);

    CPU                                          m_cpu;

    AudioPlayer                                  m_audioPlayer;

    PictureBus                                   m_pictureBus;
    PPU                                          m_ppu;
    APU                                          m_apu;
    Cartridge                                    m_cartridge;
    std::unique_ptr<Mapper>                      m_mapper;

    Controller                                   m_controller1, m_controller2;

    MainBus                                      m_bus;

    sf::RenderWindow                             m_window;
    VirtualScreen                                m_emulatorScreen;
    float                                        m_screenScale;

    TimePoint                                    m_lastWakeup;

    std::chrono::high_resolution_clock::duration m_elapsedTime;
};
}
#endif // EMULATOR_H

#include "Emulator.h"
#include "CPUOpcodes.h"
#include "Log.h"
#include "APU/Constants.h"

#include <chrono>

namespace sn
{
    using std::chrono::high_resolution_clock;

    Emulator::Emulator() :
        m_cpu(m_bus),
        m_ppu(m_pictureBus, m_emulatorScreen),
        m_apu(m_audioPlayer, [&](){ m_cpu.nmiInterrupt(InterruptType::IRQ); }),
        m_bus(m_ppu, m_apu, m_controller1, m_controller2, [&](Byte b){ DMA(b); }),
        m_screenScale(3.f),
        m_lastWakeup()
    {
        m_ppu.setInterruptCallback([&](){ m_cpu.nmiInterrupt(); });
    }

    void Emulator::run(std::string rom_path)
    {
        if (!m_cartridge.loadFromFile(rom_path))
            return;

        m_mapper = Mapper::createMapper(static_cast<Mapper::Type>(m_cartridge.getMapper()),
                                        m_cartridge,
                                        [&](){ m_cpu.nmiInterrupt(InterruptType::IRQ); },
                                        [&](){ m_pictureBus.updateMirroring(); });
        if (!m_mapper)
        {
            LOG(Error) << "Creating Mapper failed. Probably unsupported." << std::endl;
            return;
        }

        if (!m_bus.setMapper(m_mapper.get()) ||
            !m_pictureBus.setMapper(m_mapper.get()))
        {
            return;
        }

        m_cpu.reset();
        m_ppu.reset();

        m_window.create(sf::VideoMode(NESVideoWidth * m_screenScale, NESVideoHeight * m_screenScale),
                        "SimpleNES", sf::Style::Titlebar | sf::Style::Close);
        m_window.setVerticalSyncEnabled(true);
        m_emulatorScreen.create(NESVideoWidth, NESVideoHeight, m_screenScale, sf::Color::White);

        m_lastWakeup = high_resolution_clock::now();
        m_elapsedTime = m_lastWakeup - m_lastWakeup;

        m_audioPlayer.start();
        // TEST CODE
        m_apu.pulse1.set_frequency(82.41);
        m_apu.pulse1.volume.constantVolume = false;
        m_apu.pulse1.volume.isLooping = true;
        m_apu.pulse1.volume.shouldStart = true;
        m_apu.pulse1.volume.fixedVolumeOrPeriod = 1;
        // TEST CODE END

        sf::Event event;
        bool focus = true, pause = false;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                {
                    m_window.close();
                    return;
                }
                else if (event.type == sf::Event::GainedFocus)
                {
                    focus = true;
                    m_lastWakeup = high_resolution_clock::now();
                }
                else if (event.type == sf::Event::LostFocus)
                    focus = false;
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2)
                {
                    pause = !pause;
                    if (!pause)
                    {
                        m_lastWakeup = high_resolution_clock::now();
                        LOG(Info) << "Unpaused." << std::endl;
                    }
                    else
                    {
                        LOG(Info) << "Paused." << std::endl;
                    }
                }
                else if (pause && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F3)
                {
                    for (int i = 0; i < 29781; ++i) //Around one frame
                    {
                        //PPU
                        m_ppu.step();
                        m_ppu.step();
                        m_ppu.step();
                        //CPU
                        m_cpu.step();
                        // APU
                        m_apu.step();
                    }
                }
                else if (focus && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F4)
                {
                    Log::get().setLevel(Info);
                }
                else if (focus && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::F5)
                {
                    Log::get().setLevel(InfoVerbose);
                }
            }

            if (focus && !pause)
            {
                m_elapsedTime += high_resolution_clock::now() - m_lastWakeup;
                m_lastWakeup = high_resolution_clock::now();

                while (m_elapsedTime > cpu_clock_period_ns)
                {
                    // PPU
                    m_ppu.step();
                    m_ppu.step();
                    m_ppu.step();
                    // CPU
                    m_cpu.step();
                    // APU
                    m_apu.step();

                    m_elapsedTime -= cpu_clock_period_ns;
                }

                m_window.draw(m_emulatorScreen);
                m_window.display();
            }
            else
            {
                sf::sleep(sf::milliseconds(1000/60));
            }
        }
    }

    void Emulator::DMA(Byte page)
    {
        m_cpu.skipDMACycles();
        auto page_ptr = m_bus.getPagePtr(page);
        if (page_ptr != nullptr)
        {
            m_ppu.doDMA(page_ptr);
        }
        else
        {
            LOG(Error) << "Can't get pageptr for DMA" << std::endl;
        }
    }

    void Emulator::setVideoHeight(int height)
    {
        m_screenScale = height / float(NESVideoHeight);
        LOG(Info) << "Scale: " << m_screenScale << " set. Screen: "
                  << int(NESVideoWidth * m_screenScale) << "x" << int(NESVideoHeight * m_screenScale) << std::endl;
    }

    void Emulator::setVideoWidth(int width)
    {
        m_screenScale = width / float(NESVideoWidth);
        LOG(Info) << "Scale: " << m_screenScale << " set. Screen: "
                  << int(NESVideoWidth * m_screenScale) << "x" << int(NESVideoHeight * m_screenScale) << std::endl;

    }
    void Emulator::setVideoScale(float scale)
    {
        m_screenScale = scale;
        LOG(Info) << "Scale: " << m_screenScale << " set. Screen: "
                  << int(NESVideoWidth * m_screenScale) << "x" << int(NESVideoHeight * m_screenScale) << std::endl;
    }

    void Emulator::setKeys(std::vector<sf::Keyboard::Key>& p1, std::vector<sf::Keyboard::Key>& p2)
    {
        m_controller1.setKeyBindings(p1);
        m_controller2.setKeyBindings(p2);
    }

}

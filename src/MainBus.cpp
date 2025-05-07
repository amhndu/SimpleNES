#include "MainBus.h"
#include "Cartridge.h"
#include "Log.h"
#include <functional>

namespace sn
{
MainBus::MainBus(PPU& ppu, APU& apu, Controller& ctrl1, Controller& ctrl2, std::function<void(Byte)> dma)
  : m_RAM(0x800, 0)
  , m_dmaCallback(dma)
  , m_mapper(nullptr)
  , m_ppu(ppu)
  , m_apu(apu)
  , m_controller1(ctrl1)
  , m_controller2(ctrl2)
{
}

Address normalize_mirror(Address addr)
{
    if (addr >= MainBus::PPU_CTRL && addr < MainBus::APU_REGISTER_START)
    {
        // 0x2008 - 0x3fff are mirrors of 0x2000 - 0x2007
        return addr & 0x2007;
    }

    // no mirroring
    return addr;
}

Byte MainBus::read(Address addr)
{
    if (addr < 0x2000)
    {
        return m_RAM[addr & 0x7ff];
    }
    else if (addr < 0x4020) // memory-mapped registers
    {
        addr = normalize_mirror(addr);
        switch (addr)
        {
        case PPU_STATUS:
            return m_ppu.getStatus();
            break;
        case PPU_DATA:
            return m_ppu.getData();
            break;
        case JOY1:
            return m_controller1.read();
            break;
        case JOY2_AND_FRAME_CONTROL:
            return m_controller2.read();
            break;
        case OAM_DATA:
            return m_ppu.getOAMData();
            break;
        case APU_CONTROL_AND_STATUS:
            return m_apu.readStatus();
            break;
        default:
            LOG(InfoVerbose) << "Read access attempt at: " << std::hex << +addr << std::endl;
            return 0;
            break;
        }
    }
    else if (addr < 0x6000)
    {
        LOG(InfoVerbose) << "Expansion ROM read attempted. This is currently unsupported" << std::endl;
        return 0;
    }
    else if (addr < 0x8000)
    {
        if (m_mapper->hasExtendedRAM())
        {
            return m_extRAM[addr - 0x6000];
        }

        return 0;
    }
    else
    {
        return m_mapper->readPRG(addr);
    }
}

void MainBus::write(Address addr, Byte value)
{
    if (addr < 0x2000)
    {
        m_RAM[addr & 0x7ff] = value;
    }
    else if (addr < 0x4020) // memory-mapped registers
    {
        addr = normalize_mirror(addr);
        switch (addr)
        {
        case PPU_CTRL:
            m_ppu.control(value);
            break;
        case PPU_MASK:
            m_ppu.setMask(value);
            break;
        case OAM_ADDR:
            m_ppu.setOAMAddress(value);
            break;
        case OAM_DATA:
            m_ppu.setOAMData(value);
            break;
        case PPU_ADDR:
            m_ppu.setDataAddress(value);
            break;
        case PPU_SCROL:
            m_ppu.setScroll(value);
            break;
        case PPU_DATA:
            m_ppu.setData(value);
            break;
        case OAM_DMA:
            m_dmaCallback(value);
            break;
        case JOY1:
            m_controller1.strobe(value);
            m_controller2.strobe(value);
            break;

        case APU_CONTROL_AND_STATUS:
        case JOY2_AND_FRAME_CONTROL:
            m_apu.writeRegister(addr, value);
            break;

        default:
            if (addr >= APU_REGISTER_START && addr <= APU_REGISTER_END)
            {
                m_apu.writeRegister(addr, value);
            }
            else
            {
                LOG(InfoVerbose) << "Write access attempt at: " << std::hex << +addr << std::endl;
            }
            break;
        }
    }
    else if (addr < 0x6000)
    {
        LOG(InfoVerbose) << "Expansion ROM access attempted. This is currently unsupported" << std::endl;
    }
    else if (addr < 0x8000)
    {
        if (m_mapper->hasExtendedRAM())
        {
            m_extRAM[addr - 0x6000] = value;
        }
    }
    else
    {
        m_mapper->writePRG(addr, value);
    }
}

const Byte* MainBus::getPagePtr(Byte page)
{
    Address addr = page << 8;
    if (addr < 0x2000)
    {
        return &m_RAM[addr & 0x7ff];
    }
    else if (addr < 0x4020)
    {
        LOG(Error) << "Register address memory pointer access attempt" << std::endl;
    }
    else if (addr < 0x6000)
    {
        LOG(Error) << "Expansion ROM access attempted, which is unsupported" << std::endl;
    }
    else if (addr < 0x8000)
    {
        if (m_mapper->hasExtendedRAM())
        {
            return &m_extRAM[addr - 0x6000];
        }
    }
    else
    {
        LOG(Error) << "Unexpected DMA request: " << std::hex << "0x" << +addr << " (" << +page << ")" << std::dec
                   << std::endl;
    }
    return nullptr;
}

bool MainBus::setMapper(Mapper* mapper)
{
    m_mapper = mapper;

    if (!mapper)
    {
        LOG(Error) << "Mapper pointer is nullptr" << std::endl;
        return false;
    }

    if (mapper->hasExtendedRAM())
        m_extRAM.resize(0x2000);

    return true;
}
};

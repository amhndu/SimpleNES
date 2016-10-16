#include "PictureBus.h"
#include "Log.h"

namespace sn
{

    PictureBus::PictureBus() :
        m_RAM(0x800),
        m_palette(0x20),
        m_mapper(nullptr)
    {}

    Byte PictureBus::read(Address addr)
    {
        if (addr < 0x2000)
        {
            return m_mapper->readCHR(addr);
        }
        else if (addr < 0x3eff) //Name tables upto 0x3000, then mirrored upto 3eff
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400)      //NT0
                return m_RAM[NameTable0 + index];
            else if (addr < 0x2800) //NT1
                return m_RAM[NameTable1 + index];
            else if (addr < 0x2c00) //NT2
                return m_RAM[NameTable2 + index];
            else                    //NT3
                return m_RAM[NameTable3 + index];
        }
        else if (addr < 0x3fff)
        {
            return m_palette[addr & 0x1f];
        }
        return 0;
    }

    Byte PictureBus::readPalette(Byte paletteAddr)
    {
        return m_palette[paletteAddr];
    }

    void PictureBus::write(Address addr, Byte value)
    {
        if (addr < 0x2000)
        {
            m_mapper->writeCHR(addr, value);
        }
        else if (addr < 0x3eff) //Name tables upto 0x3000, then mirrored upto 3eff
        {
            auto index = addr & 0x3ff;
            if (addr < 0x2400)      //NT0
                m_RAM[NameTable0 + index] = value;
            else if (addr < 0x2800) //NT1
                m_RAM[NameTable1 + index] = value;
            else if (addr < 0x2c00) //NT2
                m_RAM[NameTable2 + index] = value;
            else                    //NT3
                m_RAM[NameTable3 + index] = value;
        }
        else if (addr < 0x3fff)
        {
            if (addr == 0x3f10)
                m_palette[0] = value;
            else
                m_palette[addr & 0x1f] = value;
       }
    }

    void PictureBus::updateMirroring()
    {
        switch (m_mapper->getNameTableMirroring())
        {
            case Horizontal:
                NameTable0 = NameTable1 = 0;
                NameTable2 = NameTable3 = 0x400;
                LOG(InfoVerbose) << "Horizontal Name Table mirroring set. (Vertical Scrolling)" << std::endl;
                break;
            case Vertical:
                NameTable0 = NameTable2 = 0;
                NameTable1 = NameTable3 = 0x400;
                LOG(InfoVerbose) << "Vertical Name Table mirroring set. (Horizontal Scrolling)" << std::endl;
                break;
            case OneScreenLower:
                NameTable0 = NameTable1 = NameTable2 = NameTable3 = 0;
                LOG(InfoVerbose) << "Single Screen mirroring set with lower bank." << std::endl;
                break;
            case OneScreenHigher:
                NameTable0 = NameTable1 = NameTable2 = NameTable3 = 0x400;
                LOG(InfoVerbose) << "Single Screen mirroring set with higher bank." << std::endl;
                break;
            default:
                NameTable0 = NameTable1 = NameTable2 = NameTable3 = 0;
                LOG(Error) << "Unsupported Name Table mirroring : " << m_mapper->getNameTableMirroring() << std::endl;
        }
    }

    bool PictureBus::setMapper(Mapper *mapper)
    {
        if (!mapper)
        {
            LOG(Error) << "Mapper argument is nullptr" << std::endl;
            return false;
        }

        m_mapper = mapper;
        updateMirroring();
        return true;
    }

}

#include "MapperSxROM.h"
#include "Log.h"

namespace sn
{
    MapperSxROM::MapperSxROM(Cartridge &cart) :
        Mapper(cart, Mapper::SxROM),
        m_modeCHR(0),
        m_modePRG(3),
        m_tempRegister(0),
        m_writeCounter(0),
        m_firstBankPRG(nullptr),
        m_secondBankPRG(nullptr),
        m_firstBankCHR(nullptr),
        m_secondBankCHR(nullptr)
    {
        if (cart.getVROM().size() == 0)
        {
            m_usesCharacterRAM = true;
            m_characterRAM.resize(0x2000);
            LOG(Info) << "Uses character RAM" << std::endl;
        }
        else
        {
            LOG(Info) << "Using CHR-ROM" << std::endl;
            m_usesCharacterRAM = false;
            m_firstBankCHR = &cart.getVROM()[0];
            m_secondBankCHR = &cart.getVROM()[0x1000];
        }

        m_firstBankPRG = &cart.getROM()[0]; //first bank
        m_secondBankPRG = &cart.getROM()[/*cart.getROM().size() - 0x4000*/0x2000 * 0x0e]; //last bank
    }

    Byte MapperSxROM::readPRG(Address addr)
    {
        if (addr < 0xc000)
            return *(m_firstBankPRG + (addr & 0x3fff));
        else
            return *(m_secondBankPRG + (addr & 0x3fff));
    }

    void MapperSxROM::writePRG(Address addr, Byte value)
    {
        if (!(value & 0x80)) //if reset bit is NOT set
        {
            m_tempRegister = (m_tempRegister >> 1) | ((value & 1) << 4);
            ++m_writeCounter;

            if (m_writeCounter == 5)
            {
                if (addr < 0xa000)
                {
                    m_modeCHR = (m_tempRegister & 0x10) >> 4;
                    m_modePRG = (m_tempRegister & 0xc) >> 2;
                    LOG(Info) << "CHR mode: " << m_modeCHR << std::endl;
                    LOG(Info) << "PRG mode: " << m_modePRG << std::endl;
                    //TODO Mirroring?
                }
                else if (addr < 0xc000)
                {
                    if (m_modeCHR == 0) //if 8KB switchable
                        m_tempRegister |= 1;

                    m_firstBankCHR = &m_cartridge.getVROM()[0x1000 * m_tempRegister];
                    if (m_modeCHR == 0)
                        m_secondBankCHR = m_firstBankCHR + 0x1000;
                }
                else if (addr < 0xe000)
                {
//                     if(m_modeCHR == 1)
                    m_secondBankCHR = &m_cartridge.getVROM()[0x1000 * m_tempRegister];

//                     else
//                         LOG(Info) << "This doesn't make sense now" << std::endl;
                }
                else
                {
                    //TODO PRG-RAM
                    if ((m_tempRegister & 0x10) == 0x10)
                    {
                        LOG(Info) << "PRG-RAM activated" << std::endl;
                    }

                    m_tempRegister &= 0xf;

                    if (m_modePRG <= 1) //32KB changeable
                    {
                        m_firstBankPRG = &m_cartridge.getROM()[0x4000 * (m_tempRegister | 1)];
                        m_secondBankPRG = m_firstBankPRG + 0x4000;   //add 16KB
                    }
                    else if (m_modePRG == 2) //fix first switch second
                    {
                        m_firstBankPRG = &m_cartridge.getROM()[0];
                        m_secondBankPRG = m_firstBankPRG + 0x4000 * m_tempRegister;
                    }
                    else //switch first fix second
                    {
                        m_firstBankPRG = &m_cartridge.getROM()[0x4000 * m_tempRegister];
                        m_secondBankPRG = &m_cartridge.getROM()[/*m_cartridge.getROM().size() - 0x4000*/0x2000 * 0x0e];
                    }
                }

                m_tempRegister = 0;
                m_writeCounter = 0;
            }
        }
        else //reset
        {
            m_tempRegister = 0;
            m_writeCounter = 0;
            m_modePRG = 3;
        }
    }

    const Byte* MapperSxROM::getPagePtr(Address addr)
    {
        if (addr < 0xc000)
            return (m_firstBankPRG + (addr & 0x3fff));
        else
            return (m_secondBankPRG + (addr & 0x3fff));
    }

    Byte MapperSxROM::readCHR(Address addr)
    {
        if (m_usesCharacterRAM)
            return m_characterRAM[addr];
        else if (addr < 0x1000)
            return *(m_firstBankCHR + addr);
        else
            return *(m_secondBankCHR + (addr & 0xfff));
    }

    void MapperSxROM::writeCHR(Address addr, Byte value)
    {
        if (m_usesCharacterRAM)
            m_characterRAM[addr] = value;
        else
            LOG(Info) << "Read-only CHR memory write attempt at " << std::hex << addr << std::endl;
    }
}
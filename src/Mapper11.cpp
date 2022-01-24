#include "Mapper11.h"
#include "Log.h"
namespace sn
{

    Mapper11::Mapper11(Cartridge &cart,std::function<void(void)> mirroring_cb) :
    Mapper(cart, Mapper::ColorDreams),
    m_mirroringCallback(mirroring_cb),
    m_mirroring(Vertical)    
    {}

    Byte Mapper11::readPRG(Address address)
    {                 
          
             if (address >= 0x8000 )
             {
              return  m_cartridge.getROM()[(prgbank * 0x8000) + address & 0x7fff];

             }
    }
       

      
    void Mapper11::writePRG(Address address, Byte value)
    {
      if (address >= 0x8000)
      {
        prgbank = ((value >> 0) & 0x3);
        chrbank = ((value  >> 4) & 0xF);

      }

    }
  

    Byte Mapper11::readCHR(Address address)
    {        
      if (address <= 0x1FFF)
      {
          return   m_cartridge.getVROM()[(chrbank * 0x2000) + address];     

      }
      
    }

  

    NameTableMirroring Mapper11::getNameTableMirroring()
    {  
        return m_mirroring;
    }

    void Mapper11::writeCHR(Address address, Byte value)
    {}

    
    const Byte *Mapper11::getPagePtr(Address address)
    {}

}

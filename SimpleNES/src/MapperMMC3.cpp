#include "MapperMMC3.h"

namespace sn
{

  MapperMMC3::MapperMMC3(Cartridge &cart, std::function<void(void)> mirroring_cb) : 
  Mapper(cart, Mapper::MMC3),
  m_mirroringCallback(mirroring_cb),
  mirrormode(Horizontal),
  nTargetRegister(0),
  bPRGBankMode(false),
  bCHRInversion(false),
  bIRQActive(false),
  bIRQEnable(false),
  nIRQCounter(0),
  nIRQReload(0),
  lastread(0),
  banks(0)
{

ramstatic.resize(32 * 1024);
	
prgbank0 = &cart.getROM()[cart.getROM().size() - 0x4000];
prgbank1 = &cart.getROM()[cart.getROM().size() - 0x2000];
prgbank2 = &cart.getROM()[cart.getROM().size() - 0x4000];
prgbank3 = &cart.getROM()[cart.getROM().size() - 0x2000];
 
 

chrbank0 = &cart.getVROM()[cart.getVROM().size() * 0x800];
chrbank1 = &cart.getVROM()[cart.getVROM().size() * 0x800];
chrbank2 = &cart.getVROM()[cart.getVROM().size() * 0x400];
chrbank3 = &cart.getVROM()[cart.getVROM().size() * 0x400];
chrbank4 = &cart.getVROM()[cart.getVROM().size() * 0x400];
chrbank5 = &cart.getVROM()[cart.getVROM().size() * 0x400];
chrbank6 = &cart.getVROM()[cart.getVROM().size() * 0x400];
chrbank7 = &cart.getVROM()[cart.getVROM().size() * 0x400];


}



 Byte MapperMMC3::readPRG(Address addr)
{
    if (addr >= 0x6000 && addr <= 0x7FFF)
	 {
	  return  ramstatic[addr & 0x1fff];

	 Byte  &value = ramstatic[addr & 0x1FFF];

		
	}
	
  
   if (addr >= 0x8000 && addr <= 0x9FFF)
	{
	 return *(prgbank0 + (addr & 0x1fff));
	}

	if (addr >= 0xA000 && addr <= 0xBFFF)
	{	
	return   *(prgbank1 + (addr & 0x1fff));
	}

	if (addr >= 0xC000 && addr <= 0xDFFF)
	{
      return *(prgbank2 + (addr & 0x1fff));
	}

	if (addr >= 0xE000 && addr <= 0xFFFF)
    {   
     return *(prgbank3  +  (addr & 0x1fff));
	}
}


Byte MapperMMC3::readCHR(Address addr)
{   

	if (addr >= 0x0000 && addr <= 0x03FF)
	{
	return *(chrbank0 + (addr & 0x03ff));
	}

	if (addr >= 0x0400 && addr <= 0x07FF)
	{
	return *(chrbank1 + (addr & 0x03ff));
	}

	if (addr >= 0x0800 && addr <= 0x0BFF)
	{
	return *(chrbank2 + (addr & 0x03ff));
	}

	if (addr >= 0x0C00 && addr <= 0x0FFF)
	{

	return *(chrbank3 + (addr & 0x03ff));
	}

	if (addr >= 0x1000 && addr <= 0x13FF)
	{
    return *(chrbank4 + (addr & 0x03ff));
	}

	if (addr >= 0x1400 && addr <= 0x17FF)
	{
	return *(chrbank5 + (addr & 0x03ff));
	}

	if (addr >= 0x1800 && addr <= 0x1BFF)
	{
	return *(chrbank6 + (addr & 0x03ff));
	}

	if (addr >= 0x1C00 && addr <= 0x1FFF)
	{
	return *(chrbank7 + (addr & 0x03ff));
	}  
}




void MapperMMC3::writePRG(Address addr, Byte value)
{

if (addr >= 0x6000 && addr <= 0x7FFF)
	{
    	ramstatic[addr & 0x1fff];

		ramstatic[addr & 0x1FFF] = value;
         return;
	}

	if (addr >= 0x8000 && addr <= 0x9FFF)
	{
		// Bank Select
		if (!(addr & 0x01))
		{
			
			nTargetRegister = value  & 0x7;
			bPRGBankMode    = (value & 0x40);
			bCHRInversion   = (value & 0x80);         
		}
		else
		{
			pRegister[nTargetRegister] = value;

			if (bCHRInversion == 0)
			{ 

                chrbank0 = &m_cartridge.getVROM()[(pRegister[0] & 0xFE) * 0x0400];
				chrbank1 = &m_cartridge.getVROM()[pRegister[0] * 0x0400 + 0x0400];
				chrbank2 = &m_cartridge.getVROM()[(pRegister[1] & 0xFE) * 0x0400];
				chrbank3 = &m_cartridge.getVROM()[pRegister[1] * 0x0400 + 0x0400];
				chrbank4 = &m_cartridge.getVROM()[pRegister[2] * 0x0400];
				chrbank5 = &m_cartridge.getVROM()[pRegister[3] * 0x0400];
				chrbank6 = &m_cartridge.getVROM()[pRegister[4] * 0x0400];
				chrbank7 = &m_cartridge.getVROM()[pRegister[5] * 0x0400];

		    
			}
			else if (bCHRInversion == 1)
			{
		        chrbank0 = &m_cartridge.getVROM()[pRegister[2] * 0x0400];
				chrbank1 = &m_cartridge.getVROM()[pRegister[3] * 0x0400];
				chrbank2 = &m_cartridge.getVROM()[pRegister[4] * 0x0400];
				chrbank3 = &m_cartridge.getVROM()[pRegister[5] * 0x0400];
				chrbank4 = &m_cartridge.getVROM()[(pRegister[0] & 0xFE) * 0x0400];
				chrbank5 = &m_cartridge.getVROM()[pRegister[0] * 0x0800 + 0x0400];
				chrbank6 = &m_cartridge.getVROM()[(pRegister[1] & 0xFE) * 0x0400];
				chrbank7 = &m_cartridge.getVROM()[pRegister[1] * 0x0400 + 0x0400];
			
			}
     
		 if (bPRGBankMode == 0)
			{
           
			  prgbank0 = &m_cartridge.getROM()[(pRegister[6] & 0x3F) * 0x2000];
			
			} else if(bPRGBankMode == 1){ 
			
		      prgbank2 = &m_cartridge.getROM()[(pRegister[6] & 0x3F) * 0x2000];
			}
			 prgbank1 = &m_cartridge.getROM()[(pRegister[7] & 0x3F) * 0x2000];
	
 

	
	}

}
	
	if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		if (!(addr & 0x01))
		{
			// Mirroring
			if (value & 0x01)
			{
				mirrormode =  NameTableMirroring::Horizontal;
			}
			else
			{
			    mirrormode = NameTableMirroring::Vertical;
			}
		m_mirroringCallback();
		}
		else
		{
			// PRG Ram Protect
		
		}
	}

	if (addr >= 0xC000 && addr <= 0xDFFF)
	{
		if (!(addr & 0x01))
		{
			nIRQReload = value;
		}
		else
		{
			nIRQCounter = 0;
		}
	}

	if (addr >= 0xE000 && addr <= 0xFFFF)
	{
		if (!(addr & 0x01))
		{
			bIRQEnable = false;
			bIRQActive = false;
		}
		else
		{
			bIRQEnable = true;
		}
	}
}


void MapperMMC3::writeCHR(Address addr, Byte value)
{}


bool MapperMMC3::irqState() 
{ 
	return bIRQActive;
}

void MapperMMC3::irqClear() 
{ 
 bIRQActive = false;
}


void MapperMMC3::scanline()
 {
   	if (nIRQCounter == 0)
	{		
		nIRQCounter = nIRQReload;
	}
	else
		nIRQCounter--;

	if (nIRQCounter == 0 && bIRQEnable)
	{
		bIRQActive = true;
	}	
}

NameTableMirroring MapperMMC3::getNameTableMirroring()
{
  return mirrormode;
}

const Byte *MapperMMC3::getPagePtr(Address addr)
{}

} // namespace sn

#include "Cartridge.h"

using namespace std;

Cartridge::Cartridge(const string& sFileName)
{
  //iNes format Header 
  struct sHeader{
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
  } header;

  bImageValid=false;

  ifstream ifs;
  ifs.open(sFileName,ifstream::binary);
  if(ifs.is_open())
  {
    ///reading file headers
    ifs.read((char*)&header ,sizeof(sHeader));

    if(header.mapper1 & 0x04)
      ifs.seekg(512,ios_base::cur);
    
    //Determine mapper id 
    nMapperID=( (header.mapper2>>4)<<4)|(header.mapper1>>4);
    mirror=(header.mapper1 & 0x01)? VERTICAL:HORIZONTAL;

    // file format 
    uint8_t nFileType=1;

    if(nFileType==0)
    {

    }

    if(nFileType==1)
    {
      nPRGBanks = header.prg_rom_chunks;
			vPRGMemory.resize(nPRGBanks * 16384);
			ifs.read((char*)vPRGMemory.data(), vPRGMemory.size());

			nCHRBanks = header.chr_rom_chunks;
			if (nCHRBanks == 0)
			{
				// Create CHR RAM
				vCHRMemory.resize(8192);
			}
			else
			{
				// Allocate for ROM
				vCHRMemory.resize(nCHRBanks * 8192);
			}
			ifs.read((char*)vCHRMemory.data(), vCHRMemory.size());
    }
    if(nFileType==2)
    {
      
    }

    //loading mapper
    if(pMapper==0)
    {
      std::make_shared<Mapper_000>(nPRGBanks,nCHRBanks);
    }

    bImageValid=true;
    ifs.close();

    
  }
}

Cartridge::~Cartridge()
{

}

bool Cartridge::ImageValid()
{
  return bImageValid;
}

bool Cartridge::cpuRead(uint16_t addr,uint8_t& data)
{
    uint32_t mapped_addr=0;
    if(pMapper->cpuMapRead(addr,mapped_addr))
    {
      data=vPRGMemory[mapped_addr];
      return true;
    }
    return false;
}

bool Cartridge::cpuWrite(uint16_t addr,uint8_t data)
{
    uint32_t mapped_addr=0;
    if(pMapper->cpuMapWrite(addr,mapped_addr))
    {
      vPRGMemory[mapped_addr]=data;
      return true;
    }
    return false;
}

bool Cartridge::ppuRead(uint16_t addr,uint8_t& data)
{
  uint32_t mapped_addr=0;
  if(pMapper->ppuMapRead(addr,mapped_addr))
  {
    data=vCHRMemory[mapped_addr];
    return true;
  }
  return false;
}

bool Cartridge::ppuWrite(uint16_t addr,uint8_t data)
{
  uint32_t mapped_addr=0;
  if(pMapper->ppuMapWrite(addr,mapped_addr))
  {
    vCHRMemory[mapped_addr]=data;
    return true;
  }
  return false;
}

void Cartridge::reset()
{
	if (pMapper != nullptr)
		pMapper->reset();
}
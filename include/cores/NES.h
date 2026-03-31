#pragma once
#if !defined(OSCR_CORE_NES_H_)
# define OSCR_CORE_NES_H_

# include "config.h"

# if HAS_NES
#   include "syslibinc.h"
#   include "common/Types.h"

#   define nesCRDB            ((OSCR::Databases::NES *)cartCRDB)

#   define NES_MAPPER         romDetail->mapper
#   define NES_SUBMAPPER      romDetail->submapper
#   define NES_PRGSIZE        romDetail->prgsize
#   define NES_PRG            romDetail->prg
#   define NES_CHRSIZE        romDetail->chrsize
#   define NES_CHR            romDetail->chr
#   define NES_RAMSIZE        romDetail->ramsize
#   define NES_RAM            romDetail->ram
#   define NES_INES           romDetail->iNES
#   define NES_NAME           romDetail->name
#   define NES_NAME_SIZE      101

#   define NES_ROMSEL_HI      PORTF |= (1 << 1)
#   define NES_ROMSEL_LOW     PORTF &= ~(1 << 1)
#   define NES_PHI2_HI        PORTF |= (1 << 0)
#   define NES_PHI2_LOW       PORTF &= ~(1 << 0)
#   define NES_PRG_READ       PORTF |= (1 << 7)
#   define NES_PRG_WRITE      PORTF &= ~(1 << 7)
#   define NES_CHR_READ_HI    PORTF |= (1 << 5)
#   define NES_CHR_READ_LOW   PORTF &= ~(1 << 5)
#   define NES_CHR_WRITE_HI   PORTF |= (1 << 2)
#   define NES_CHR_WRITE_LOW  PORTF &= ~(1 << 2)

#   define NES_MODE_WRITE     DDRK = 0xFF
#   define NES_MODE_READ \
  { \
    PORTK = 0xFF; \
    DDRK = 0; \
  }

/**
 * @brief System core for the %NES
 */
namespace OSCR::Cores::NES
{
  struct mapper_t
  {
    uint16_t mapper;
    uint8_t prglo;
    uint8_t prghi;
    uint8_t chrlo;
    uint8_t chrhi;
    uint8_t ramlo;
    uint8_t ramhi;
  };

  struct crdbNESMapperRecord
  {
    uint32_t mapper;
    uint32_t submapper;
    uint32_t prglo;
    uint32_t prghi;
    uint32_t chrlo;
    uint32_t chrhi;
    uint32_t ramlo;
    uint32_t ramhi;
    char name[10];
  };

  struct crdbNESRecord
  {
    crc32_t crc32;
    crc32_t id32a;
    crc32_t id32b;
    uint16_t mapper;
    uint16_t submapper;
    uint16_t prgsize;
    uint16_t prg;
    uint16_t chrsize;
    uint16_t chr;
    uint16_t ramsize;
    uint16_t ram;
    uint8_t iNES[16];
    char name[101];
  };

  extern uint16_t const prgSizes[];
  extern uint16_t const chrSizes[];
  extern uint8_t const ramSizes[];

  extern crdbNESRecord* romDetail;
  extern crdbNESMapperRecord* mapperDetail;

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void menu();
  void dbBrowseMenu();

  void chipMenu();

  void readRom();
  void readRaw();

  uint8_t read_chr_byte(uint16_t address);
  uint8_t read_prg_byte(uint16_t address);
  void write_prg_byte(uint16_t address, uint8_t data);
  void write_reg_byte(uint16_t address, uint8_t data);
  void write_ram_byte(uint16_t address, uint8_t data);
  void write_wram_byte(uint16_t address, uint8_t data);

  void printNESSettings(void);

  //void read(char const * fileSuffix, uint8_t const * header, uint8_t headersize, bool renamerom);
  void read(bool const renamerom);

  void write_mmc1_byte(uint16_t address, uint8_t data);

  void dumpPRG_pulsem2(uint16_t base, uint16_t address);
  void dumpCHR_pulsem2(uint16_t address);

  void CreateRAMFileInSD();

  bool setMapper();
  void checkMapperSize();

  void setPRGSize();
  void setCHRSize();
  void setRAMSize();

  void configureCart();
  bool pickCart();

  void checkMMC6();
  void checkStatus();

  void dumpPRG(uint16_t base, uint16_t address);
  void dumpCHR(uint16_t address);
  void dumpCHR_M2(uint16_t address);
  void dumpMMC5RAM(uint16_t base, uint16_t address);
  void writeMMC5RAM(uint16_t base, uint16_t address);
  void dumpBankPRG(size_t from, size_t to, size_t base);
  void dumpBankCHR(size_t from, size_t to);
  void readPRG(bool readrom);
  void readCHR(bool readrom);
  void readRAM();

  void writeBankPRG(size_t from, size_t to, size_t base);
  void writeBankWRAM(size_t from, size_t to, size_t base);
  void writeRAM();

  void eepromStart();
  void eepromStop();
  void eepromSet0();
  void eepromSet1();
  void eepromStatus();
  void eepromReadData();
  void eepromDevice();
  void eepromReadMode();
  void eepromWriteMode();
  void eepromFinish();
  void eepromSetAddress01(uint8_t address);
  void eepromSetAddress02(uint8_t address);
  void eepromWriteData01(uint8_t & data);
  void eepromWriteData02(uint8_t & data);
  void eepromRead(uint8_t address);
  void eepromWrite(uint8_t address);

#   if defined(ENABLE_PINCONTROL)
  uint8_t read_prg_byte(uint16_t address);
  uint8_t read_chr_byte(uint16_t address);
#   else /* !ENABLE_PINCONTROL */
  //
#   endif /* ENABLE_PINCONTROL */

#   if HAS_FLASH
  void flashMenu();
  void writeFlash();

  void NESmaker_Cmd(uint8_t cmd);
  void NESmaker_ID();
  void NESmaker_SectorErase(uint8_t bank, uint16_t address);
  void NESmaker_ByteProgram(uint8_t bank, uint16_t address, uint8_t data);
  void NESmaker_ChipErase();

  void A29040B_ID();
  void A29040B_PRG_ResetFlash();
  void A29040B_PRG_Write(uint16_t address, uint8_t data);
  void A29040B_PRG_SectorErase(uint16_t sec);
  void A29040B_PRG_ChipErase();
  void A29040B_CHR_ResetFlash();
  void A29040B_CHR_Write(uint16_t address, uint8_t data);
  void A29040B_CHR_SectorErase(uint16_t sec);
  void A29040B_CHR_ChipErase();
  void A29040B_writeFlash();
#   endif /* HAS_FLASH */
} /* namespace OSCR::Cores::NES */

# endif /* HAS_NES */
#endif /* OSCR_CORE_NES_H_ */

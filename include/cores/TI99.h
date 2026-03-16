#pragma once
#if !defined(OSCR_CORE_TI99_H_)
# define OSCR_CORE_TI99_H_

# include "config.h"

# if HAS_TI99
#   include "syslibinc.h"
#   include "common/Types.h"
#   include "common/crc32.h"

#   define ti99CRDB ((OSCR::Databases::TI99 *)cartCRDB)

/**
 * @brief System core for the Texas Instruments TI-99
 */
namespace OSCR::Cores::TI99
{
  struct crdbTI99Record
  {
    crc32_t crc32;
    uint16_t mapper;
    uint16_t size;
    uint16_t gmapper;
    uint16_t gsize;
    char name[101];
  };

  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  uint8_t readROM(uint16_t addr);
  void readSegment(uint16_t startaddr, uint16_t endaddr);
  void setupGROM();
  void pulseGRC(int times);
  void checkGRC();
  void pulseGROM(uint16_t addr);
  void readSegmentGROM(uint32_t startaddr, uint32_t endaddr);
  void writeData(uint16_t addr, uint8_t data);
  void readGROM();
  void readCROM();
  void checkStatus();
  void setMapper();
  void gromMenu();
  void setCart();
} /* namespace OSCR::Cores::TI99 */

# endif /* HAS_TI99 */
#endif /* OSCR_CORE_TI99_H_ */

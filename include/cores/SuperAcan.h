#pragma once
#if !defined(OSCR_CORE_SUPERACAN_H_)
# define OSCR_CORE_SUPERACAN_H_

# include "config.h"

# if HAS_SUPRACAN
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Super A'can
 */
namespace OSCR::Cores::SuperAcan
{
  enum class MenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    WriteFlash,
    RefreshCart,
    UM6650,
    Back,
  };

  constexpr uint8_t const kMenuOptionMax = 7;

  void menu();

  void cartOn();
  void cartOff();

  void openCRDB();
  void closeCRDB();

  void printHeader();

  void writeCommand(uint32_t offset, uint16_t command);
  void readROM();
  void readSRAM();
  void writeSRAM();
  void verifySRAM();
  void readUM6650();
  void writeUM6650();
  void flashCart();
  bool checkROM();
  uint32_t getRomSize();
  void resetCart();
  void writeWord(uint32_t addr, uint16_t data);
  uint16_t readWord(uint32_t addr);
  uint32_t getFlashChipSize(uint16_t chip_id);
} /* namespace OSCR::Cores::SuperAcan */

# endif /* HAS_SUPRACAN */
#endif /* OSCR_CORE_SUPERACAN_H_ */

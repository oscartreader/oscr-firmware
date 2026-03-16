#pragma once
#if !defined(OSCR_CORE_VIRTUALBOY_H_)
# define OSCR_CORE_VIRTUALBOY_H_

# include "config.h"

# if HAS_VBOY
#   include "syslibinc.h"
#   include "common/Types.h"

/**
 * @brief System core for the Virtual Boy
 */
namespace OSCR::Cores::VirtualBoy
{
  enum class MenuOption : uint8_t
  {
    ReadROM,
    ReadSave,
    WriteSave,
    SetSize,
    RefreshCart,
    Back,
  };

  constexpr uint8_t const kMenuOptionMax = 6;

  void menu();

  void openCRDB();
  void closeCRDB();

  void cartOn();
  void cartOff();

  void printHeader();

  void printDetails();

  void writeByte(uint32_t myAddress, uint8_t myData);
  uint16_t readWord(uint32_t myAddress);
  uint8_t readByte(uint32_t myAddress);

  bool checkCart();
  void readROM();
  void writeSRAM();
  void readSRAM();
  void setRomSize();
} /* namespace OSCR::Cores::VirtualBoy */

# endif /* HAS_VBOY */
#endif /* OSCR_CORE_VIRTUALBOY_H_ */

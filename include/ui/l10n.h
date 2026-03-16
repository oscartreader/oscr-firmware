/********************************************************************
 *                   Open Source Cartridge Reader                    *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_H_
# define OSCR_LANG_H_

# include "syslibinc.h"
# include "config.h"
# include "common/Types.h"

namespace OSCR
{
  namespace Lang
  {
    extern void menuHeader(char buffer[], size_t size, char const * flashStr);

    extern char * formatSubmenuTitle(char const * flashStr);

    extern void printUpper(char const * str);
    extern void printUpper(__FlashStringHelper const * flashStr);
    extern void printUpper_P(char const * flashStr);
    extern void printUpper_P(__FlashStringHelper const * flashStr);

    extern void printUpperLine(char const * str);
    extern void printUpperLine(__FlashStringHelper const * flashStr);
    extern void printUpperLine_P(char const * flashStr);
    extern void printUpperLine_P(__FlashStringHelper const * flashStr);

    extern void printBits(uint32_t bits);
    extern void printBitsLine(uint32_t bits);

    extern void printBytes(uint32_t bytes);
    extern void printBytesLine(uint32_t bytes);

    extern void printSwitchVoltage(Voltage voltage);
    extern void printErrorVerifyBytes(uint32_t byteCount);

    namespace Updater
    {
      extern void oscrInfo();
      extern void flagValue(char const flag[], uint8_t value);
      extern void flagValue(char const flag[], uint16_t value);
      extern void flagValue(char const flag[], uint32_t value);
      extern void flagValue(char const flag[], uint64_t value);
      extern void flagValue(char const flag[], char const value[]);
      extern void flagValue(char const flag[]);
      extern void flagValueV(char const flag[], uint8_t values[], size_t valueCount);
      extern void flagValueV(char const flag[], uint32_t values[], size_t valueCount);
    }
  }

  namespace Strings
  {
    // Common words/phrases
    namespace Common
    {
      extern char const PROGMEM OSCR[];

      extern char const PROGMEM ROM[];
      extern char const PROGMEM RAM[];
      extern char const PROGMEM SRAM[];
      extern char const PROGMEM PRG[];
      extern char const PROGMEM CHR[];
      extern char const PROGMEM EEPROM[];
      extern char const PROGMEM Flash[];
      extern char const PROGMEM None[];

      extern char const PROGMEM Lower[];
      extern char const PROGMEM Upper[];

      extern char const PROGMEM Save[];
      extern char const PROGMEM Blank[];
      extern char const PROGMEM NotBlank[];

      extern char const PROGMEM Invalid[];
      extern char const PROGMEM Unknown[];

      extern char const PROGMEM Finished[];
      extern char const PROGMEM Done[];

      extern char const PROGMEM OK[];
      extern char const PROGMEM FAIL[];
      extern char const PROGMEM DONE[];
    }

    namespace MenuOptions // most work as headers too
    {
      extern char const PROGMEM Cartridge[];
      extern char const PROGMEM Controller[];

      extern char const PROGMEM ReadROM[];
      extern char const PROGMEM WriteFlash[];
      extern char const PROGMEM ReadSave[];
      extern char const PROGMEM WriteSave[];

      extern char const PROGMEM SetCartType[];
      extern char const PROGMEM SetCartSize[];
      extern char const PROGMEM SetROMSize[];
      extern char const PROGMEM SetMapper[];
      extern char const PROGMEM SetSaveType[];
      extern char const PROGMEM SetSaveSize[];
      extern char const PROGMEM SetSize[];

      extern char const PROGMEM SelectCart[];
      extern char const PROGMEM SelectFile[];

      extern char const PROGMEM RefreshCart[];
      extern char const PROGMEM CartInfo[];

      extern char const PROGMEM Read[];
      extern char const PROGMEM Write[];
      extern char const PROGMEM Erase[];

      extern char const PROGMEM Abort[];
      extern char const PROGMEM Retry[];
      extern char const PROGMEM Continue[];

      extern char const PROGMEM Yes[];
      extern char const PROGMEM No[];

      extern char const PROGMEM Back[];
      extern char const PROGMEM Reset[];

      extern char const PROGMEM Unlock[];
    }

    // Power
    namespace Power
    {
      extern char const PROGMEM VSELECT[];
      extern char const PROGMEM VCC[];
      extern char const PROGMEM Voltage3V3[];
      extern char const PROGMEM Voltage5[];
    }

    // Sizes
    namespace Units
    {
      extern char const PROGMEM Size32B[];
      extern char const PROGMEM Size64B[];
      extern char const PROGMEM Size128B[];
      extern char const PROGMEM Size256B[];
      extern char const PROGMEM Size512B[];
      extern char const PROGMEM Size1KB[];
      extern char const PROGMEM Size2KB[];
      extern char const PROGMEM Size4KB[];
      extern char const PROGMEM Size8KB[];
      extern char const PROGMEM Size16KB[];
      extern char const PROGMEM Size24KB[];
      extern char const PROGMEM Size32KB[];
      extern char const PROGMEM Size64KB[];
      extern char const PROGMEM Size128KB[];
      extern char const PROGMEM Size256KB[];
      extern char const PROGMEM Size512KB[];
      extern char const PROGMEM Size1MB[];
      extern char const PROGMEM Size2MB[];
      extern char const PROGMEM Size4MB[];
      extern char const PROGMEM Size8MB[];
      extern char const PROGMEM Size12MB[];
      extern char const PROGMEM Size16MB[];
      extern char const PROGMEM Size32MB[];
      extern char const PROGMEM Size64MB[];
      extern char const PROGMEM Size128MB[];

      extern char const PROGMEM B[];
      extern char const PROGMEM K[];
      extern char const PROGMEM M[];
      extern char const PROGMEM KB[];
      extern char const PROGMEM MB[];
    }

    // Symbols
    namespace Symbol
    {
      // Generic Symbols
      extern char const PROGMEM Empty[];
      extern char const PROGMEM Space[];
      extern char const PROGMEM Comma[];
      extern char const PROGMEM Plus[];
      extern char const PROGMEM Minus[];
      extern char const PROGMEM Percent[];
      extern char const PROGMEM Asterisk[];
      extern char const PROGMEM Slash[];
      extern char const PROGMEM MenuSpaces[];
      extern char const PROGMEM MenuSelection[];
      extern char const PROGMEM Arrow[];
      extern char const PROGMEM NotEqual[];
      extern char const PROGMEM LabelEnd[];
      extern char const PROGMEM FullStop[];
      extern char const PROGMEM Ellipsis[];
      extern char const PROGMEM NewLine[];
      extern char const PROGMEM X[];

      extern char const PROGMEM PaddedSlash[];
      extern char const PROGMEM PaddedX[];

      // Progress Bar
      extern char const PROGMEM ProgressBarOpen[];
      extern char const PROGMEM ProgressBarClose[];
      extern char const PROGMEM ProgressBarEmpty[];
      extern char const PROGMEM ProgressBarFilled[];
      extern char const PROGMEM ProgressBarUnknown[];
    }

    namespace Names
    {
      extern char const PROGMEM NormalCFI8[];
      extern char const PROGMEM SwitchedCFI8[];
      extern char const PROGMEM NormalCFI16[];
      extern char const PROGMEM SwitchedCFI16[];
    }

    // Headings
    namespace Headings
    {
      extern char const PROGMEM OSCR[];
      extern char const PROGMEM CRDB[];

      extern char const PROGMEM ChangeVoltage[];
      extern char const PROGMEM OverrideVoltage[];

      extern char const PROGMEM BlankCheck[];

      extern char const PROGMEM SelectCartSize[];
      extern char const PROGMEM SelectSaveSize[];
      extern char const PROGMEM SelectBufferSize[];
      extern char const PROGMEM SelectSectorSize[];
      extern char const PROGMEM SelectMapper[];
      extern char const PROGMEM SelectType[];
      extern char const PROGMEM SelectOne[];
      extern char const PROGMEM SelectCRDBEntry[];

      extern char const PROGMEM HardwareProblem[];

      extern char const PROGMEM FatalError[];
      extern char const PROGMEM CartridgeError[];
      extern char const PROGMEM SD[];

      extern char const PROGMEM ConfirmErase[];

      extern char const PROGMEM Warning[];

      extern char const PROGMEM CRDBDebugROM[];
      extern char const PROGMEM CRDBDebugMapper[];
      extern char const PROGMEM CRDBDebugEnd[];
    }

    // Errors
    namespace Errors
    {
      extern char const PROGMEM StorageError[];
      extern char const PROGMEM BuffereOverflow[];
      extern char const PROGMEM NameOverflow[];
      extern char const PROGMEM ClockGenMissing[];
      extern char const PROGMEM RTCMissing[];

      extern char const PROGMEM NoSave[];
      extern char const PROGMEM NotLargeEnough[];
      extern char const PROGMEM NotSupportedByCart[];
      extern char const PROGMEM HeaderNotFound[];
      extern char const PROGMEM InvalidType[];
      extern char const PROGMEM UnknownType[];
      extern char const PROGMEM IncorrectChecksum[];
      extern char const PROGMEM IncorrectFileSize[];
      extern char const PROGMEM UnlockFailed[];
      extern char const PROGMEM TimedOut[];
      extern char const PROGMEM NoDataReceived[];

      extern char const PROGMEM DatabaseError[];
      extern char const PROGMEM DatabaseNotFound[];
      extern char const PROGMEM NotFoundDB[];

      extern char const PROGMEM IncorrectVoltage[];
    }

    // Statuses
    namespace Status
    {
      extern char const PROGMEM Reading[];
      extern char const PROGMEM Writing[];
      extern char const PROGMEM Erasing[];
      extern char const PROGMEM Verifying[];
      extern char const PROGMEM Checking[];
      extern char const PROGMEM Searching[];
      extern char const PROGMEM Checksum[];
      extern char const PROGMEM CRC32[];
      extern char const PROGMEM Locking[];
      extern char const PROGMEM Unlocking[];
      extern char const PROGMEM SearchingDatabase[];
      extern char const PROGMEM PressButton[];
    }

    // Labels
    namespace Labels
    {
      extern char const PROGMEM CHK[];
      extern char const PROGMEM CHECKSUM[];
      extern char const PROGMEM ERROR[];
      extern char const PROGMEM MAPPER[];
      extern char const PROGMEM SUBMAPPER[];
      extern char const PROGMEM NUMBER[];
      extern char const PROGMEM NAME[];
      extern char const PROGMEM TYPE[];
      extern char const PROGMEM BANK[];
      extern char const PROGMEM BANKS[];
      extern char const PROGMEM SAVE[];
      extern char const PROGMEM SAVE_SIZE[];
      extern char const PROGMEM SAVE_TYPE[];
      extern char const PROGMEM REVISION[];
      extern char const PROGMEM RAM[];
      extern char const PROGMEM RAM_SIZE[];
      extern char const PROGMEM ROM[];
      extern char const PROGMEM ROM_SIZE[];
      extern char const PROGMEM SIZE[];
      extern char const PROGMEM Address[];

      extern char const PROGMEM ID[];
      extern char const PROGMEM CRCSum[];
      extern char const PROGMEM Selected[];
      extern char const PROGMEM SizeLow[];
      extern char const PROGMEM SizeHigh[];

      extern char const PROGMEM File[];
    }

    // Templates
    namespace Templates
    {
      extern char const PROGMEM PaddedHex2[];

      extern char const PROGMEM SizeBits[];
      extern char const PROGMEM SizeK[];
      extern char const PROGMEM SizeM[];
      extern char const PROGMEM SizeG[];

      extern char const PROGMEM SizeBytes[];
      extern char const PROGMEM SizeKB[];
      extern char const PROGMEM SizeMB[];
      extern char const PROGMEM SizeGB[];

      extern char const PROGMEM Browse[];

      extern char const PROGMEM VoltageSwitchTo[];

      extern char const PROGMEM ErrorVerifyBytes[];

      extern char const PROGMEM OSCRHeaderPrefix[];
    }

    namespace Features
    {
      extern char const PROGMEM CheckVoltage[];
      extern char const PROGMEM Settings[];
      extern char const PROGMEM About[];
      extern char const PROGMEM Reset[];
    }

    namespace Directory
    {
      extern char const PROGMEM ROM[];
      extern char const PROGMEM MPK[];
      extern char const PROGMEM Save[];
      extern char const PROGMEM Raw[];
      extern char const PROGMEM SIMM[];
    }

    namespace FileType
    {
      extern char const PROGMEM Save[];
      extern char const PROGMEM SaveRAM[];
      extern char const PROGMEM SaveRA[];
      extern char const PROGMEM SaveFlash[];
      extern char const PROGMEM SaveEEPROM[];
      extern char const PROGMEM SaveBackup[];
      extern char const PROGMEM Raw[];
      extern char const PROGMEM Map[];
      extern char const PROGMEM CRCX[];
      extern char const PROGMEM U2[];

      extern char const PROGMEM DefaultName[];

      extern char const PROGMEM GameBoy[];
      extern char const PROGMEM GBMemoryModule[];
      extern char const PROGMEM GBSmartModule[];
      extern char const PROGMEM GameBoyAdvance[];
      extern char const PROGMEM NES[];
      extern char const PROGMEM SNES[];
      extern char const PROGMEM SNESD[];
      extern char const PROGMEM Satellaview[];
      extern char const PROGMEM SFM[];
      extern char const PROGMEM SFM_NP[];
      extern char const PROGMEM ST[];
      extern char const PROGMEM GPC[];
      extern char const PROGMEM N64[];
      extern char const PROGMEM N64F[];
      extern char const PROGMEM MPK[];
      extern char const PROGMEM MegaDrive[];
      extern char const PROGMEM MegaDrive32X[];
      extern char const PROGMEM SMS[];
      extern char const PROGMEM GameGear[];
      extern char const PROGMEM SG1000[];
      extern char const PROGMEM PCEngine[];
      extern char const PROGMEM WonderSwan[];
      extern char const PROGMEM NeoGeoPocket[];
      extern char const PROGMEM Intellivision[];
      extern char const PROGMEM Colecovision[];
      extern char const PROGMEM VirtualBoy[];
      extern char const PROGMEM Supervision[];
      extern char const PROGMEM PocketChallengeW[];
      extern char const PROGMEM Atari2600[];
      extern char const PROGMEM Atari5200[];
      extern char const PROGMEM Atari7800[];
      extern char const PROGMEM Odyssey2[];
      extern char const PROGMEM Arcadia2001[];
      extern char const PROGMEM ChannelF[];
      extern char const PROGMEM SuperAcan[];
      extern char const PROGMEM MSX[];
      extern char const PROGMEM PokemonMini[];
      extern char const PROGMEM CasioLoopy[];
      extern char const PROGMEM Commodore64[];
      extern char const PROGMEM AtariJaguar[];
      extern char const PROGMEM AtariLynx[];
      extern char const PROGMEM Vectrex[];
      extern char const PROGMEM Atari8[];
      extern char const PROGMEM BallyAstrocade[];
      extern char const PROGMEM LittleJammer[];
      extern char const PROGMEM LittleJammerPro[];
      extern char const PROGMEM CasioPV1000[];
      extern char const PROGMEM VIC20[];
      extern char const PROGMEM Leapster[];
      extern char const PROGMEM RCAStudio2[];
      extern char const PROGMEM TI99[];
      extern char const PROGMEM TomyPyuuta[];
      extern char const PROGMEM TRS80[];
      extern char const PROGMEM VSmile[];
      extern char const PROGMEM CPS3[];
      extern char const PROGMEM Flash[];
    }

    namespace Cores
    {
      extern char const PROGMEM GameBoy[];
      extern char const PROGMEM GBMemoryModule[];
      extern char const PROGMEM GBSmartModule[];
      extern char const PROGMEM GameBoyAdvance[];
      extern char const PROGMEM NES[];
      extern char const PROGMEM SNES[];
      extern char const PROGMEM Satellaview[];
      extern char const PROGMEM SFM[];
      extern char const PROGMEM ST[];
      extern char const PROGMEM GPC[];
      extern char const PROGMEM N64[];
      extern char const PROGMEM MegaDrive[];
      extern char const PROGMEM SMSGGSG[];
      extern char const PROGMEM SMS[];
      extern char const PROGMEM GameGear[];
      extern char const PROGMEM SG1000[];
      extern char const PROGMEM PCEngine[];
      extern char const PROGMEM WonderSwan[];
      extern char const PROGMEM NeoGeoPocket[];
      extern char const PROGMEM Intellivision[];
      extern char const PROGMEM Colecovision[];
      extern char const PROGMEM VirtualBoy[];
      extern char const PROGMEM Supervision[];
      extern char const PROGMEM PocketChallengeW[];
      extern char const PROGMEM Atari2600[];
      extern char const PROGMEM Atari5200[];
      extern char const PROGMEM Atari7800[];
      extern char const PROGMEM Odyssey2[];
      extern char const PROGMEM Arcadia2001[];
      extern char const PROGMEM ChannelF[];
      extern char const PROGMEM SuperAcan[];
      extern char const PROGMEM MSX[];
      extern char const PROGMEM PokemonMini[];
      extern char const PROGMEM CasioLoopy[];
      extern char const PROGMEM Commodore64[];
      extern char const PROGMEM AtariJaguar[];
      extern char const PROGMEM AtariLynx[];
      extern char const PROGMEM Vectrex[];
      extern char const PROGMEM Atari8[];
      extern char const PROGMEM BallyAstrocade[];
      extern char const PROGMEM LittleJammer[];
      extern char const PROGMEM LittleJammerPro[];
      extern char const PROGMEM CasioPV1000[];
      extern char const PROGMEM VIC20[];
      extern char const PROGMEM Leapster[];
      extern char const PROGMEM RCAStudio2[];
      extern char const PROGMEM TI99[];
      extern char const PROGMEM TomyPyuuta[];
      extern char const PROGMEM TRS80[];
      extern char const PROGMEM VSmile[];
      extern char const PROGMEM Flashrom[];
      extern char const PROGMEM CPS3[];
      extern char const PROGMEM SelfTest[];
    }
  }
}

#endif /* OSCR_LANG_H_ */

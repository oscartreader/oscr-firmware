/********************************************************************
 *                   Open Source Cartridge Reader                   *
 ********************************************************************/
#pragma once
#ifndef OSCR_LANG_EN_H_
#define OSCR_LANG_EN_H_

# if OSCR_LANGUAGE == LANG_EN

namespace OSCR::Strings
{
  // Common words/phrases
  namespace Common
  {
    constexpr char const PROGMEM OSCR[]             = "OSCR";

    constexpr char const PROGMEM ROM[]              = "ROM";
    constexpr char const PROGMEM RAM[]              = "RAM";
    constexpr char const PROGMEM SRAM[]             = "SRAM";
    constexpr char const PROGMEM PRG[]              = "PRG";
    constexpr char const PROGMEM CHR[]              = "CHR";
    constexpr char const PROGMEM EEPROM[]           = "EEPROM";
    constexpr char const PROGMEM Flash[]            = "Flash";
    constexpr char const PROGMEM None[]             = "None";

    constexpr char const PROGMEM Lower[]            = "Lower";
    constexpr char const PROGMEM Upper[]            = "Upper";

    constexpr char const PROGMEM Save[]             = "Save";

    constexpr char const PROGMEM Blank[]            = "Blank";
    constexpr char const PROGMEM NotBlank[]         = "Not Blank";

    constexpr char const PROGMEM Invalid[]          = "Invalid";
    constexpr char const PROGMEM Unknown[]          = "Unknown";

    constexpr char const PROGMEM Finished[]         = "Finished";
    constexpr char const PROGMEM Done[]             = "Done";

    constexpr char const PROGMEM OK[]               = "OK";
    constexpr char const PROGMEM FAIL[]             = "FAIL";
    constexpr char const PROGMEM DONE[]             = "DONE";
  }

  namespace MenuOptions // most work as headers too
  {
    constexpr char const PROGMEM Cartridge[]        = "Cartridge";
    constexpr char const PROGMEM Controller[]       = "Controller";

    constexpr char const PROGMEM ReadROM[]          = "Read ROM";
    constexpr char const PROGMEM WriteFlash[]       = "Write Flash";
    constexpr char const PROGMEM ReadSave[]         = "Read Save";
    constexpr char const PROGMEM WriteSave[]        = "Write Save";

    constexpr char const PROGMEM SetCartType[]      = "Set Cart Type";
    constexpr char const PROGMEM SetCartSize[]      = "Set Cart Size";
    constexpr char const PROGMEM SetROMSize[]       = "Set ROM Size";
    constexpr char const PROGMEM SetMapper[]        = "Set Mapper";
    constexpr char const PROGMEM SetSaveType[]      = "Set Save Type";
    constexpr char const PROGMEM SetSaveSize[]      = "Set Save Size";
    constexpr char const PROGMEM SetSize[]          = "Set Size";

    constexpr char const PROGMEM SelectCart[]       = "Select Cart";
    constexpr char const PROGMEM SelectFile[]       = "Select File";

    constexpr char const PROGMEM RefreshCart[]      = "Refresh Cart";
    constexpr char const PROGMEM CartInfo[]         = "Cart Info";

    constexpr char const PROGMEM Read[]             = "Read";
    constexpr char const PROGMEM Write[]            = "Write";
    constexpr char const PROGMEM Erase[]            = "Erase";

    constexpr char const PROGMEM Abort[]            = "Abort";
    constexpr char const PROGMEM Retry[]            = "Retry";
    constexpr char const PROGMEM Continue[]         = "Continue";

    constexpr char const PROGMEM Yes[]              = "Yes";
    constexpr char const PROGMEM No[]               = "No";

    constexpr char const PROGMEM Back[]             = "Back";
    constexpr char const PROGMEM Reset[]            = "Reset";

    constexpr char const PROGMEM Unlock[]           = "Unlock";
  }

  namespace Names
  {
    constexpr char const PROGMEM NormalCFI8[]       = "Normal CFI x8";
    constexpr char const PROGMEM SwitchedCFI8[]     = "Switched CFI x8";
    constexpr char const PROGMEM NormalCFI16[]      = "Normal CFI x16";
    constexpr char const PROGMEM SwitchedCFI16[]    = "Switched CFI x16";
  }

  // Headings
  namespace Headings
  {
    constexpr char const PROGMEM OSCR[]             = "Open Source Cart Reader";
    constexpr char const PROGMEM CRDB[]             = "Cartridge Reader Database";

    constexpr char const PROGMEM ChangeVoltage[]    = "Change Voltage";
    constexpr char const PROGMEM OverrideVoltage[]  = "Override Voltage";

    constexpr char const PROGMEM BlankCheck[]       = "Blank Check";

    constexpr char const PROGMEM SelectCartSize[]   = "Select Cartridge Size";
    constexpr char const PROGMEM SelectSaveSize[]   = "Select Save Size";
    constexpr char const PROGMEM SelectBufferSize[] = "Select Buffer Size";
    constexpr char const PROGMEM SelectSectorSize[] = "Select Sector Size";
    constexpr char const PROGMEM SelectMapper[]     = "Select Mapper";
    constexpr char const PROGMEM SelectType[]       = "Select Type";
    constexpr char const PROGMEM SelectOne[]        = "Select One";

    constexpr char const PROGMEM SelectCRDBEntry[]  = "Select CRDB Entry";

    constexpr char const PROGMEM HardwareProblem[]  = "Hardware Problem";

    constexpr char const PROGMEM FatalError[]       = "Fatal Error";
    constexpr char const PROGMEM CartridgeError[]   = "Cartridge Error";
    constexpr char const PROGMEM SD[]               = "SD Error";

    constexpr char const PROGMEM ConfirmErase[]     = "Erase Data?";

    constexpr char const PROGMEM Warning[]          = "!!!! WARNING !!!!";

    constexpr char const PROGMEM CRDBDebugROM[]     = " ===== ROM DEBUG INFO ===== ";
    constexpr char const PROGMEM CRDBDebugMapper[]  = " === MAPPER  DEBUG INFO === ";
    constexpr char const PROGMEM CRDBDebugEnd[]     = " ========================== ";
  }

  // Errors
  namespace Errors
  {
    constexpr char const PROGMEM StorageError[]       = "SD Error";
    constexpr char const PROGMEM BuffereOverflow[]    = "Buffer overflow!";
    constexpr char const PROGMEM NameOverflow[]       = "Name exceeds limit";
    constexpr char const PROGMEM ClockGenMissing[]    = "Clock Generator not found";
    constexpr char const PROGMEM RTCMissing[]         = "RTC not found";

    constexpr char const PROGMEM HeaderNotFound[]     = "Header not found";
    constexpr char const PROGMEM NoSave[]             = "Cart doesn't have saves";
    constexpr char const PROGMEM NotLargeEnough[]     = "Cart is not large enough";
    constexpr char const PROGMEM NotSupportedByCart[] = "Not supported by cartridge";
    constexpr char const PROGMEM InvalidType[]        = "Invalid type";
    constexpr char const PROGMEM UnknownType[]        = "Unknown type";
    constexpr char const PROGMEM IncorrectChecksum[]  = "Incorrect Checksum";
    constexpr char const PROGMEM IncorrectFileSize[]  = "Incorrect File Size";
    constexpr char const PROGMEM UnlockFailed[]       = "Unlock Failed";
    constexpr char const PROGMEM TimedOut[]           = "Timed Out";
    constexpr char const PROGMEM NoDataReceived[]     = "No Data Received";

    constexpr char const PROGMEM DatabaseError[]      = "Database Error";
    constexpr char const PROGMEM DatabaseNotFound[]   = "Database file not found";
    constexpr char const PROGMEM NotFoundDB[]         = "Not found in database";

    constexpr char const PROGMEM IncorrectVoltage[]   = "Incorrect voltage, aborting";
  }

  // Statuses
  namespace Status
  {
    constexpr char const PROGMEM Reading[]            = "Reading... ";
    constexpr char const PROGMEM Writing[]            = "Writing... ";
    constexpr char const PROGMEM Erasing[]            = "Erasing... ";
    constexpr char const PROGMEM Verifying[]          = "Verifying... ";
    constexpr char const PROGMEM Checking[]           = "Checking... ";
    constexpr char const PROGMEM Searching[]          = "Searching... ";
    constexpr char const PROGMEM Checksum[]           = "Checksum... ";
    constexpr char const PROGMEM CRC32[]              = "CRC32... ";
    constexpr char const PROGMEM Locking[]            = "Locking... ";
    constexpr char const PROGMEM Unlocking[]          = "Unlocking... ";
    constexpr char const PROGMEM SearchingDatabase[]  = "Searching database... ";
    constexpr char const PROGMEM PressButton[]        = "Press Button... ";
  }

  // Labels
  namespace Labels
  {
    constexpr char const PROGMEM CHK[]        = "CHK: ";
    constexpr char const PROGMEM CHECKSUM[]   = "Checksum: ";
    constexpr char const PROGMEM ERROR[]      = "Error: ";
    constexpr char const PROGMEM FILE_SIZE[]  = "File Size: ";
    constexpr char const PROGMEM MAPPER[]     = "Mapper: ";
    constexpr char const PROGMEM SUBMAPPER[]  = "Submapper: ";
    constexpr char const PROGMEM NUMBER[]     = "Number: ";
    constexpr char const PROGMEM NAME[]       = "Name: ";
    constexpr char const PROGMEM TYPE[]       = "Type: ";
    constexpr char const PROGMEM BANK[]       = "Bank: ";
    constexpr char const PROGMEM BANKS[]      = "Banks: ";
    constexpr char const PROGMEM SAVE[]       = "Save: ";
    constexpr char const PROGMEM SAVE_SIZE[]  = "Save Size: ";
    constexpr char const PROGMEM SAVE_TYPE[]  = "Save Type: ";
    constexpr char const PROGMEM REVISION[]   = "Revision: ";
    constexpr char const PROGMEM RAM[]        = "RAM: ";
    constexpr char const PROGMEM RAM_SIZE[]   = "RAM Size: ";
    constexpr char const PROGMEM ROM[]        = "ROM: ";
    constexpr char const PROGMEM ROM_SIZE[]   = "ROM Size: ";
    constexpr char const PROGMEM SIZE[]       = "Size: ";
    constexpr char const PROGMEM Address[]    = "Address: ";

    constexpr char const PROGMEM ID[]         = "ID: ";
    constexpr char const PROGMEM CRCSum[]     = "CRC: ";
    constexpr char const PROGMEM Selected[]   = "Selected: ";
    constexpr char const PROGMEM SizeLow[]    = "Size Low: ";
    constexpr char const PROGMEM SizeHigh[]   = "Size High: ";

    constexpr char const PROGMEM File[]       = "File:";
  }

  // Templates
  namespace Templates
  {
    constexpr char const PROGMEM VoltageSwitchTo[]    = "Set voltage switch to %s";

    constexpr char const PROGMEM Browse[]             = "Browse %s";

    constexpr char const PROGMEM ErrorVerifyBytes[]   = "Error: %" PRIu32 " bytes\r\n did not verify.";

    constexpr char const PROGMEM FileMustBe[]         = "File must be %s";
    constexpr char const PROGMEM FileMustBeU32[]      = "File must be %" PRIu32;

#   if defined(ENABLE_SERIAL_ANSI)
    constexpr char const PROGMEM OSCRHeaderPrefix[]   = "OSCR - %s";
#   else
    constexpr char const PROGMEM OSCRHeaderPrefix[]   = "OSCR: %s";
#   endif
  }

  namespace Features
  {
    constexpr char const PROGMEM Settings[]           = "Settings";
    constexpr char const PROGMEM About[]              = "About";
    constexpr char const PROGMEM Reset[]              = "Reset";
    constexpr char const PROGMEM CheckVoltage[]       = "Check Voltage";
  }

  namespace Directory
  {
    constexpr char const PROGMEM ROM[]                = "ROM";
    constexpr char const PROGMEM MPK[]                = "MPK";
    constexpr char const PROGMEM Save[]               = "SAVE";
    constexpr char const PROGMEM Raw[]                = "RAW";
    constexpr char const PROGMEM SIMM[]               = "SIMM";
  }
}
# endif /* OSCR_LANGUAGE == EN */
#endif /* OSCR_LANG_EN_H_ */

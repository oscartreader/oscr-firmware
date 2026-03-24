/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/

#include "common/crdb/flash.h"
#include "apps/CRDBBrowser.h"
#include "cores/include.h"
#include "cores/flash.h"

namespace
{
  // No direct access to help prevent people introducing leaks
  OSCR::Databases::Flash::Flash * crdbFlash;

  char currentDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
}

namespace OSCR::Databases::Flash
{
  crdbFlashRecord * flashDetail = nullptr;
  FlashRecord * flashRecord = nullptr;

  bool flashReady = false;

  void resetFlashRecord()
  {
    flashDetail = nullptr;
    flashRecord = nullptr;
    OSCR::Cores::fromCRDB = false;
  }

  void resetFlash()
  {
    if (flashReady) delete crdbFlash;

    crdbFlash = nullptr;

    resetFlashRecord();

    flashReady = false;
  }

  void _setupFlash(char const * newDatabasePath)
  {
    if (!strcmp(currentDatabasePath, newDatabasePath)) return;

    resetFlash();

    crdbFlash = new Flash(currentDatabasePath);
    flashReady = true;
  }

  void setupFlash(char const * databasePath)
  {
    char newDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName(databasePath, newDatabasePath);

    _setupFlash(newDatabasePath);
  }

  void setupFlash(__FlashStringHelper const * databaseName)
  {
    char newDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName_P(databaseName, newDatabasePath);

    _setupFlash(newDatabasePath);
  }

  void _updateFlashRecord()
  {
    flashRecord = crdbFlash->record();
    flashDetail = flashRecord->data();
  }

  void browseFlash()
  {
    OSCR::crdbBrowser(FS(OSCR::Strings::Headings::SelectFlash), crdbFlash);
    _updateFlashRecord();
  }

  bool matchFlash(uint16_t searchId, uint8_t offset)
  {
    if (!crdbFlash->findRecord(searchId, offset))
    {
      resetFlashRecord();
      return false;
    }

    _updateFlashRecord();

    return true;
  }

  void printFlashNotFound(uint16_t flashId)
  {
    OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CRDB));
    OSCR::UI::printLabel(OSCR::Strings::Common::Flash);
    OSCR::UI::printHexLine(flashId);
    OSCR::UI::error(FS(OSCR::Strings::Errors::NotFoundDB));
  }

#pragma region FlashRecord

  char const * FlashRecord::getName() const
  {
    return _data.name;
  }

  bool FlashRecord::readName(char * buffer, size_t size) const
  {
    return OSCR::Util::copyStr(buffer, size, _data.name);
  }

  void FlashRecord::debug() const
  {
#   if CRDB_DEBUGGING
    OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugFlash));
    OSCR::Serial::printValue(OSCR::Strings::Common::ID, _data.id);
    OSCR::Serial::printValue(OSCR::Strings::Common::Name, _data.name);
    OSCR::Serial::printType(OSCR::Strings::Common::Flash, _data.typeName);
    OSCR::Serial::printType(OSCR::Strings::Common::Flash, static_cast<uint8_t>(_data.type));
    OSCR::Serial::printValue(OSCR::Strings::Common::Variant, _data.variant);
    OSCR::Serial::printValue(OSCR::Strings::Units::Bits, _data.bits);
    OSCR::Serial::printValue(OSCR::Strings::Common::Size, _data.size);
    OSCR::Serial::printValue(OSCR::Strings::Common::Sectors, _data.sector);
    OSCR::Serial::printValue(OSCR::Strings::Common::Buffer, _data.buffer);
    OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
#   endif /* CRDB_DEBUGGING */
  }

  void Flash::nextRecord()
  {
    uint8_t tempType = 0;
    clearError();
    readNum32(&currentRecord->data()->id);
    readNum32(&currentRecord->data()->variant);
    readNum32(&currentRecord->data()->bits);
    readNum32(&currentRecord->data()->size);
    readNum32(&currentRecord->data()->sector);
    readNum32(&currentRecord->data()->buffer);
    readNum8(&currentRecord->data()->voltage);
    readNum8(&tempType);
    readBytes(&currentRecord->data()->typeName, 50);
    readBytes(&currentRecord->data()->name, 50);

    currentRecord->data()->type = static_cast<FlashType>(tempType);
  }

  Flash * getFlashCRDB()
  {
    return crdbFlash;
  }
}

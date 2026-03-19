/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/

#include "common/crdb/mapper.h"
#include "apps/CRDBBrowser.h"
#include "cores/include.h"

namespace
{
  // No direct access to help prevent people introducing leaks
  OSCR::Databases::Basic::CRMapperDatabase * crdbMapper;

  char currentDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
}

namespace OSCR::Databases::Basic
{
  crdbMapperRecord * mapperDetail = nullptr;
  MapperRecord * mapperRecord = nullptr;

  bool mapperReady = false;

  void resetMapperRecord()
  {
    mapperDetail = nullptr;
    mapperRecord = nullptr;
    OSCR::Cores::fromCRDB = false;
  }

  void resetMapper()
  {
    if (mapperReady) delete crdbMapper;

    crdbMapper = nullptr;

    resetMapperRecord();

    mapperReady = false;
  }

  void _setupMapper(char const * newDatabasePath)
  {
    if (!strcmp(currentDatabasePath, newDatabasePath)) return;

    resetMapper();

    crdbMapper = new CRMapperDatabase(currentDatabasePath);
    mapperReady = true;
  }

  void setupMapper(char const * databasePath)
  {
    char newDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName(databasePath, newDatabasePath);

    _setupMapper(newDatabasePath);
  }

  void setupMapper(__FlashStringHelper const * databaseName)
  {
    char newDatabasePath[OSCR::CRDB::kDefaultDatabasePathLength];
    OSCR::CRDB::dbName_P(databaseName, newDatabasePath);

    _setupMapper(newDatabasePath);
  }

  void _updateMapperRecord()
  {
    mapperRecord = crdbMapper->record();
    mapperDetail = mapperRecord->data();
  }

  void browseMappers()
  {
    OSCR::crdbBrowser(FS(OSCR::Strings::Headings::SelectMapper), crdbMapper);
    _updateMapperRecord();
  }

  void browseMappers(char const * databasePath)
  {
    setupMapper(databasePath);
    browseMappers();
  }

  void browseMappers(__FlashStringHelper const * databaseName)
  {
    setupMapper(databaseName);
    browseMappers();
  }

  bool matchMapper(uint16_t searchId, uint8_t offset)
  {
    if (!crdbMapper->findRecord(searchId, offset))
    {
      resetMapperRecord();
      return false;
    }

    _updateMapperRecord();

    return true;
  }

  bool matchMapper(char const * databasePath, uint16_t searchId, uint8_t offset)
  {
    setupMapper(databasePath);
    return matchMapper(searchId, offset);
  }

  bool matchMapper(__FlashStringHelper const * databaseName, uint16_t searchId, uint8_t offset)
  {
    setupMapper(databaseName);
    return matchMapper(searchId, offset);
  }

  bool searchMapper(char const * databasePath, uint16_t searchId)
  {
    setupMapper(databasePath);
    return matchMapper(searchId);
  }

  bool searchMapper(__FlashStringHelper const * databaseName, uint16_t searchId)
  {
    setupMapper(databaseName);
    return matchMapper(searchId);
  }

  void printMapperNotFound(uint16_t mapperId)
  {
    OSCR::UI::printErrorHeader(FS(OSCR::Strings::Headings::CRDB));
    OSCR::UI::printLabel(OSCR::Strings::Common::Mapper);
    OSCR::UI::printHexLine(mapperId);
    OSCR::UI::error(FS(OSCR::Strings::Errors::NotFoundDB));
  }

#pragma region MapperRecord

  char const * MapperRecord::getName() const
  {
    return _data.name;
  }

  bool MapperRecord::readName(char * buffer, size_t size) const
  {
    return OSCR::Util::copyStr(buffer, size, _data.name);
  }

  void MapperRecord::debug() const
  {
#   if CRDB_DEBUGGING
    OSCR::Serial::printLine(FS(OSCR::Strings::Headings::CRDBDebugMapper));
    OSCR::Serial::print(FS(OSCR::Strings::Labels::ID));
    OSCR::Serial::printLine(_data.id);
    OSCR::Serial::print(FS(OSCR::Strings::Labels::NAME));
    OSCR::Serial::printLine(_data.name);
    OSCR::Serial::print(FS(OSCR::Strings::Labels::SizeLow));
    OSCR::Serial::printLine(_data.sizeLow);
    OSCR::Serial::print(FS(OSCR::Strings::Labels::SizeHigh));
    OSCR::Serial::printLine(_data.sizeHigh);
    OSCR::Serial::print(F("Meta1: "));
    OSCR::Serial::printLine(_data.meta1);
    OSCR::Serial::print(F("Meta2: "));
    OSCR::Serial::printLine(_data.meta2);
    OSCR::Serial::printLine(FS(OSCR::Strings::Headings::CRDBDebugEnd));
#   endif /* CRDB_DEBUGGING */
  }

  void CRMapperDatabase::nextRecord()
  {
    clearError();
    readNum16(&currentRecord->data()->id);
    readNum32(&currentRecord->data()->sizeLow);
    readNum32(&currentRecord->data()->sizeHigh);
    readNum32(&currentRecord->data()->meta1);
    readNum32(&currentRecord->data()->meta2);
    readBytes(&currentRecord->data()->name, 100);
  }

  CRMapperDatabase * getMapperCRDB()
  {
    return crdbMapper;
  }
}

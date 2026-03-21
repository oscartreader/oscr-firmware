/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/

#include "common/crdb/extended.h"

#include "apps.h"

namespace OSCR::Databases::Extended
{
  crdbRecord * romDetail;
  ExtendedRecord * romRecord;
  CRDatabase * crdb;
  bool ready = false;

  uint32_t ExtendedRecord::getCRC32()
  {
    return _data.crc32;
  }

  void ExtendedRecord::debug()
  {
# if CRDB_DEBUGGING
    OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugROM));
    OSCR::Serial::printValue(OSCR::Strings::Common::Name, _data.name);
    OSCR::Serial::printValue(OSCR::Strings::Common::CRCSum, _data.crc32);
    OSCR::Serial::printValue(OSCR::Strings::Common::ID, _data.id32);
    OSCR::Serial::printValue(OSCR::Strings::Common::Mapper, _data.mapper);
    OSCR::Serial::printValue(OSCR::Strings::Common::Size, _data.size);
    OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
# endif /* CRDB_DEBUGGING */
  }

  void CRDatabase::nextRecord()
  {
    clearError();
    readNum32(currentRecord->data()->crc32);
    readNum32(currentRecord->data()->id32);
    readNum32(&currentRecord->data()->mapper);
    readNum32(&currentRecord->data()->size);
    readBytes(&currentRecord->data()->name, 100);
  }

  void setup(char const * databasePath)
  {
    if (ready) delete crdb;
    crdb = new CRDatabase(databasePath);
    ready = true;
  }

  void setup(__FlashStringHelper const * databasePath)
  {
    if (ready) delete crdb;
    crdb = new CRDatabase(databasePath);
    ready = true;
  }

  bool searchDatabase(crc32_t * crc32ptr)
  {
    OSCR::UI::print(F("Find "));
    OSCR::UI::print(*crc32ptr);
    OSCR::UI::printLineSync(FS(OSCR::Strings::Symbol::Ellipsis));

    if (!crdb->findRecord(*crc32ptr))
    {
      OSCR::UI::printLine();
      OSCR::UI::printLineSync(F("CRC not found in database"));

      return false;
    }

    ExtendedRecord * record = crdb->record();

    if (crdb->hasError())
    {
      return false;
    }

    romRecord = record;
    romDetail = record->data();

    return true;
  }

  bool browseDatabase()
  {
    OSCR::crdbBrowser(FS(OSCR::Strings::Headings::SelectCRDBEntry), crdb);

    romRecord = crdb->record();
    romDetail = crdb->record()->data();

    return true;
  }

  bool matchCRC(crc32_t * crc32ptr, uint8_t offset)
  {
    return crdb->matchCRC(crc32ptr, offset);
  }

  bool compareCRC(char const * databasePath, crc32_t * crc32ptr, uint8_t offset)
  {
    setup(databasePath);
    return matchCRC(crc32ptr, offset);
  }

  bool compareCRC(char const * databasePath, crc32_t & crc32, uint8_t offset)
  {
    setup(databasePath);
    return matchCRC(&crc32, offset);
  }

  bool compareCRC(__FlashStringHelper const * databaseName, crc32_t * crc32ptr, uint8_t offset)
  {
    setup(databaseName);
    return matchCRC(crc32ptr, offset);
  }

  bool compareCRC(__FlashStringHelper const * databaseName, crc32_t & crc32, uint8_t offset)
  {
    setup(databaseName);
    return matchCRC(&crc32, offset);
  }
}


#pragma once
#ifndef CRDB_GBA_H_
# define CRDB_GBA_H_

# include "config.h"

# if HAS_GBX
#   include "common/CRDB.h"
#   include "cores/GameBoyAdvance.h"

namespace OSCR::Databases
{
  using OSCR::Cores::GameBoyAdvance::crdbGBARecord;

  class GBARecord
    : public OSCR::CRDB::GenericCartRecord<crdbGBARecord>
  {
  public:
    void debug()
    {
#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugROM));

      OSCR::Serial::printSync(FS(OSCR::Strings::Labels::NAME));
      OSCR::Serial::printLineSync(_data.name);

      OSCR::Serial::printSync(FS(OSCR::Strings::Labels::CRCSum));
      OSCR::Serial::printLineSync(_data.crc32);

      OSCR::Serial::printSync(FS(OSCR::Strings::Labels::ID));
      OSCR::Serial::printLineSync(_data.serial);

      OSCR::Serial::printSync(FS(OSCR::Strings::Labels::SIZE));
      OSCR::Serial::printLineSync(_data.size);

      OSCR::Serial::printSync(FS(OSCR::Strings::Labels::SAVE));
      OSCR::Serial::printLineSync(_data.saveType);

      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
#   endif /* ENABLE_CRDB_DEBUG */
    }
  };

  class GBA : public OSCR::CRDB::CRDB<GBARecord, CRDB_RECORD_SIZE_GBA>
  {
    using CRDB::CRDB;
  public:
    void nextRecord()
    {
      clearError();
      readNum32(currentRecord->data()->crc32);
      readBytes(&currentRecord->data()->serial, 4);
      readNum16(&currentRecord->data()->size);
      readNum16(&currentRecord->data()->saveType);
      readBytes(&currentRecord->data()->name, 100);

#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLineSync(currentRecord->data()->name);
#   endif /* ENABLE_CRDB_DEBUG */
    }

    bool searchRecord(char const * serialSearch, uint16_t startingRecord = 0)
    {
      constexpr uint8_t const offset = 4;
      clearError();

#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLineSync(F("<CRDB> Searching..."));
      OSCR::Serial::printSync(F("<CRDB> Serial Search: "));
      OSCR::Serial::printLineSync(serialSearch);
#   endif /* ENABLE_CRDB_DEBUG */

      for (uint_fast32_t i = startingRecord; gotoRecordIndex(i) && file.seekCur(offset); ++i)
      {
        char serial[5];
        int_fast8_t const readLen = readBytes(serial, 4);

        if (readLen != 4) break;

        if (
          (serial[0] == serialSearch[0]) &&
          (serial[1] == serialSearch[1]) &&
          (serial[2] == serialSearch[2]) &&
          (serial[3] == serialSearch[3])
        )
        {
#   if defined(ENABLE_CRDB_DEBUG)
          OSCR::Serial::printSync(F("<CRDB> "));
          OSCR::Serial::printLineSync(FS(OSCR::Strings::Common::OK));
#   endif /* ENABLE_CRDB_DEBUG */

          if (!loadRecordIndex(i))
          {
#   if defined(ENABLE_CRDB_DEBUG)
            OSCR::Serial::printLineSync(F("<CRDB> Failed to load record!"));
#   endif /* ENABLE_CRDB_DEBUG */
            error(OSCR::CRDB::ERRORS::E_READ_ERROR);
            return false;
          }

#   if defined(ENABLE_CRDB_DEBUG)
          OSCR::Serial::printLineSync(F("<CRDB> Loaded record."));
#   endif /* ENABLE_CRDB_DEBUG */

          return true;
        }
      }

#   if defined(ENABLE_CRDB_DEBUG)
      OSCR::Serial::printLineSync(F("<CRDB> NOT FOUND"));
#   endif /* ENABLE_CRDB_DEBUG */

      gotoRecordIndex(0);
      return false;
    }

    bool searchRecordNext(char const * serialSearch)
    {
      return searchRecord(serialSearch, recordNum);
    }
  };
}

# endif /* HAS_GBX */

#endif /* CRDB_GBA_H_ */


#pragma once
#ifndef CRDB_NES_H_
# define CRDB_NES_H_

# include "common/CRDB.h"
# include "cores/NES.h"

namespace OSCR::Databases
{
  using OSCR::Cores::NES::crdbNESRecord;
  using OSCR::Cores::NES::crdbNESMapperRecord;

  class NESMapperRecord
    : public OSCR::CRDB::GenericRecord<crdbNESMapperRecord>
  {
      void debug()
      {
#   if CRDB_DEBUGGING
        OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugMapper));
        OSCR::Serial::printValue(OSCR::Strings::Common::Name, _data.name);
        OSCR::Serial::printValue(OSCR::Strings::Common::Mapper, _data.mapper);
        OSCR::Serial::printValue(OSCR::Strings::Common::Submapper, _data.submapper);
        OSCR::Serial::printValue(OSCR::Strings::Common::PRG, _data.prglo, _data.prghi);
        OSCR::Serial::printValue(OSCR::Strings::Common::CHR, _data.chrlo, _data.chrhi);
        OSCR::Serial::printValue(OSCR::Strings::Common::Lower, _data.ramlo, _data.ramhi);
        OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
#   endif /* CRDB_DEBUGGING */
      }
  };

  class NESMapper
    : public OSCR::CRDB::CRDB<NESMapperRecord, CRDB_RECORD_SIZE_NES_MAPPER, OSCR::CRDB::CRDBType::NamedByMapperSub>
  {
    using CRDB::CRDB;
    public:
      NESMapper(char const * crdb_filename)
       : CRDB(crdb_filename)
      {
        // ...
      }

      void nextRecord()
      {
        clearError();
        readNum32(&currentRecord->data()->mapper);
        readNum32(&currentRecord->data()->submapper);
        readNum32(&currentRecord->data()->prglo);
        readNum32(&currentRecord->data()->prghi);
        readNum32(&currentRecord->data()->chrlo);
        readNum32(&currentRecord->data()->chrhi);
        readNum32(&currentRecord->data()->ramlo);
        readNum32(&currentRecord->data()->ramhi);

        snprintf_P(BUFFN(currentRecord->data()->name), PSTR("%03" PRIu32 ".%02" PRIu32), currentRecord->data()->mapper, currentRecord->data()->submapper);
      }
  };

  class NESRecord
    : public OSCR::CRDB::GenericCartRecord<crdbNESRecord>
  {
  public:
    void debug()
    {
#   if CRDB_DEBUGGING
      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugROM));

      OSCR::Serial::printValue(OSCR::Strings::Common::Name, _data.name);
      OSCR::Serial::printValue(OSCR::Strings::Common::Mapper, _data.mapper);
      OSCR::Serial::printValue(OSCR::Strings::Common::Submapper, _data.submapper);
      //OSCR::Serial::printSize(OSCR::Strings::Common::PRG, _data.prgsize);
      OSCR::Serial::printValue(OSCR::Strings::Common::PRG, _data.prg);
      //OSCR::Serial::printSize(OSCR::Strings::Common::CHR, _data.chrsize);
      OSCR::Serial::printValue(OSCR::Strings::Common::CHR, _data.chr);
      //OSCR::Serial::printSize(OSCR::Strings::Common::RAM, _data.ramsize);
      OSCR::Serial::printValue(OSCR::Strings::Common::RAM, _data.ram);

      OSCR::Serial::print(F("iNES: "));
      char buffer[9];
      snprintf_P(buffer, 9, PSTR("%02X%02X%02X%02X"), _data.iNES[0], _data.iNES[1], _data.iNES[2], _data.iNES[3]);
      OSCR::Serial::printLine(buffer);

      OSCR::Serial::printLineSync(FS(OSCR::Strings::Headings::CRDBDebugEnd));
#   endif /* CRDB_DEBUGGING */
    }
  };

  class NES : public OSCR::CRDB::CRDB<NESRecord, CRDB_RECORD_SIZE_NES>
  {
    using CRDB::CRDB;
  public:
    void nextRecord()
    {
      clearError();
      readNum32(currentRecord->data()->crc32);
      readNum32(currentRecord->data()->id32a);
      readNum32(currentRecord->data()->id32b);
      readNum16(&currentRecord->data()->mapper);
      readNum16(&currentRecord->data()->submapper);
      readNum16(&currentRecord->data()->prg);
      readNum16(&currentRecord->data()->chr);
      readNum16(&currentRecord->data()->ramsize);
      readBytes(&currentRecord->data()->iNES, 16);
      readBytes(&currentRecord->data()->name, 100);
    }
  };
}

#endif /* CRDB_NES_H_ */

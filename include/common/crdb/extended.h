/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef CRDB_EXTENDED_H_
# define CRDB_EXTENDED_H_

#include "common/CRDB.h"

namespace OSCR
{
  namespace Databases
  {
    namespace Extended
    {
      struct crdbRecord
      {
        crc32_t crc32;
        crc32_t id32;
        uint32_t mapper;
        uint32_t size;
        uint32_t save;
        char name[101];
      };

      class ExtendedRecord : public OSCR::CRDB::GenericCartRecord<crdbRecord>
      {
      public:
        uint32_t getCRC32();
        void debug();
      };

      class CRDatabase : public OSCR::CRDB::CRDB<ExtendedRecord, CRDB_RECORD_SIZE_EXTENDED>
      {
      using CRDB::CRDB;
      public:
        void nextRecord();

        virtual ~CRDatabase() = default;
      };

      extern crdbRecord * romDetail;
      extern ExtendedRecord * romRecord;
      extern CRDatabase * crdb;
      extern bool ready;

      extern void setup(char const * databasePath);
      extern void setup(__FlashStringHelper const * databasePath);

      extern bool matchCRC(crc32_t * crc32ptr = nullptr, uint8_t offset = 0);
      extern bool searchDatabase(crc32_t * crc32ptr = nullptr);

      extern bool browseDatabase();

      extern bool compareCRC(char const * databasePath, crc32_t * crc32ptr = nullptr, uint8_t offset = 0);
      extern bool compareCRC(__FlashStringHelper const * databaseName, crc32_t * crc32ptr = nullptr, uint8_t offset = 0);

      extern bool compareCRC(char const * databasePath, crc32_t & crc32, uint8_t offset = 0);
      extern bool compareCRC(__FlashStringHelper const * databaseName, crc32_t & crc32, uint8_t offset = 0);
    }
  }
}

#endif /* CRDB_EXTENDED_ID_H_ */

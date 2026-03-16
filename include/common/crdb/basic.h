/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef CRDB_BASIC_H_
#define CRDB_BASIC_H_

#include "common/CRDB.h"

namespace OSCR
{
  namespace Databases
  {
    namespace Basic
    {
      struct crdbRecord
      {
        crc32_t crc32;
        char name[101];
      };

      class BasicRecord : public OSCR::CRDB::GenericCartRecord<crdbRecord>
      {
      public:
        uint32_t getCRC32();
        void debug();
      };

      class CRDatabase : public OSCR::CRDB::CRDB<BasicRecord, CRDB_RECORD_SIZE_BASIC>
      {
      using CRDB::CRDB;
      public:
        void nextRecord();

        virtual ~CRDatabase() = default;
      };

      extern crdbRecord * romDetail;
      extern BasicRecord * romRecord;
      extern CRDatabase * crdb;
      extern bool ready;

      extern void setup(char const * databasePath);
      extern void setup(__FlashStringHelper const * databasePath);

      extern bool matchCRC(crc32_t * crc32ptr = nullptr, uint8_t offset = 0);
      extern bool searchDatabase(crc32_t * crc32ptr = nullptr);

      extern bool compareCRC(char const * databasePath, crc32_t * crc32ptr = nullptr, uint8_t offset = 0);
      extern bool compareCRC(__FlashStringHelper const * databaseName, crc32_t * crc32ptr = nullptr, uint8_t offset = 0);

      extern bool compareCRC(char const * databasePath, crc32_t & crc32, uint8_t offset = 0);
      extern bool compareCRC(__FlashStringHelper const * databaseName, crc32_t & crc32, uint8_t offset = 0);

    }
  }
}

#endif /* CRDB_BASIC_H_ */

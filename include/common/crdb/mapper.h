/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef CRDB_BASIC_MAPPER_H_
# define CRDB_BASIC_MAPPER_H_

#include "common/CRDB.h"

namespace OSCR
{
  namespace Databases
  {
    namespace Basic
    {
      struct crdbMapperRecord
      {
        uint16_t id;
        uint32_t sizeLow;
        uint32_t sizeHigh;
        uint32_t meta1;
        uint32_t meta2;
        char name[101];
      };

      class MapperRecord : public OSCR::CRDB::GenericRecord<crdbMapperRecord>
      {
      public:
        char const * getName() const;
        bool readName(char * buffer, size_t size) const;
        void debug() const;
      };

      class CRMapperDatabase : public OSCR::CRDB::CRDB<MapperRecord, CRDB_RECORD_SIZE_BASIC_MAPPER, OSCR::CRDB::CRDBType::NamedByID>
      {
      using CRDB::CRDB;
      public:
        void nextRecord();

        virtual ~CRMapperDatabase() = default;
      };

      extern crdbMapperRecord * mapperDetail;
      extern MapperRecord * mapperRecord;
      extern bool mapperReady;
      extern bool fromCRDB;

      extern void resetMapper();
      extern void setupMapper(char const * databasePath);
      extern void setupMapper(__FlashStringHelper const * databaseName);
      extern CRMapperDatabase * getMapperCRDB();

      extern bool matchMapper(uint16_t searchId, uint8_t offset = 0);

      extern bool matchMapper(char const * databasePath, uint16_t searchId, uint8_t offset = 0);
      extern bool matchMapper(__FlashStringHelper const * databaseName, uint16_t searchId, uint8_t offset = 0);

      extern bool searchMapper(char const * databasePath, uint16_t searchId);
      extern bool searchMapper(__FlashStringHelper const * databaseName, uint16_t searchId);

      extern void browseMappers();
      extern void browseMappers(char const * databasePath);
      extern void browseMappers(__FlashStringHelper const * databaseName);

      extern void printMapperNotFound(uint16_t mapperId);
    }
  }
}

#endif /* CRDB_BASIC_MAPPER_H_ */

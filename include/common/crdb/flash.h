/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef CRDB_FLASH_H_
# define CRDB_FLASH_H_

#include "common/CRDB.h"
#include "cores/Flash.h"

namespace OSCR
{
  namespace Databases
  {
    namespace Flash
    {
      using OSCR::Cores::Flash::FlashType;

      struct crdbFlashRecord
      {
        uint32_t id;
        uint32_t variant;
        uint32_t bits;
        uint32_t size;
        uint32_t sector;
        uint32_t buffer;
        uint8_t voltage;
        FlashType type;
        char typeName[50];
        char name[50];
      };

      class FlashRecord : public OSCR::CRDB::GenericRecord<crdbFlashRecord>
      {
      public:
        char const * getName() const;
        bool readName(char * buffer, size_t size) const;
        void debug() const;
      };

      class Flash : public OSCR::CRDB::CRDB<FlashRecord, CRDB_RECORD_SIZE_FLASH, OSCR::CRDB::CRDBType::NamedByID>
      {
      using CRDB::CRDB;
      public:
        void nextRecord();

        virtual ~Flash() = default;
      };

      extern crdbFlashRecord * flashDetail;
      extern FlashRecord * flashRecord;
      extern bool flashReady;
      extern bool fromCRDB;

      extern void resetFlash();
      extern void setupFlash(char const * databasePath);
      extern void setupFlash(__FlashStringHelper const * databaseName);
      extern Flash * getFlashCRDB();

      extern bool matchFlash(uint16_t searchId, uint8_t offset = 0);

      extern void browseFlash();

      extern void printFlashNotFound(uint16_t flashId);
    }
  }
}

#endif /* CRDB_FLASH_H_ */

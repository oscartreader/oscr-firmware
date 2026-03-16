/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef CRDB_H_
# define CRDB_H_

# include "config.h"
# include "syslibinc.h"
# include "hardware/outputs/Serial.h"
# include "ui.h"
# include "common/specializations.h"
# include "common/crc32.h"

namespace OSCR
{
  namespace CRDB
  {
    constexpr uint8_t   const         kDefaultDatabasePathLength = 32;
    extern    uint32_t  const PROGMEM kDatabaseFileHeader;
    extern    uint16_t  const PROGMEM kVersion;
    extern    uint32_t  const PROGMEM kDatestampMinimum;
    extern    char      const PROGMEM PATH_TEMPLATE[];
    extern    char      const PROGMEM PATH_TEMPLATE_FS[];

    enum class CRDBType : uint8_t
    {
      NamedByCRC32, // Search by CRC32, has Name
      ByID, // Search by ID (uint16_t), unnamed
      NamedByID, // Search by ID (uint16_t), has Name
    };

    using OSCR::Storage::sd;
    using OSCR::Storage::Shared::sharedFile;
    using OSCR::Storage::Shared::sharedFileName;

    enum ERRORS: uint8_t
    {
      E_NONE = 0,
      E_REACHED_EOF,
      E_DATABASE_ERROR,
      E_DATABASE_NOT_FOUND,
      E_RECORD_NOT_FOUND,
      E_READ_ERROR,
    };

    extern void dbName(char const * databaseName, char databasePath[], uint8_t maxLength = kDefaultDatabasePathLength);
    extern void dbName_P(__FlashStringHelper const * databaseName, char databasePath[], uint8_t maxLength = kDefaultDatabasePathLength);

    /*C******************************************************************
    * NAME :            GenericCartRecord / GenericRecord
    *
    * DESCRIPTION :
    *           Extend this generic CRDB record template to make a
    *           database record class.
    *
    * USAGE :           ...
    *
    * NOTES :           ...
    *C*/
    template<typename RecordType>
    class GenericCartRecord
    {
    public:
      uint32_t getCRC32() const
      {
        return _data.crc32;
      }

      RecordType * data()
      {
        return &_data;
      }

    protected:
      RecordType _data = RecordType();
    };

    template<typename RecordType>
    class GenericRecord
    {
    public:
      RecordType * data()
      {
        return &_data;
      }

    protected:
      RecordType _data = RecordType();
    };

    class CRDBBase
    {
    public:
      CRDBBase(char const * crdb_filename, uint_fast32_t crdb_recordsize);
      CRDBBase(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize);

      virtual ~CRDBBase();
      virtual void nextRecord() = 0;

      void error(ERRORS code);
      ERRORS getError() const;
      bool hasError() const;
      void clearError();

      void databaseError();

      int16_t readBytes(void * dest, uint8_t bytes);
      uint8_t readNum8(uint8_t * dest);
      uint8_t readNum16(uint16_t * dest);
      uint8_t readNum32(uint32_t * dest);
      uint8_t readNum32(crc32_t & dest);

      uint32_t numRecords() const;

      bool loadRecordIndex(uint16_t index);
      bool gotoRecordIndex(uint16_t index);

#if defined(NEEDS_CRDB_ISQUIET)
      bool isQuiet();
      void quiet(bool shouldBeQuiet = true);
#endif

    protected:
      FsFile file;
      uint_fast16_t const recordSize;
      uint_fast16_t recordCount;
      uint_fast16_t recordNum = 0;
#if defined(NEEDS_CRDB_ISQUIET)
      bool beQuiet = false;
#endif
      ERRORS errorCode = E_NONE;

      void openDatabase(char const * crdb_filename);
    };

    class CRDBByIDBase : public CRDBBase
    {
    public:
      CRDBByIDBase(char const * crdb_filename, uint_fast32_t crdb_recordsize)
        : CRDBBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      CRDBByIDBase(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      bool findRecord(uint16_t const idSearch, uint16_t const offset = 0);
    };

    template <typename RecordType>
    class CRDBByID : public CRDBByIDBase
    {
    public:
      CRDBByID(char const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBByIDBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      CRDBByID(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBByIDBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      inline
      RecordType * record()
      {
        return currentRecord;
      }

    protected:
      RecordType * currentRecord = new RecordType();
    };

    template <typename RecordType>
    class CRDBNamedByID : public CRDBByIDBase
    {
    public:
      CRDBNamedByID(char const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBByIDBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      CRDBNamedByID(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBByIDBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      inline
      RecordType * record()
      {
        return currentRecord;
      }

      inline
      char * getRecordName() const
      {
        return currentRecord->data()->name;
      }

    protected:
      RecordType * currentRecord = new RecordType();
    };

    class CRDBNamedByCRC32Base : public CRDBBase
    {
    public:
      CRDBNamedByCRC32Base(char const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      CRDBNamedByCRC32Base(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBBase(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      bool findRecord(uint32_t const crc32, uint16_t const offset = 0);
      bool findRecord(crc32_t const & crc32search, uint16_t const offset = 0);
      bool findEitherRecord(crc32_t crc32search1, crc32_t crc32search2, uint16_t offset = 0);

      bool matchCRC(uint32_t crc32, int dbOffset = 0);
      bool matchCRC(crc32_t & crc32, int dbOffset = 0);
      bool matchCRC(crc32_t * crc32ptr = nullptr, int dbOffset = 0);

      // CRC match events
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_POSTMATCHCRC)
      virtual bool postMatchCRC() = 0;
#endif
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHFAIL)
      virtual bool crcMatchFail() = 0;
#endif
#if defined(NEEDS_CRDB_EVENTS) || defined(NEEDS_CRDB_EVENT_CRCMATCHSUCCESS)
      virtual bool crcMatchSuccess() = 0;
#endif

      virtual char * getRecordName() const = 0;

    protected:
      virtual bool cmprCRC32(crc32_t * crc32ptr = nullptr) const = 0;
      // ...
    };

    template <typename RecordType>
    class CRDBNamedByCRC32 : public CRDBNamedByCRC32Base
    {
    public:
      CRDBNamedByCRC32(char const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBNamedByCRC32Base(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      CRDBNamedByCRC32(__FlashStringHelper const * crdb_filename, uint_fast32_t crdb_recordsize)
       : CRDBNamedByCRC32Base(crdb_filename, crdb_recordsize)
      {
        // ...
      }

      inline
      RecordType * record()
      {
        return currentRecord;
      }

      inline
      char * getRecordName() const override
      {
        return currentRecord->data()->name;
      }

    protected:
      RecordType * currentRecord = new RecordType();

      inline
      bool cmprCRC32(crc32_t * crc32ptr = nullptr) const override
      {
        return (*crc32ptr == currentRecord->getCRC32());
      }
    };

    template <typename RecordType, CRDBType crdbType> struct get_crdb_class { typedef void type; };
    template <typename RecordType> struct get_crdb_class <RecordType, CRDBType::NamedByCRC32> { typedef CRDBNamedByCRC32<RecordType> type; };
    template <typename RecordType> struct get_crdb_class <RecordType, CRDBType::NamedByID> { typedef CRDBNamedByID<RecordType> type; };
    template <typename RecordType> struct get_crdb_class <RecordType, CRDBType::ByID> { typedef CRDBByID<RecordType> type; };

    /*C******************************************************************
    * NAME :            CRDB
    *
    * DESCRIPTION :
    *           Extend this generic CRDB template with the record class
    *           as the type parameter to create a new DB type.
    *
    * USAGE :           ...
    *
    * NOTES :           ...
    *C*/
    template <typename RecordType,
              uint_fast32_t dbRecordSize,
              CRDBType crdbType = CRDBType::NamedByCRC32,
              typename CRDBParent = typename get_crdb_class<RecordType, crdbType>::type>
    class CRDB : public CRDBParent
    {
    public:
      CRDB(char const * crdb_filename)
       : CRDBParent(crdb_filename, dbRecordSize)
      {
        // ...
      }

      CRDB(__FlashStringHelper const * crdb_filename)
       : CRDBParent(crdb_filename, dbRecordSize)
      {
        // ...
      }

    protected:
      // ...
    };
  }
}

#endif /* CRDB_H_ */

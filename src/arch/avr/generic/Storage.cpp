#include "arch/avr/syslibinc.h"

#if defined(OSCR_ARCH_AVR)
# include "macros.h"
# include "ui.h"
# include "api/Time.h"
# include "api/Storage.h"
# include "apps/Logger.h"
# include "hardware/outputs/Serial.h"

namespace
{
# if defined(__FILE_NAME__)
  constexpr char const PROGMEM ThisFilename[] = __FILE_NAME__;
# else
  constexpr char const PROGMEM ThisFilename[] = "Storage.cpp";
# endif
}

# if OPTION_UNIQUE_DIRECTORY_PADDING > 0
#   define INCREMENT_TEMPLATE "0" NUM2STR(OPTION_UNIQUE_DIRECTORY_PADDING) PRIu16
# else
#   define INCREMENT_TEMPLATE PRIu16
# endif

# if (OPTION_UNIQUE_DIRECTORY_FULLYEAR)
#   define YEAR_TEMPLATE "04" PRIu16
# else
#   define YEAR_TEMPLATE "02" PRIu8
# endif

namespace OSCR
{
  namespace Storage
  {
    SdFs sd;

    constexpr char const PROGMEM AllowedSymbols[] = "-_ .()";
    constexpr uint8_t const kAllowedSymbolCount = sizeof(AllowedSymbols) - 1;

    constexpr char const PROGMEM FilenameTemplate[] = "%s.%s";
    constexpr char const PROGMEM FilenameTemplateP[] = "%s.%S";
    constexpr char const PROGMEM FilenameTemplatePP[] = "%S.%S";

    bool isReservedChr(uint8_t c)
    {
      return lfnReservedChar(c);
    }

    void updatedFolderIncrement()
    {
# if (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_INCREMENT)
      snprintf_P(BUFFN(OSCR::Storage::Shared::folderIncrementStr), PSTR("%" INCREMENT_TEMPLATE), Shared::folderIncrement);
# elif (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_RTC)
      DateTime now = OSCR::Time::now();

#   if (OPTION_UNIQUE_DIRECTORY_FULLYEAR)
      uint16_t const year = now.year();
#   else
      uint8_t const year = now.year() - 2000;
#   endif /* OPTION_UNIQUE_DIRECTORY_FULLYEAR */

      snprintf_P(
        BUFFN(OSCR::Storage::Shared::folderIncrementStr),
        PSTR("%" YEAR_TEMPLATE "%02" PRIu8 "%02" PRIu8 "%02" PRIu8 "%02" PRIu8), year, now.month(), now.day(), now.hour(), now.minute());
# elif (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_BOTH)
      DateTime now = OSCR::Time::now();

#   if (OPTION_UNIQUE_DIRECTORY_FULLYEAR)
      uint16_t const year = now.year();
#   else
      uint8_t const year = now.year() - 2000;
#   endif /* OPTION_UNIQUE_DIRECTORY_FULLYEAR */

      snprintf_P(BUFFN(OSCR::Storage::Shared::folderIncrementStr), PSTR("%" YEAR_TEMPLATE "%02" PRIu8 "%02" PRIu8 "%02" PRIu8 "%02" PRIu8 "%" INCREMENT_TEMPLATE), year, now.month(), now.day(), now.hour(), now.minute(), Shared::folderIncrement);
# else
#   error Invalid OPTION_UNIQUE_DIRECTORY_METHOD
# endif
    }

# define SPI_CLOCK SD_SCK_MHZ(8)

# if (HARDWARE_OUTPUT_TYPE == OUTPUT_SERIAL) && defined(ENABLE_DEDICATED_SPI) && (ENABLE_DEDICATED_SPI)
#   define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI)
# else
#   define SD_CONFIG SdSpiConfig(SS, SHARED_SPI)
# endif

    void setup()
    {
# if defined(ENABLE_RTC)
      SdFile::dateTimeCallback(dateTimeCB);
# endif

      // Init SD card
      if (!sd.begin(SD_CONFIG))
      {
        OSCR::UI::fatalErrorStorage();
      }

# if ((OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_INCREMENT) || (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_BOTH))
      // Get folder increment
      EEPROM_readAnything(0, Shared::folderIncrement);
      if (Shared::folderIncrement < 0) Shared::folderIncrement = 0;
# endif

      updatedFolderIncrement();
    }

    bool __allowedSymbol(uint8_t src)
    {
      return (strchr_P(AllowedSymbols, src) != NULL);
    }

    bool __allowedCharacter(uint8_t src)
    {
      return ((__allowedSymbol(src)) || OSCR::Util::isAlphaNumeric(src)); // ((src >= 'a') && (src <= 'z')) || ((src >= 'A') && (src <= 'Z')) || ((src >= '0') && (src <= '9')));
    }

    uint8_t copyFileName(char const * const src, uint8_t const srcMaxLen, char * const dest, uint8_t const destMaxLen)
    {
      uint8_t const srcLen = strnlen(src, srcMaxLen);
      uint8_t destLen = 0;

      OSCR::Util::setNulls(dest, destMaxLen);

      // Scan the string and copy characters as needed
      for (uint8_t i = 0; (i < srcLen) && (destLen < destMaxLen); i++)
      {
        if (__allowedCharacter(src[i]))
        {
          dest[destLen++] = src[i];
        }
      }

      // Trim symbols from the end of the file name (and leave room for at least 1 NUL)
      while (((destLen > 0) && __allowedSymbol(dest[destLen - 1])) || ((destLen + 1) == destMaxLen))
      {
        dest[--destLen] = '\0';
      }

      return destLen;
    }

    uint8_t copyFileName_P(char const * const src, char * const dest, uint8_t const destMaxLen)
    {
      uint8_t const srcLen = strlen_P(src);
      uint8_t destLen = 0;

      OSCR::Util::setNulls(dest, destMaxLen);

      // Scan the string and copy characters as needed
      for (uint8_t i = 0; (i < srcLen) && (destLen < destMaxLen); i++)
      {
        uint8_t chr = pgm_read_byte(src + i);

        if (__allowedCharacter(chr))
        {
          dest[destLen++] = chr;
        }
      }

      // Trim symbols from the end of the file name (and leave room for at least 1 NUL)
      while (((destLen > 0) && __allowedSymbol(dest[destLen - 1])) || ((destLen + 1) == destMaxLen))
      {
        dest[destLen--] = '\0';
      }

      return destLen;
    }

# if defined(ENABLE_RTC)
    // Callback for file timestamps
    void dateTimeCB(uint16_t * date, uint16_t * time)
    {
      DateTime now = OSCR::Time::now();

      // Return date using FAT_DATE macro to format fields
      *date = FAT_DATE(now.year(), now.month(), now.day());

      // Return time using FAT_TIME macro to format fields
      *time = FAT_TIME(now.hour(), now.minute(), now.second());
    }
# endif

    bool exists(char const * path)
    {
      return OSCR::Storage::sd.exists(path);
    }

    bool exists(__StringHelper const * path)
    {
      char tmpPath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpPath), path);
      return OSCR::Storage::sd.exists(tmpPath);
    }

    bool isDir(char const * path)
    {
      FsFile file;

      if (!file.open(path)) return false;

      bool pathIsDir = file.isDir();

      file.close();

      return pathIsDir;
    }

    bool isDir(__StringHelper const * path)
    {
      char tmpPath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpPath), path);
      return isDir(tmpPath);
    }

    bool isFile(char const * path)
    {
      FsFile file;

      if (!file.open(path)) return false;

      bool pathIsFile = file.isFile();

      file.close();

      return pathIsFile;
    }

    bool isFile(__StringHelper const * path)
    {
      char tmpPath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpPath), path);
      return isFile(tmpPath);
    }

    bool createFolder(char const * path, bool recursive)
    {
      return sd.mkdir(path, recursive);
    }

    bool createFolder(__StringHelper const * path, bool recursive)
    {
      char tmpPath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpPath), path);
      return sd.mkdir(tmpPath, recursive);
    }

    File::File()
    {
      // ...
    }

    File::File(char const * fileName, oflag_t oflag)
    {
      open(fileName, oflag);
    }

    File::File(__StringHelper const * fileName, oflag_t oflag)
    {
      open(fileName, oflag);
    }

    void File::assignBuffer(uint8_t * fileBuffer, uint16_t fileBufferSize)
    {
      buffer = fileBuffer;
      bufferSize = fileBufferSize;
    }

    void File::unassignBuffer()
    {
      buffer = nullptr;
      bufferSize = 0;
    }

    void File::crcReset()
    {
      fCRC32.reset();
      crcState = CRC32State::Summing;
    }

    void File::sumCRC32()
    {
      uint8_t buffer[512];
      uint16_t length;
      uint64_t startPos = curPosition();

      rewind();

      fCRC32.reset();

      while ((length = read(BUFFN(buffer))) != 0)
      {
        for (uint16_t i = 0; i < length; ++i)
        {
          fCRC32 += buffer[i];
        }
      }

      fCRC32.done();

      crcState = CRC32State::Summed;

      seekSet(startPos);
    }

    OSCR::CRC32::crc32_t File::getCRC32()
    {
      switch (crcState)
      {
      case CRC32State::None:
        sumCRC32();
        return fCRC32;

      case CRC32State::Summing:
        return OSCR::CRC32::crc32_t(~(fCRC32.value));

      case CRC32State::Summed:
        return fCRC32;

      default:
        OSCR::Util::unreachable();
      }
    }

    void File::postOpen()
    {
      opened = true;

      if (fileOflag & O_CREAT)
      {
        crcReset();
      }
    }

    void File::postClose()
    {
      opened = false;

      if (crcState == CRC32State::Summing)
      {
        fCRC32.done();
        crcState = CRC32State::Summed;
      }
    }

    bool File::open(char const * fileName, oflag_t oflag)
    {
      if (opened) close();

      if (file.open(fileName, oflag))
      {
        fileOflag = oflag;
        postOpen();
        return true;
      }

      if (!fatalErrors)
      {
        opened = false;
        return false;
      }

      OSCR::UI::fatalErrorStorage();
    }

    bool File::open(__StringHelper const * fileName, oflag_t oflag)
    {
      char tmpFilePath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpFilePath), fileName);
      return open(tmpFilePath, oflag);
    }

    bool File::open(uint32_t index, oflag_t oflag)
    {
      if (opened) close();

      if (file.open(index, oflag))
      {
        fileOflag = oflag;
        postOpen();
        return true;
      }

      if (!fatalErrors)
      {
        opened = false;
        return false;
      }

      OSCR::UI::fatalErrorStorage();
    }

    bool File::isOpen()
    {
      return opened;
    }

    bool File::close()
    {
      if (!opened) return true;

      if (file.close())
      {
        postClose();
        return true;
      }

      if (!fatalErrors) return false;

      OSCR::UI::fatalErrorStorage();
    }

    void File::sync()
    {
      if (!opened) return;

      file.sync();
    }

    bool File::rename(char const * newPath)
    {
      if (file.rename(newPath)) return true;

      if (!fatalErrors) return false;

      OSCR::UI::fatalErrorStorage();
    }

    bool File::rename(__StringHelper const * newPath)
    {
      char tmpFilePath[kFilePathnameLength];
      OSCR::Util::copyStr_P(BUFFN(tmpFilePath), newPath);
      return rename(tmpFilePath);
    }

    bool File::hasBuffer() const
    {
      return ((bufferSize > 0) && (buffer != nullptr));
    }

    void File::bufferCheck(size_t count)
    {
      if (hasBuffer() && count <= bufferSize) return;

      // Close the file first to avoid corruption
      file.close();

      if (!hasBuffer())
      {
        OSCR::UI::fatalErrorNoBuffer();
      }

      OSCR::UI::fatalErrorBufferOverflow();
    }

    uint8_t File::peek()
    {
      return file.peek();
    }

    __optimize_file_write
    bool File::writeRaw(uint8_t byte)
    {
      if (crcState == CRC32State::Summing)
      {
        fCRC32 += byte;
      }

      return file.write(byte);
    }

    __optimize_file_write
    size_t File::writeRaw(uint8_t const * buf, size_t count)
    {
      if (crcState == CRC32State::Summing)
      {
        for (size_t i = 0; i < count; i++)
        {
          fCRC32 += buf[i];
        }
      }

      return file.write(buf, count);
    }

    __optimize_file_write
    size_t File::writeRaw(size_t count)
    {
      bufferCheck(count);

      return writeRaw(buffer, count);
    }

    __optimize_file_write
    size_t File::write(uint8_t const * buf, size_t count)
    {
      size_t written = writeRaw(buf, count);

      if (file.getWriteError())
      {
        OSCR::UI::fatalErrorStorage();
      }

      return written;
    }

    __optimize_file_write
    size_t File::write(char const * buf, size_t count)
    {
      return write((uint8_t *)buf, count);
    }

    __optimize_file_write
    bool File::write(uint8_t byte)
    {
      bool written = writeRaw(byte);

      if (file.getWriteError())
      {
        OSCR::UI::fatalErrorStorage();
      }

      return written;
    }

    __optimize_file_write
    size_t File::write(char const * buf)
    {
      size_t length = strlen(buf);

      return write(buf, length);
    }

    __optimize_file_write
    size_t File::writeNewLine()
    {
      return write("\n", 1);
    }

    __optimize_file_write
    size_t File::write(size_t count)
    {
      bufferCheck(count);

      return write(buffer, count);
    }

    __optimize_file_write
    size_t File::write()
    {
      bufferCheck(bufferSize);

      return write(buffer, bufferSize);
    }

    __optimize_file_read
    size_t File::readRaw(uint8_t * buf, size_t count)
    {
      return file.read(buf, count);
    }

    __optimize_file_read
    size_t File::readRaw(size_t count)
    {
      bufferCheck(count);

      return file.read(buffer, count);
    }

    __optimize_file_read
    size_t File::readRaw()
    {
      bufferCheck(bufferSize);

      return file.read(buffer, bufferSize);
    }

    __optimize_file_read
    size_t File::read(uint8_t * buf, size_t count)
    {
      int readCount = file.read(buf, count);

      if (readCount < 0)
      {
        OSCR::UI::fatalErrorStorage();
      }

      return readCount;
    }

    __optimize_file_read
    size_t File::fill()
    {
      bufferCheck(bufferSize);

      return read(buffer, bufferSize);
    }

    __optimize_file_read
    int16_t File::read()
    {
      return file.read();
    }

    __optimize_file_read
    size_t File::read(size_t count)
    {
      bufferCheck(count);

      return read(buffer, count);
    }

    // Read terminator from char

    __optimize_file_read
    size_t File::readBytesUntil(char terminator, uint8_t * buf, size_t count)
    {
      return file.readBytesUntil(terminator, buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil(char terminator, char * buf, size_t count)
    {
      return file.readBytesUntil(terminator, buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil(char terminator, size_t count)
    {
      bufferCheck(count);
      return file.readBytesUntil(terminator, buffer, count);
    }

    // Read terminator from string in PROGMEM

    __optimize_file_read
    size_t File::readBytesUntil(__StringHelper const * terminator, uint8_t * buf, size_t count)
    {
      char tmpTerminator[2]; // terminator + null
      OSCR::Util::copyStr_P(BUFFN(tmpTerminator), terminator);

      return readBytesUntil(tmpTerminator[0], buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil(__StringHelper const * terminator, char * buf, size_t count)
    {
      return readBytesUntil(terminator, (uint8_t *)buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil(__StringHelper const * terminator, size_t count)
    {
      char tmpTerminator[2]; // terminator + null
      OSCR::Util::copyStr_P(BUFFN(tmpTerminator), terminator);

      return readBytesUntil(tmpTerminator[0], buffer, count);
    }

    // Read terminator from byte in PROGMEM

    __optimize_file_read
    size_t File::readBytesUntil_P(uint8_t const * terminator, uint8_t * buf, size_t count)
    {
      return readBytesUntil(pgm_read_byte(terminator), buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil_P(uint8_t const * terminator, char * buf, size_t count)
    {
      return readBytesUntil(pgm_read_byte(terminator), buf, count);
    }

    __optimize_file_read
    size_t File::readBytesUntil_P(uint8_t const * terminator, size_t count)
    {
      return readBytesUntil(pgm_read_byte(terminator), buffer, count);
    }

    // Find
    bool File::find(uint8_t const * target)
    {
      return file.find((uint8_t *)target);
    }

    bool File::find(char const * target)
    {
      return find((uint8_t *)target);
    }

    bool File::find(__StringHelper const * target)
    {
      char tmpTarget[kFilePathnameLength]; // terminator + null
      OSCR::Util::copyStr_P(BUFFN(tmpTarget), target);
      return find((uint8_t *)tmpTarget);
    }

    bool File::remove()
    {
      bool result = file.remove();

      if (result) opened = false;

      return result;
    }

    uint32_t File::fileSize() const
    {
      return OSCR::Util::clamp<uint32_t>(file.fileSize(), 0UL, UINT32_MAX);
    }

    uint64_t File::fileSize64() const
    {
      return file.fileSize();
    }

    uint32_t File::available() const
    {
      return OSCR::Util::clamp<uint32_t>(file.available64(), 0UL, UINT32_MAX);
    }

    uint64_t File::available64() const
    {
      return file.available64();
    }

    void File::enableFatalErrors()
    {
      fatalErrors = true;
    }

    void File::disableFatalErrors()
    {
      fatalErrors = false;
    }

    bool File::getName(char* name, size_t len)
    {
      return file.getName(name, len);
    }

    uint64_t File::curPosition() const
    {
      return file.curPosition();
    }

    bool File::seekSet(uint64_t pos)
    {
      return file.seekSet(pos);
    }

    bool File::seekCur(int64_t offset)
    {
      return seekSet(curPosition() + offset);
    }

    bool File::seekEnd(int64_t offset)
    {
      return file.seekEnd(offset);
    }

    __optimize_file_write
    void File::rewind()
    {
      file.rewind();
    }

    //Path::Path()
    //{
      // ...
    //}

    bool Path::getPath(char buffer[], size_t bufferSize) const
    {
      buffer[0] = '/';

      if (depth == 0)
      {
        buffer[1] = '\0';
        return true;
      }

      size_t pos = 1;

      for (size_t currentDepth = 0; pos < bufferSize && currentDepth < depth; currentDepth++)
      {
        if (pos > 1)
        {
          buffer[pos] = '/';
          pos++;
        }

        size_t copyMax = min(kMaxNameLength, kMaxPathLength - pos - 1);
        size_t pathLength = strlen(path[currentDepth]);

        if (pathLength > copyMax)
        {
          OSCR::UI::clear();
          OSCR::UI::fatalError(F("Name exceeds limit"));
        }

        size_t newLength = pathLength + pos;

        if (newLength > bufferSize)
        {
          OSCR::UI::clear();
          OSCR::UI::fatalError(F("Path exceeds limit"));
        }

        strncpy(buffer + pos, path[currentDepth], copyMax);

        pos += pathLength;
      }

      return true;
    }

    String Path::getPath() const
    {
      char cwd[kMaxPathLength] = { '/', 0x00 };

      if (depth == 0) return String(cwd);

      uint8_t pos = 1;

      for (uint8_t currentDepth = 0; pos < kMaxPathLength && currentDepth < depth; currentDepth++)
      {
        if (pos > 1)
        {
          cwd[pos] = '/';
          pos++;
        }
        strlcpy(cwd + pos, path[currentDepth], min(kMaxNameLength, kMaxPathLength - pos - 1));
        pos += strlen(path[currentDepth]);
      }

      return String(cwd);
    }

    void Path::cd()
    {
      for (uint8_t i = 0; i < kMaxPathDepth; i++)
      {
        path[i][0] = '\0';
      }

      depth = 0;
    }

    void Path::cd(char const * dir)
    {
      if (depth >= kMaxPathDepth)
      {
        OSCR::UI::fatalError(F("File::cd exceeds depth limit"));
      }

      strlcpy(path[depth], dir, kMaxNameLength);

      depth++;
    }

    void Path::cd(__StringHelper const * dir)
    {
      if (depth >= kMaxPathDepth)
      {
        OSCR::UI::fatalError(F("File::cd exceeds depth limit"));
      }

      strlcpy_P(path[depth], FSP(dir), kMaxNameLength);
      depth++;
    }

    void Path::chdir()
    {
      OSCR::Storage::sd.chdir();

      if (depth == 0) return;

      for (uint8_t i = 0; i < depth; i++)
      {
        if (!OSCR::Storage::sd.exists(path[i]))
        {
          sd.mkdir(path[i]);
        }

        OSCR::Storage::sd.chdir(path[i]);
      }
    }

    uint8_t Path::getDepth()
    {
      return depth;
    }

    namespace Shared
    {
      File sharedFile; // previously myFile
      Path sharedFilePath{};

      __constinit uint8_t buffer[kBufferSize]; // previously sdBuffer

      __constinit char sharedFileName[kFileNameLength];
      __constinit char sharedFilePathname[kFilePathnameLength];

      // 4 chars for console type, 4 chars for SAVE/ROM, 21 chars for ROM name, 4 chars for folder number, 3 chars for slashes, one char for termination, one char savety
      __constinit uint16_t folderIncrement = 0; // previously foldern
      __constinit char folderIncrementStr[kFolderIncrementLength];

      __optimize_shared_file
      void open(oflag_t oflag)
      {
        sharedFilePath.chdir();
        sharedFile.open(sharedFileName, oflag);
        sharedFile.assignBuffer(BUFFN(buffer));
      }

      void openRW()
      {
        open(O_RDWR);
      }

      void openRWC()
      {
        open(O_RDWR | O_CREAT);
      }

      void openRO()
      {
        open(O_RDONLY);
      }

      __optimize_shared_file
      uint32_t getSize()
      {
        return sharedFile.fileSize();
      }

      __optimize_shared_file
      uint32_t available()
      {
        return OSCR::Util::clamp<uint32_t>(sharedFile.available64(), 0UL, UINT32_MAX);
      }

      __optimize_shared_file
      size_t fill()
      {
        return sharedFile.fill();
      }

      size_t dump()
      {
        return sharedFile.write();
      }

      __optimize_shared_file
      void rewind()
      {
        sharedFile.rewind();
      }

      __optimize_shared_file
      size_t readBuffer(size_t length)
      {
        length = OSCR::Util::clamp(length, 0U, kBufferSize);
        return sharedFile.read(buffer, length);
      }

      __optimize_shared_file
      size_t writeBuffer(size_t length)
      {
        length = OSCR::Util::clamp(length, 0U, kBufferSize);

        return sharedFile.write(buffer, length);
      }

      __optimize_shared_file
      void crcReset()
      {
        sharedFile.crcReset();
      }

      __optimize_shared_file
      OSCR::CRC32::crc32_t getCRC32()
      {
        OSCR::CRC32::current = sharedFile.getCRC32();
        return OSCR::CRC32::current;
      }

      void close()
      {
        sharedFile.close();
        getCRC32();
        sharedFile.unassignBuffer();
      }

      void cd()
      {
        sharedFilePath.cd();
        sharedFilePathname[0] = '\0';
        sd.chdir();
      }

      void cd(char const * folder)
      {
        sharedFilePath.cd(folder);
        sharedFilePath.chdir();
      }

      void cd(__StringHelper const * folder)
      {
        sharedFilePath.cd(folder);
        sharedFilePath.chdir();
      }

      void updateName(char const * gameName, char const * fileSuffix)
      {
        char fileSfx[8];

        OSCR::Util::setNulls(BUFFN(fileSfx));
        OSCR::Util::setNulls(BUFFN(sharedFileName));

        if (!OSCR::Util::copyStrLwr(BUFFN(fileSfx), fileSuffix))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }

        int32_t bufferWritten = snprintf_P(BUFFN(sharedFileName), FilenameTemplate, gameName, fileSfx);

        if ((bufferWritten < 0) || (bufferWritten > kFileNameLength))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }
      }

      void updateName(char const * gameName, __StringHelper const * fileSuffix)
      {
        char fileSfx[8];

        OSCR::Util::setNulls(BUFFN(fileSfx));
        OSCR::Util::setNulls(BUFFN(sharedFileName));

        if (!OSCR::Util::copyStrLwr_P(BUFFN(fileSfx), fileSuffix))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }

        int32_t bufferWritten = snprintf_P(BUFFN(sharedFileName), FilenameTemplate, gameName, fileSfx);

        if ((bufferWritten < 0) || (bufferWritten > kFileNameLength))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }
      }

      void updateName(char const * gameName, uint16_t fileSuffix)
      {
        OSCR::Util::setNulls(BUFFN(sharedFileName));

        int32_t bufferWritten = snprintf_P(BUFFN(sharedFileName), PSTR("%s.%03" PRIu16), gameName, fileSuffix);

        if ((bufferWritten < 0) || (bufferWritten > kFileNameLength))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }
      }

      void createFolder(char const * system, char const * subfolder, char const * gameName)
      {
        char systemUpr[kMaxNameLength];

        OSCR::Util::setNulls(BUFFN(systemUpr));

        if (!OSCR::Util::copyStrUpr(BUFFN(systemUpr), system))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }

        sharedFilePath.cd();
        sharedFilePath.cd(systemUpr);
        if (subfolder != NULL) sharedFilePath.cd(subfolder);
        sharedFilePath.cd(gameName);
        sharedFilePath.cd(folderIncrementStr);
        sharedFilePath.chdir();
      }

      void createFolder(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName)
      {
        char systemUpr[kMaxNameLength];

        OSCR::Util::setNulls(BUFFN(systemUpr));

        if (!OSCR::Util::copyStrUpr_P(BUFFN(systemUpr), system))
        {
          OSCR::debugLine(ThisFilename, __LINE__, OSCR::Strings::Errors::NameOverflow);
          OSCR::UI::fatalErrorNameOverflow();
        }

        sharedFilePath.cd();
        sharedFilePath.cd(systemUpr);
        if (subfolder != NULL) sharedFilePath.cd(subfolder);
        sharedFilePath.cd(gameName);
        sharedFilePath.cd(folderIncrementStr);
        sharedFilePath.chdir();
      }

      void incrementFolder()
      {
# if ((OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_INCREMENT) || (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_BOTH))
        folderIncrement = folderIncrement + 1;
        EEPROM_writeAnything(0, folderIncrement);
# endif
        updatedFolderIncrement();
      }

      void createFile(char const * system, char const * subfolder, char const * gameName, char const * fileSuffix)
      {
        incrementFolder();

        createFolder(system, subfolder, gameName);

        updateName(gameName, (fileSuffix == nullptr) ? system : fileSuffix);

        openRWC();
      }

      void createFile(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName, __StringHelper const * fileSuffix)
      {
        incrementFolder();

        createFolder(system, subfolder, gameName);

        updateName(gameName, (fileSuffix == nullptr) ? system : fileSuffix);

        openRWC();
      }

      void createFile(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName, uint16_t fileSuffix)
      {
        incrementFolder();

        createFolder(system, subfolder, gameName);

        updateName(gameName, fileSuffix);

        openRWC();
      }

      void createFileCWD(char const * gameName, char const * fileSuffix)
      {
        if (sharedFilePath.getDepth() == 0) OSCR::UI::fatalErrorStorage();

        sharedFilePath.chdir();

        updateName(gameName, fileSuffix);

        openRWC();
      }

      void createFileCWD(char const * gameName, __StringHelper const * fileSuffix)
      {
        if (sharedFilePath.getDepth() == 0) OSCR::UI::fatalErrorStorage();

        sharedFilePath.chdir();

        updateName(gameName, fileSuffix);

        openRWC();
      }

      void createFileCWD(char const * gameName, uint16_t fileSuffix)
      {
        if (sharedFilePath.getDepth() == 0) OSCR::UI::fatalErrorStorage();

        sharedFilePath.chdir();

        updateName(gameName, fileSuffix);

        openRWC();
      }

      void rename(char const * newName)
      {
        bool wasOpen = sharedFile.isOpen();

        if (!wasOpen)
        {
          openRO();
        }

        if (sharedFile.rename(newName))
        {
          char const * newPathFile = strrchr(newName, '/');

          if (newPathFile == NULL) newPathFile = newName;

          OSCR::Util::copyStr(BUFFN(sharedFileName), newPathFile);
        }

        if (!wasOpen)
        {
          close();
        }
      }

      void rename(__StringHelper const * newName)
      {
        bool wasOpen = sharedFile.isOpen();

        if (!wasOpen)
        {
          openRO();
        }

        if (sharedFile.rename(newName))
        {
          char const * newPathFile = strrchr_P(FSP(newName), '/');

          if (newPathFile == NULL) newPathFile = FSP(newName);

          OSCR::Util::copyStr_P(BUFFN(sharedFileName), newPathFile);
        }

        if (!wasOpen)
        {
          close();
        }
      }

      bool rename_P(char const * baseName, char const * fileExt)
      {
        char fileSfx[8];
        char newFileName[kFileNameLength];

        if (!OSCR::Util::copyStrLwr_P(BUFFN(fileSfx), fileExt)) return false;

        int32_t bufferWritten = snprintf_P(BUFFN(newFileName), FilenameTemplate, baseName, fileSfx);

        if ((bufferWritten < 0) || (bufferWritten > sizeof(newFileName))) return false;

        rename(newFileName);

        return true;
      }

      bool rename_P(char const * baseName, __StringHelper const * fileExt)
      {
        return rename_P(baseName, FSP(fileExt));
      }

      bool renameTemplate(char const * pathTemplate, char const * newName)
      {
        char newFileName[kFileNameLength];

        bool wasSuccess = OSCR::Util::applyTemplate(BUFFN(newFileName), pathTemplate, newName);

        if (!wasSuccess) return false;

        rename(newFileName);

        return true;
      }

      bool renameTemplate_P(char const * pathTemplate, char const * newName)
      {
        char newFileName[kFileNameLength];

        bool wasSuccess = OSCR::Util::applyTemplate_P(BUFFN(newFileName), pathTemplate, newName);

        if (!wasSuccess) return false;

        rename(newFileName);

        return true;
      }

      bool renameTemplate_P(char const * pathTemplate, __StringHelper const * newName)
      {
        return renameTemplate_P(pathTemplate, FSP(newName));
      }

      bool renameTemplate_P(char const * pathTemplate, char const * baseName, char const * fileExt)
      {
        char fileSfx[8];
        char newFileName[kFileNameLength];
        char newFilePath[kFileNameLength];

        if (!OSCR::Util::copyStrLwr_P(BUFFN(fileSfx), fileExt)) return false;

        int32_t bufferWritten = snprintf_P(BUFFN(newFileName), FilenameTemplate, baseName, fileSfx);

        if ((bufferWritten < 0) || (bufferWritten > sizeof(newFileName))) return false;

        if (!OSCR::Util::applyTemplate_P(BUFFN(newFilePath), pathTemplate, newFileName)) return false;

        rename(newFilePath);

        return true;
      }

      bool renameTemplate_P(char const * pathTemplate, char const * baseName, __StringHelper const * fileExt)
      {
        return renameTemplate_P(pathTemplate, baseName, FSP(fileExt));
      }
    }
  }
}

#endif /* OSCR_ARCH_AVR  */

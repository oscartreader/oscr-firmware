/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#pragma once
#ifndef OSCR_STORAGE_H_
# define OSCR_STORAGE_H_

# include "syslibinc.h"
# include "config.h"
# include "common/Types.h"

namespace OSCR
{
  namespace Storage
  {
    constexpr size_t const kMaxPathDepth = PATH_MAX_DEPTH;
    constexpr size_t const kMaxNameLength = UI_FILE_BROWSER_FILENAME_MAX;
    constexpr size_t const kMaxPathLength = (kMaxPathDepth * (kMaxNameLength + 1)) + 1;
    constexpr uint8_t const kFileNameLength = 100;
    constexpr uint8_t const kFilePathnameLength = 132;
    constexpr size_t const kBufferSize = 512;
# if (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_INCREMENT)
    constexpr uint8_t const kFolderIncrementLength = 5; // IIII = 4 + NUL
# elif (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_RTC)
    constexpr uint8_t const kFolderIncrementLength = 11; // YYMMDDHHMM = 10 + NUL
# elif (OPTION_UNIQUE_DIRECTORY_METHOD == UNQDIR_BOTH)
    constexpr uint8_t const kFolderIncrementLength = 15; // YYMMDDHHMMIIII = 14 + NUL
# endif

    extern char const PROGMEM AllowedSymbols[];
    extern char const PROGMEM FilenameTemplate[];
    extern char const PROGMEM FilenameTemplateP[];
    extern char const PROGMEM FilenameTemplatePP[];

    enum class CRC32State : uint8_t
    {
      None,
      Summing,
      Summed,
    };

    extern SdFs sd;

    extern void setup();

    extern uint8_t copyFileName(char const * const src, uint8_t const srcMaxLen, char * const dest, uint8_t const destMaxLen);
    extern uint8_t copyFileName_P(char const * const src, char * const dest, uint8_t const destMaxLen);

# if defined(ENABLE_RTC)
    extern void dateTimeCB(uint16_t * date, uint16_t * time);
# endif

    extern bool exists(__StringHelper const * path);
    extern bool exists(char const * path);

    extern bool isDir(char const * path);
    extern bool isDir(__StringHelper const * path);

    extern bool isFile(char const * path);
    extern bool isFile(__StringHelper const * path);

    extern bool createFolder(char const * path, bool recursive = true);
    extern bool createFolder(__StringHelper const * path, bool recursive = true);

    class Path
    {
    public:
      Path() = default;

      bool getPath(char buffer[], size_t bufferSize = kMaxPathLength) const;

      String getPath() const;

      void cd();
      void cd(char const * dir);
      void cd(__StringHelper const * dir);

      void chdir();

      uint8_t getDepth();

    protected:
      uint8_t depth = 0;
      char path[kMaxPathDepth][kMaxNameLength];
    };

    class File
    {
    public:
      File();

      File(uint8_t * fileBuffer, uint16_t fileBufferSize);

      File(char const * fileName, oflag_t oflag);
      File(__StringHelper const * fileName, oflag_t oflag);

      void assignBuffer(uint8_t * fileBuffer, uint16_t fileBufferSize);
      void unassignBuffer();
      bool hasBuffer() const;

      bool open(char const * fileName, oflag_t oflag);
      bool open(__StringHelper const * fileName, oflag_t oflag);
      bool open(uint32_t index, oflag_t oflag);

      bool isOpen();

      bool close();

      void sync();

      bool rename(char const * newPath);
      bool rename(__StringHelper const * newPath);

      bool writeRaw(uint8_t byte);
      size_t writeRaw(size_t count);

      size_t writeRaw(uint8_t const * buf, size_t count);

      size_t write();
      size_t write(size_t count);

      bool write(uint8_t byte);

      size_t write(char const * buf);
      size_t write(char const * buf, size_t count);
      size_t write(uint8_t const * buf, size_t count);

      size_t writeNewLine();

      uint8_t peek();

      size_t readRaw();
      size_t readRaw(size_t count);
      size_t readRaw(uint8_t * buf, size_t count);

      size_t fill();

      int16_t read();
      size_t read(size_t count);
      size_t read(uint8_t * buf, size_t count);

      bool find(uint8_t const * target);
      bool find(char const * target);
      bool find(__StringHelper const * target);

      size_t readBytesUntil(char terminator, uint8_t * buf, size_t count);
      size_t readBytesUntil(char terminator, char * buf, size_t count);
      size_t readBytesUntil(char terminator, size_t count);

      size_t readBytesUntil(__StringHelper const * terminator, uint8_t * buf, size_t count);
      size_t readBytesUntil(__StringHelper const * terminator, char * buf, size_t count);
      size_t readBytesUntil(__StringHelper const * terminator, size_t count);

      size_t readBytesUntil_P(uint8_t const * terminator, uint8_t * buf, size_t count);
      size_t readBytesUntil_P(uint8_t const * terminator, char * buf, size_t count);
      size_t readBytesUntil_P(uint8_t const * terminator, size_t count);

      uint32_t fileSize() const;
      uint64_t fileSize64() const;

      uint32_t available() const;
      uint64_t available64() const;

      bool remove();

      uint64_t curPosition() const;
      bool seekCur(int64_t offset);
      bool seekSet(uint64_t pos);
      bool seekEnd(int64_t offset = 0);
      void rewind();

      void crcReset();
      OSCR::CRC32::crc32_t getCRC32();

      void enableFatalErrors();
      void disableFatalErrors();

      bool getName(char * name, size_t len);

    protected:
      FsFile file;
      oflag_t fileOflag;
      bool fatalErrors = true;
      bool opened = false;
      CRC32State crcState = CRC32State::None;
      OSCR::CRC32::crc32_t fCRC32 = OSCR::CRC32::crc32_t();

      uint8_t * buffer = nullptr;
      uint16_t bufferSize = 0;

      void bufferCheck(size_t count);
      void postOpen();
      void postClose();
      void sumCRC32();
    };

    namespace Shared
    {
      extern File sharedFile;
      extern Path sharedFilePath;

      extern __constinit uint8_t buffer[kBufferSize];

      extern __constinit char sharedFileName[kFileNameLength];
      extern __constinit char sharedFilePathname[kFilePathnameLength];

      extern __constinit uint16_t folderIncrement;
      extern __constinit char folderIncrementStr[kFolderIncrementLength];

      /**
       * Increment (or update) the unique folder name.
       */
      extern void incrementFolder();

      /**
       * Recursively create a directory path. A new unique directory
       * will be generated and created as part of the process.
       *
       * @param system The string used as the first dir name.
       * @param subfolder The string used as the first subdir, or `nullptr`.
       * @param gameName The string to use for the third and final subfolder.
       */
      extern void createFolder(char const * system, char const * subfolder, char const * gameName);

      /**
       * Recursively create a directory path. A new unique directory
       * will be generated and created as part of the process.
       *
       * @param system The flash string used as the dir name.
       * @param subfolder The flash string used as the first subdir, or `nullptr`.
       * @param gameName The (SRAM) string to use for the third and final subfolder.
       */
      extern void createFolder(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName);

      /**
       * Recursively create a directory path as needed and create a
       * new file inside of it.
       *
       * @param system The string used as the directory name in root.
       * @param subfolder The string of the subfolder under the system directory, or `nullptr` for no subdirectory.
       * @param gameName The string to use for the third and final subfolder as well as the file name.
       * @param fileSuffix The string to use for the extension.
       */
      extern void createFile(char const * system, char const * subfolder, char const * gameName, char const * fileSuffix = NULL);

      /**
       * Recursively create a directory path as needed and create a
       * new file inside of it.
       *
       * @param system The flash string used as the directory name in root.
       * @param subfolder The flash string of the subfolder under the system directory, or `nullptr` for no subdirectory.
       * @param gameName The (SRAM) string to use for the third and final subfolder as well as the file name.
       * @param fileSuffix The flash string to use for the extension.
       */
      extern void createFile(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName, __StringHelper const * fileSuffix = NULL);

      /**
       * Recursively create a directory path as needed and create a
       * new file inside of it.
       *
       * @param system The flash string used as the directory name in root.
       * @param subfolder The flash string of the subfolder under the system directory, or `nullptr` for no subdirectory.
       * @param gameName The (SRAM) string to use for the third and final subfolder as well as the file name.
       * @param fileSuffix A number to use as the file extension. If under 3 characters, it will be left-padded with zeros.
       */
      extern void createFile(__StringHelper const * system, __StringHelper const * subfolder, char const * gameName, uint16_t fileSuffix);

      /**
       * Create a new file using the same path as the most recent
       * call to `createFile` or `createFolder`.
       *
       * @param gameName The string to use for the file name.
       * @param fileSuffix The string to use for the extension.
       */
      extern void createFileCWD(char const * gameName, char const * fileSuffix = NULL);

      /**
       * Create a new file using the same path as the most recent
       * call to `createFile` or `createFolder`.
       *
       * @param gameName The (SRAM) string to use for the file name.
       * @param fileSuffix The flash string to use for the extension.
       */
      extern void createFileCWD(char const * gameName, __StringHelper const * fileSuffix = NULL);

      /**
       * Create a new file using the same path as the most recent
       * call to `createFile` or `createFolder`.
       *
       * @param gameName The string to use for the file name.
       * @param fileSuffix A number to use as the file extension. If under 3 characters, it will be left-padded with zeros.
       */
      extern void createFileCWD(char const * gameName, uint16_t fileSuffix);

      /**
       * Open the current shared file using the provided flags.
       */
      extern void open(oflag_t oflag);

      /**
       * Open the current existing shared file in read-write mode.
       */
      extern void openRW();

      /**
       * Open the current shared file in read/write mode, creating it if needed.
       */
      extern void openRWC();

      /**
       * Open the current existing shared file in read-only mode.
       */
      extern void openRO();

      /**
       * Change to the root directory of the OSCR's storage.
       */
      extern void cd();

      /**
       * Change the current directory to the specified subfolder. The
       * provided path must be a valid directory name and cannot be
       * absolute or contain relatives (i.e. `../name` is invalid).
       *
       * If the directory does not exist, it will be created.
       *
       * @param dir The name of the directory.
       */
      extern void cd(char const * dir);

      /**
       * Change the current directory to the specified subfolder. The
       * provided path must be a valid directory name and cannot be
       * absolute or contain relatives (i.e. `../name` is invalid).
       *
       * If the directory does not exist, it will be created.
       *
       * @param newName A flash string with the directory name.
       */
      extern void cd(__StringHelper const * dir);

      /**
       * Renames the shared file. This can also be used to move a
       * file, but it will not update the shared path to the new dir.
       *
       * @param newName The new name.
       *
       * @return Success/failure
       */
      extern void rename(char const * newName);

      /**
       * Renames the shared file. This can also be used to move a
       * file, but it will not update the shared path to the new dir.
       *
       * @param newName The flash string with the new name.
       *
       * @return Success/failure
       */
      extern void rename(__StringHelper const * newName);

      /**
       * Renames the shared file.
       *
       * @param baseName The (SRAM) string with the new name.
       * @param baseName The PROGMEM string with the extension.
       *
       * @return Success/failure
       */
      extern bool rename_P(char const * baseName, char const * fileExt);

      /**
       * Renames the shared file.
       *
       * @param baseName The (SRAM) string with the new name.
       * @param baseName The flash string with the extension.
       *
       * @return Success/failure
       */
      extern bool rename_P(char const * baseName, __StringHelper const * fileExt);

      /**
       * Renames the shared file using the provided template.
       *
       * @param pathTemplate The PROGMEM string with the template.
       * @param newName The string with the file name.
       *
       * @return Success/failure
       */
      extern bool renameTemplate_P(char const * pathTemplate, char const * newName);

      /**
       * Renames the shared file using the provided template.
       *
       * @param pathTemplate The PROGMEM string with the template.
       * @param newName The flash string with the file name.
       *
       * @return Success/failure
       */
      extern bool renameTemplate_P(char const * pathTemplate, __StringHelper const * newName);

      /**
       * Renames the shared file using the provided template.
       *
       * @param pathTemplate The PROGMEM string with the template.
       * @param baseName The string with the file's base name.
       * @param baseName The PROGMEM string with the file's extension.
       *
       * @return Success/failure
       */
      extern bool renameTemplate_P(char const * pathTemplate, char const * baseName, char const * newExt);

      /**
       * Renames the shared file using the provided template.
       *
       * @param pathTemplate The PROGMEM string with the template.
       * @param baseName The string with the file's base name.
       * @param baseName The flash string with the file's extension.
       *
       * @return Success/failure
       */
      extern bool renameTemplate_P(char const * pathTemplate, char const * baseName, __StringHelper const * newExt);

      /**
       * Get the size of the shared file.
       *
       * @note This function returns a 32-bit number that has been
       *       clamped from a 64-bit value. If the file is larger
       *       than a 32-bit unsigned int can represent, `UINT32_MAX`
       *       will be returned.
       */
      extern uint32_t getSize();

      /**
       * The number of bytes remaining from the current position.
       *
       * @note This function returns a 32-bit number that has been
       *       clamped from a 64-bit value. If the file is larger
       *       than a 32-bit unsigned int can represent, `UINT32_MAX`
       *       will be returned until the it is less than that.
       */
      extern uint32_t available();

      /**
       * Read data into the shared buffer, up to the maximum size of
       * the buffer or the remaining data in the file.
       *
       * @return The number of bytes placed into the buffer.
       */
      extern size_t fill();

      /**
       * Write the all of the data in the shared buffer.
       *
       * @return The number of bytes written.
       */
      extern size_t dump();

      /**
       * Sets the file's position to the start of the file.
       */
      extern void rewind();

      /**
       * Read data into the shared buffer, up to the maximum size of
       * the buffer, the specified length, or the remaining data in
       * the file, whichever is shorter.
       *
       * @param length Maximum number of bytes to write [default: `kBufferSize`]
       *
       * @return The number of bytes placed into the buffer.
       */
      extern size_t readBuffer(size_t length = kBufferSize);

      /**
       * Write the data in the shared buffer, up to the maximum size
       * of the buffer or the specified length.
       *
       * @param length Maximum number of bytes to write [default: `kBufferSize`]
       *
       * @return The number of bytes written.
       */
      extern size_t writeBuffer(size_t length = kBufferSize);

      /**
       * Reset the CRC checksum.
       */
      extern void crcReset();

      /**
       * Get the current CRC32.
       */
      extern OSCR::CRC32::crc32_t getCRC32();

      /**
       * Close the shared file.
       */
      extern void close();
    } /* namespace Shared */
  } /* namespace Storage */
} /* namespace OSCR */

#endif /* OSCR_STORAGE_H_ */

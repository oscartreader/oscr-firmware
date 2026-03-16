#include "common/Configuration.h"
#include "common/Util.h"

namespace OSCR::Configuration
{
#if defined(ENABLE_CONFIG)

  OSCR::Storage::File configFile;
  __constinit bool useConfig = false;

  namespace
  {
    // Together these eat 9 bytes of SRAM that will never be freed, and
    //  sadly, there doesn't seem to be a way around it.
    __constinit uint8_t callbackCount = 0; // Stores the number of registered callbacks (`UINT8_MAX` when full or already ran) [1 Byte of SRAM]
    __constinit ConfigCallback * callbacks = nullptr; // Stores the pointer to our array of callbacks [8 Bytes of SRAM]

    void runCallbacks()
    {
      if (callbacks == nullptr) return;

      for (uint8_t i = 0; i < callbackCount; i++)
      {
        callbacks[i]();
      }

      // Cleanup cb array
      delete [] callbacks;

      // Prevent new callbacks being registered
      callbackCount = UINT8_MAX;
    }
  }

  void init()
  {
    if (useConfig) return;

    useConfig = configFile.open(F(CONFIG_FILE), O_READ);

    runCallbacks();
  }

  bool enabled()
  {
    return useConfig;
  }

  void registerCallback(ConfigCallback cb)
  {
    // If `cb` is null, do nothing.
    if (cb == nullptr) return;

    // If config is ready, just run the cb
    if (useConfig)
    {
      cb();
      return;
    }

    // If deregistered or full somehow
    if (callbackCount == (uint8_t)UINT8_MAX) return;

    bool shouldRegister = true;

    // If this is not the first cb registered, copy the existing array first
    if (callbackCount > 0)
    {
      // New array
      ConfigCallback * _callbacks = new ConfigCallback[callbackCount + 1];

      for (uint8_t i = 0; i < callbackCount; i++)
      {
        // Dupe check
        if (callbacks[i] == cb)
        {
          shouldRegister = false;
        }

        _callbacks[i] = callbacks[i];
      }

      delete [] callbacks;
      callbacks = _callbacks;
    }
    else if (callbacks == nullptr) // If this is the first cb, create a new array
    {
      callbacks = new ConfigCallback[1];
    }

    if (shouldRegister)
    {
      callbacks[callbackCount++] = cb;
    }
  }

  /* PROCESS
   * =======
   *  - [1]  Get key length and convert to char array/string.
   *  - [2]  Parse file line by line.
   *  - [3]  Check if key in file matches.
   *  - [4]  Copy string after equals character.
   *  - [5]  Add null terminator.
   *
   * NOTES
   * -----
   *  - You aren't meant to use this function directly. Use the
   *    functions `getStr()` and `getLong()` instead.
   */
  uint8_t findKey(__FlashStringHelper const * searchKey, char * value)
  {
    if (!useConfig) return 0;

    char key[CONFIG_KEY_MAX + 1]; // null term
    char buffer[CONFIG_KEY_MAX + CONFIG_VALUE_MAX + 4]; // = \r \n null term
    int keyLen = 0;
    int valueLen = 0;

    keyLen = strlcpy_P(key, reinterpret_cast<char const *>(searchKey), CONFIG_KEY_MAX); /*[1]*/

    configFile.rewind();

    while (configFile.available()) /*[2]*/
    {
      int bufferLen = configFile.readBytesUntil(F("\n"), buffer, CONFIG_KEY_MAX + CONFIG_VALUE_MAX + 3);

      if (buffer[bufferLen - 1] == '\r') // ignore \r
        bufferLen--;

      if (bufferLen > (keyLen + 1))
      {
        if (memcmp(buffer, key, keyLen) == 0) /*[3]*/
        {
          if (buffer[keyLen] == '=') /*[4]*/
          {
            valueLen = bufferLen - keyLen - 1;
            memcpy(&value[0], &buffer[keyLen + 1], valueLen);
            value[valueLen] = '\0'; /*[5]*/

            break;
          }
        }
      }
    }

    return valueLen;
  }

  bool getBool(__FlashStringHelper const * key, bool & dest)
  {
    char value[CONFIG_VALUE_MAX + 1];

    uint8_t valueLen = findKey(key, value);

    if (valueLen < 1) return false;

    if (strcasecmp_P(value, PSTR("true")) == 0)
    {
      dest = true;
      return true;
    }

    if (strcasecmp_P(value, PSTR("false")) == 0)
    {
      dest = false;
      return true;
    }

    uint32_t result;

    bool readInt = OSCR::Util::strToInt(value, result);

    if (!readInt) return false;

    dest = (result > 0);

    return true;
  }

  /* PROCESS
   * =======
   *  - [1] Find the key via findKey().
   *  - [2] Check if the key was found.
   *  - [3] Convert the `char` array to an `int32`.
   *  - [4] Compute max/min values based on `dest`s type.
   *  - [5] Check if within bounds.
   *  - [6] Set dest to value.
   *
   * NOTES
   * -----
   *  - Since `maxSize` is a constexpr, the 64-bit cast does not
   *    affect PROGMEM usage.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_signed<T>::value, bool> Enable>
  bool getInteger(__FlashStringHelper const * key, T & dest)
  {
    char value[CONFIG_VALUE_MAX + 1];

    uint8_t valueLen = findKey(key, value); /*[1]*/

    if (valueLen < 1) return false; /*[2]*/

    int32_t tempValue;

    if (!OSCR::Util::strToInt(value, tempValue)) return false; /*[3]*/

    /*[4]*/
    constexpr int32_t const maxSize = ((int64_t)1 << ((8 * sizeof(T)) - 1)) - 1;
    constexpr int32_t const minSize = -maxSize;

    if ((tempValue > maxSize) || (tempValue < minSize)) return false; /*[5]*/

    dest = tempValue;

    return true;
  }

  template bool getInteger<int8_t>(__FlashStringHelper const *, int8_t &);
  template bool getInteger<int16_t>(__FlashStringHelper const *, int16_t &);
  template bool getInteger<int32_t>(__FlashStringHelper const *, int32_t &);
  template bool getInteger<uint8_t>(__FlashStringHelper const *, uint8_t &);
  template bool getInteger<uint16_t>(__FlashStringHelper const *, uint16_t &);
  template bool getInteger<uint32_t>(__FlashStringHelper const *, uint32_t &);

  /* PROCESS
   * =======
   *  - [1] Find the key via findKey().
   *  - [2] Check if the key was found.
   *  - [3] Convert the char array to an `int32`.
   *  - [4] Compute max/min values based on dest type.
   *  - [5] Check if within bounds.
   *  - [6] Set dest to value.
   *
   * NOTES
   * -----
   *    Since `maxSize` is a constexpr, the 64-bit cast does not
   *    affect PROGMEM usage.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_unsigned<T>::value, bool> Enable>
  bool getInteger(__FlashStringHelper const * key, T & dest)
  {
    char value[CONFIG_VALUE_MAX + 1];

    uint8_t valueLen = findKey(key, value);

    if (valueLen < 1) return false;

    uint32_t tempValue;

    if (!OSCR::Util::strToInt(value, tempValue)) return false;

    constexpr uint32_t const maxSize = ((uint64_t)1 << (8 * sizeof(T))) - 1;

    if (tempValue > maxSize) return false;

    dest = tempValue;

    return true;
  }

  /* PROCESS
   * =======
   *  - [1]  Find the key via findKey().
   *  - [2]  Return empty if nothing was found.
   *  - [3]  Convert to String type.
   *
   * NOTES :
   *  - You could use this to get strings stored in the config file.
   *  - You probably shouldn't be using this.
   */
  String getString(__FlashStringHelper const * key)
  {
    if (!useConfig) return {};
    char value[CONFIG_VALUE_MAX + 1];

    uint8_t valueLen = findKey(key, value); /*[1]*/

    if (valueLen < 1) return {}; /*[2]*/

    return String(value); /*[3]*/
  }
#else /* !ENABLE_CONFIG */
  bool enabled()
  {
    return false;
  }

  void init()
  {
    // ...
  }
#endif /* ENABLE_CONFIG */
}

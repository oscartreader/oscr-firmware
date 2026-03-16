#pragma once
#ifndef OSCR_CONFIGURATION_H_
# define OSCR_CONFIGURATION_H_

# include "common/Types.h"
# include "common/specializations.h"
# include "api/Storage.h"

namespace OSCR::Configuration
{
  /**
   * Setup the config file.
   */
  extern void init();

  extern bool enabled();

  extern void registerCallback(ConfigCallback cb);

  /**
   * Search for a key=value pair.
   *
   * @param searchKey The key to get the value for.
   * @param value to store the value in.
   *
   * @returns Length of the value.
   *
   * @note You aren't meant to use this function directly. Check the
   *       functions `getBool()`, `getString()`, and `getInteger()`.
   */
  extern uint8_t findKey(__FlashStringHelper const * searchKey, char * value);

  /**
   * Copy the value of `key` to `dest`.
   *
   * @param key     The key to get the value for.
   * @param dest    The destination variable to store the value in.
   *
   * @returns `true` if successful, `false` if not.
   *
   * @note The config value can be a positive number or one a string
   *       of `true` or `false`.
   */
  extern bool getBool(__FlashStringHelper const * key, bool & dest);

  /**
   * Copy the value of `key` to `dest`.
   *
   * @param key     The key to get the value for.
   * @param dest    The destination variable to store the value in.
   *
   * @returns `true` if successful, `false` if not.
   *
   * @note The value can be hex, i.e. 0xFF for 255.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_signed<T>::value, bool> Enable = true>
  extern bool getInteger(__FlashStringHelper const * key, T & dest);

  /**
   * Copy the value of `key` to `dest`.
   *
   * @param key     The key to get the value for.
   * @param dest    The destination variable to store the value in.
   *
   * @returns `true` if successful, `false` if not.
   *
   * @note The value can be hex, i.e. 0xFF for 255.
   */
  template <typename T,
            OSCR::Util::enable_if_t<OSCR::Util::is_unsigned<T>::value, bool> Enable = true>
  extern bool getInteger(__FlashStringHelper const * key, T & dest);

  /**
   * Get the value of a key as a String.
   *
   * @param searchKey The key to get the value for.
   *
   * @returns The value of the key or an empty string.
   *
   * @note You should strongly consider using `getStr()` instead.
   */
  extern String getString(__FlashStringHelper const * key);
}

#endif /* OSCR_CONFIGURATION_H_ */

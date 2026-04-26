#include <pebble.h>
#include "config.h"

static DictionaryIterator s_locale_dict_storage;
static uint8_t *s_dict_buffer = NULL;

void locale_init(void) {
#ifdef ENABLE_LOCALIZE_FORCE
  const char *locale_str = ENABLE_LOCALIZE_FORCE;
#else
  const char *locale_str = i18n_get_system_locale();
#endif

  ResHandle locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_ENGLISH);
  int locale_size = resource_size(locale_handle);

  if (strncmp(locale_str, "fr", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_FRENCH);
  } else if (strncmp(locale_str, "es", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_SPANISH);
  } else if (strncmp(locale_str, "de", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_GERMAN);
  } else if (strncmp(locale_str, "it", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_ITALIAN);
  } else if (strncmp(locale_str, "ja", 2) == 0) {
    locale_handle = resource_get_handle(RESOURCE_ID_LOCALE_JAPANESE);
  }

  locale_size = resource_size(locale_handle);

  int offset = 0;
  int entries = 0;

  offset += resource_load_byte_range(
      locale_handle,
      offset,
      (uint8_t *)&entries,
      sizeof(entries));

  typedef struct {
    int32_t hashval;
    int32_t strlen;
  } locale_entry_t;

  int dict_size = locale_size + (7 * entries);

  s_dict_buffer = malloc(dict_size);
  if (!s_dict_buffer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Locale alloc failed");
    return;
  }

  dict_write_begin(&s_locale_dict_storage, s_dict_buffer, dict_size);

  for (int i = 0; i < entries; i++) {
    locale_entry_t entry;

    offset += resource_load_byte_range(
        locale_handle,
        offset,
        (uint8_t *)&entry,
        sizeof(entry));

    char *buffer = malloc(entry.strlen + 1);
    if (!buffer) {
      continue;
    }

    offset += resource_load_byte_range(
        locale_handle,
        offset,
        (uint8_t *)buffer,
        entry.strlen);

    buffer[entry.strlen] = '\0';

    dict_write_cstring(&s_locale_dict_storage,
                       entry.hashval,
                       buffer);

    free(buffer);
  }

  dict_write_end(&s_locale_dict_storage);
}

const char *locale_str(int hashval) {
  Tuple *tuple = dict_find(&s_locale_dict_storage, hashval);

  if (!tuple || !tuple->value) {
    return "";
  }

  // SAFE access: treat as Pebble dict API intended usage
  return tuple->value->cstring ? tuple->value->cstring : "";
}

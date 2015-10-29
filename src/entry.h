#pragma once

#define ENTRY_NAME_LENGTH 32

typedef void (*EntryCallback)(char *name);

void entry_init(char *name);

void entry_deinit(void);

void entry_get_name(char *name, EntryCallback cb);

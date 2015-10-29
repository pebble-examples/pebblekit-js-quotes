#pragma once

#define ENTRY_NAME_LENGTH 32
#define ENTRY_X_OFFSET    PBL_IF_ROUND_ELSE(53, 35)

typedef void (*EntryCallback)(char *name);

void entry_init(char *name);

void entry_deinit(void);

void entry_get_name(char *name, EntryCallback cb);

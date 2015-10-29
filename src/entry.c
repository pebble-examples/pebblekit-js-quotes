#include <pebble.h>

#include "entry.h"

static Window *s_window;
static TextLayer *s_title_text, *s_chars_layers[4];
static TextLayer *s_selection_layer;

static EntryCallback s_callback;
static char *s_name; // Pointer to put the name into.
static char s_entry_name[ENTRY_NAME_LENGTH];
static char s_entry_chars[4][2];
static uint8_t s_selection_index;

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selection_index < 4) {
    if (s_entry_chars[s_selection_index][0] == 'Z') {
      s_entry_chars[s_selection_index][0] = 'A';
    } else {
      s_entry_chars[s_selection_index][0]++;
    }
    layer_mark_dirty(text_layer_get_layer(s_chars_layers[s_selection_index]));
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_selection_index < 4) {
    if (s_entry_chars[s_selection_index][0] == 'A') {
      s_entry_chars[s_selection_index][0] = 'Z';
    } else {
      s_entry_chars[s_selection_index][0]--;
    }
    layer_mark_dirty(text_layer_get_layer(s_chars_layers[s_selection_index]));
  }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text_color(s_chars_layers[s_selection_index], GColorBlack);

  if (s_selection_index == 3) {
    s_selection_index = 0;
  } else {
    s_selection_index++;
  }

  text_layer_set_text_color(s_chars_layers[s_selection_index], GColorWhite);
  layer_set_frame(text_layer_get_layer(s_selection_layer), GRect(35 + (20 * s_selection_index), 66, 15, 31));
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (s_entry_chars[3][0] == '\0') {
    s_entry_chars[3][0] = 'A';
  } else {
    s_entry_chars[3][0] = '\0';
  }

  layer_mark_dirty(text_layer_get_layer(s_chars_layers[s_selection_index]));
}

static void click_config_provider(void *context) {
  const uint16_t repeat_interval_ms = 100;
  window_single_repeating_click_subscribe(BUTTON_ID_UP, repeat_interval_ms, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, repeat_interval_ms, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_title_text = text_layer_create(GRect(0, 0, bounds.size.w, 64));
  text_layer_set_text(s_title_text, s_entry_name);
  text_layer_set_text_alignment(s_title_text, GTextAlignmentCenter);
  text_layer_set_font(s_title_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_title_text));

  s_selection_layer = text_layer_create(GRect(35, 66, 16, 31));
  text_layer_set_background_color(s_selection_layer, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(s_selection_layer));

  for (int col = 0; col < 4; col++) {
    strncpy(s_entry_chars[col], "A", 2);

    s_chars_layers[col] = text_layer_create(GRect(35 + (20 * col), 64, 15, 50));
    text_layer_set_font(s_chars_layers[col], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_chars_layers[col], GTextAlignmentCenter);
    text_layer_set_text(s_chars_layers[col], s_entry_chars[col]);
    text_layer_set_background_color(s_chars_layers[col], GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(s_chars_layers[col]));
  }

  s_selection_index = 0;
  text_layer_set_text_color(s_chars_layers[s_selection_index], GColorWhite);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_title_text);
  for (int i = 0; i < 4; i++) {
    s_name[i] = s_entry_chars[i][0];
    text_layer_destroy(s_chars_layers[i]);
  }
  text_layer_destroy(s_selection_layer);

  // NULL-terminate the string and return
  s_name[4] = '\0';
  s_callback(s_name);
}

void entry_init(char *name) {
  s_window = window_create();
  strncpy(s_entry_name, name, ENTRY_NAME_LENGTH);
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
}

void entry_deinit(void) {
  window_destroy(s_window);
}

void entry_get_name(char *name, EntryCallback callback) {
  s_callback = callback;
  s_name = name;
  window_stack_push(s_window, true);
}

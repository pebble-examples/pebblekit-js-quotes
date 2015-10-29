#include <pebble.h>

#include "entry.h"

static Window *s_main_window;
static TextLayer *s_symbol_layer;
static TextLayer *s_price_layer;
static char s_symbol[5];
static char s_price[10];

typedef enum {
  QuoteKeyInit = 0,
  QuoteKeyFetch,
  QuoteKeySymbol,
  QuoteKeyPrice,
} QuoteKey;

static bool send_to_phone_multi(int quote_key, char *s_symbol) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if(s_symbol) {
    // We are sending the stocks symbol
    dict_write_cstring(iter, quote_key, s_symbol);
  } else {
    // Some other request with no string data
    const int dummy_val = 1;
    dict_write_int(iter, quote_key, &dummy_val, sizeof(int), true);
  }

  dict_write_end(iter);
  app_message_outbox_send();
  return true;
}

static void entry_complete_handler(char *symbolName) {
  send_to_phone_multi(QuoteKeySymbol, symbolName);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *symbol_tuple = dict_find(iter, QuoteKeySymbol);
  Tuple *price_tuple = dict_find(iter, QuoteKeyPrice);

  if (symbol_tuple) {
    strncpy(s_symbol, symbol_tuple->value->cstring, 5);
    text_layer_set_text(s_symbol_layer, s_symbol);
  }
  if (price_tuple) {
    strncpy(s_price, price_tuple->value->cstring, 10);
    text_layer_set_text(s_price_layer, s_price);
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
  text_layer_set_text(s_price_layer, "Loading...");
  send_to_phone_multi(QuoteKeyFetch, NULL);
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // refresh
  entry_get_name(s_symbol, entry_complete_handler);
  text_layer_set_text(s_symbol_layer, s_symbol);
  text_layer_set_text(s_price_layer, "Loading...");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_symbol_layer = text_layer_create(GRect(0, 20, bounds.size.w, 50));
  text_layer_set_text(s_symbol_layer, "PBL");
  text_layer_set_text_alignment(s_symbol_layer, GTextAlignmentCenter);
  text_layer_set_font(s_symbol_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_symbol_layer));

  s_price_layer = text_layer_create(GRect(0, 75, bounds.size.w, 50));
  text_layer_set_text(s_price_layer, "$0000.00");
  text_layer_set_text_alignment(s_price_layer, GTextAlignmentCenter);
  text_layer_set_font(s_price_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  layer_add_child(window_layer, text_layer_get_layer(s_price_layer));

  send_to_phone_multi(QuoteKeyInit, NULL);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_symbol_layer);
  text_layer_destroy(s_price_layer);
}

static void init(void) {
  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  
  // Init buffers
  app_message_open(64, 64);

  entry_init("Enter Symbol");

  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

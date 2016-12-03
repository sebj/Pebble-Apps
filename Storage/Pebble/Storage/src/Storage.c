#include <pebble.h>

//UI
static Window *window;
static ScrollLayer *scroll_layer;
static TextLayer *text_layer;

//Maximum dimensions
static int WIDTH = 144;
static int MAX_HEIGHT = 338;

enum {
  PERSIST_INVERTED = 0,
  PERSIST_TEXT
};

bool inverted; //true = white on black, false = black on white

static char text[248];

static void setInverted(bool new_inverted) {
  if (new_inverted) {
    text_layer_set_text_color(text_layer, GColorWhite);
    text_layer_set_background_color(text_layer, GColorBlack);
    window_set_background_color(window, GColorBlack);
  } else {
    text_layer_set_text_color(text_layer, GColorBlack);
    text_layer_set_background_color(text_layer, GColorWhite);
    window_set_background_color(window, GColorWhite);
  }
  inverted = new_inverted;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //Invert colours
  setInverted(!inverted);
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void content_offset_changed(ScrollLayer *scroll_layer, void *context) {
	//Do nothing - only necessary to avoid crash
}

static void resizeScrollLayer() {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(text_layer);
  
  //If width is less than bounds, it's probably under 1 line, meaning it gets resized too heavily/badly
  if (max_size.w < bounds.size.w) {
    max_size.w = bounds.size.w;
  }

  text_layer_set_size(text_layer, GSize(max_size.w,max_size.h+12));

  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, max_size.h+14));
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *tuple = dict_find(iter, 0x0);

  if (tuple) {
    strcpy(text, tuple->value->cstring);
    text_layer_set_text(text_layer, text);
    resizeScrollLayer();
  }
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(scroll_layer, window);

  ScrollLayerCallbacks cbacks;
  cbacks.click_config_provider = &config_provider;
  cbacks.content_offset_changed_handler = &content_offset_changed;
  scroll_layer_set_callbacks(scroll_layer, cbacks);

  text_layer = text_layer_create(GRect(0, 0, WIDTH, MAX_HEIGHT));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);

  scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

  //Load persistent data

  inverted = false;
  if (persist_exists(PERSIST_INVERTED)) {
    inverted = persist_read_int(PERSIST_INVERTED);
  }
  setInverted(inverted);

  if (persist_exists(PERSIST_TEXT)) {
    persist_read_string(PERSIST_TEXT, text, sizeof(text));
    text_layer_set_text(text_layer, text);
  } else {
    text_layer_set_text(text_layer, "Waiting for text...");
  }

  resizeScrollLayer();
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
  scroll_layer_destroy(scroll_layer);
}

static void init(void) {
  window = window_create();
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);

  //~124 limit inbound, though in reality about 115 characters
  app_message_register_inbox_received(in_received_handler);
  app_message_open(124, 16);
}

static void deinit(void) {
  persist_write_int(PERSIST_INVERTED, inverted);
  persist_write_string(PERSIST_TEXT, text);

  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
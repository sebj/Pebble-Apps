#include <pebble.h>


static Window *window;
static TextLayer *battery_layer, *bluetooth_layer;


//Battery status
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%\ncharged  ";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Charging\n(%d%%)", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%\ncharged", charge_state.charge_percent);
  }
  
  text_layer_set_text(battery_layer, battery_text);

  layer_mark_dirty(window_get_root_layer(window));
}

//Bluetooth status
static void handle_bluetooth(bool connected) {
  text_layer_set_text(bluetooth_layer, connected ? "Connected\nto phone" : "Disconnected\nfrom phone");
}


//Draw layer
static void layer_update_callback(Layer *layer, GContext* ctx) {
  GRect screenFrame = layer_get_bounds(layer);
  
  //Solid fill
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, screenFrame, 0, GCornerNone);
  
  //Line
  graphics_context_set_fill_color(ctx, GColorWhite);

  BatteryChargeState charge_state = battery_state_service_peek();
  uint8_t percentage = charge_state.charge_percent;

  int16_t width = 144;
  if (percentage >= 90) {
    width = 130;
  } else if (percentage >= 80) {
    width = 115;
  } else if (percentage >= 70) {
    width = 101;
  } else if (percentage >= 60) {
    width = 86;
  } else if (percentage >= 50) {
    width = 72;
  } else if (percentage >= 40) {
    width = 58;
  } else if (percentage >= 30) {
    width = 43;
  } else if (percentage >= 20) {
    width = 29;
  } else if (percentage >= 10) {
    width = 14;
  }

  graphics_fill_rect(ctx, GRect(0, (screenFrame.size.h-2)/2, width, 2), 0, GCornerNone);
}

//Common text layer setup
static void setupTextLayer(TextLayer *layer) {
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(layer, GTextOverflowModeWordWrap);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
}


//Setting up
static void init(void) {
  //Window
  window = window_create();  
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);

  window_stack_push(window, true);
  
  Layer *window_root_layer = window_get_root_layer(window);
  layer_set_update_proc(window_root_layer, layer_update_callback);


  //Battery
  battery_layer = text_layer_create(GRect(0, 7, 144, 84));
  setupTextLayer(battery_layer);

  battery_state_service_subscribe(handle_battery);
  
  layer_add_child(window_root_layer, text_layer_get_layer(battery_layer));
  
  
  //Bluetooth
  bluetooth_layer = text_layer_create(GRect(0, 91, 144, 84));
  setupTextLayer(bluetooth_layer);

  bluetooth_connection_service_subscribe(handle_bluetooth);
  
  layer_add_child(window_root_layer, text_layer_get_layer(bluetooth_layer));


  //"Peek" at both statuses
  BatteryChargeState state = battery_state_service_peek();
  handle_battery(state);

  bool bluetooth_connected = bluetooth_connection_service_peek();
  handle_bluetooth(bluetooth_connected);
}

static void deinit(void) {
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  text_layer_destroy(battery_layer);
  text_layer_destroy(bluetooth_layer);
  
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
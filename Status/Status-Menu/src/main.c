#include <pebble.h>


static Window *window;
static MenuLayer *menu_layer;

//menu_layer_reload_data
static char battery_text[16] = "100% charged";
static char bluetooth_text[32] = "Connected to phone";

//Battery status
static void handle_battery(BatteryChargeState charge_state) {
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Charging\n(%d%%)", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }

  menu_layer_reload_data(menu_layer);
}

//Bluetooth status
static void handle_bluetooth(bool connected) {
  snprintf(bluetooth_text, sizeof(bluetooth_text), connected ? "Connected to phone" : "Disconnected from phone");

  menu_layer_reload_data(menu_layer);
}

//Menu Layer
void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  switch(cell_index->row) {
    case 0:
      menu_cell_basic_draw(ctx, cell_layer, "Battery", battery_text, NULL);
      break;

    case 1:
      menu_cell_basic_draw(ctx, cell_layer, "Bluetooth", bluetooth_text, NULL);
      break;
  }
}
 
uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return 2;
}
 
void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  //int which = cell_index->row;
}


//Setting up
static void init(void) {
  //Window
  window = window_create();
  window_set_fullscreen(window, false);

  window_stack_push(window, true);

  //Create it - 16 is approx height of the top bar
  menu_layer = menu_layer_create(GRect(0, 0, 144, 168 - 16));

  menu_layer_set_click_config_onto_window(menu_layer, window);

  MenuLayerCallbacks callbacks = {
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
    .select_click = (MenuLayerSelectCallback) select_click_callback
  };
  menu_layer_set_callbacks(menu_layer, NULL, callbacks);

  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));

  //Battery/Bluetooth status
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  //"Peek" at both statuses
  BatteryChargeState state = battery_state_service_peek();
  handle_battery(state);

  bool bluetooth_connected = bluetooth_connection_service_peek();
  handle_bluetooth(bluetooth_connected);
}

static void deinit(void) {
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  menu_layer_destroy(menu_layer);
  
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
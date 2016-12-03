#include <pebble.h>


static Window *window;
static MenuLayer *menu_layer;

static char bluetooth_text[16] = "Connected";
static char battery_text[16] = "100% charged";
static char phone_battery_text[16] = "100% charged";


//Bluetooth status
static void handle_bluetooth(bool connected) {
  snprintf(bluetooth_text, sizeof(bluetooth_text), connected ? "Connected" : "Disconnected");

  menu_layer_reload_data(menu_layer);
}

//Battery status
static void handle_battery(BatteryChargeState charge_state) {
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Charging (%d%%)", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }

  menu_layer_reload_data(menu_layer);
}


//Send Pebble battery => phone
//It responds with phone battery
void sendBattery() {
  DictionaryIterator *dict;
  if (app_message_outbox_begin(&dict) == APP_MSG_OK) {
    BatteryChargeState state = battery_state_service_peek();

    dict_write_int8(dict, 1, state.charge_percent);
    app_message_outbox_send();
  }
}

//Phone battery received
void in_rcv_handler(DictionaryIterator *received, void *context) {
  Tuple* cmd_tuple = dict_find(received, 0);

  if (cmd_tuple != NULL) {
    //Kick off refresh
    sendBattery();
  }

  cmd_tuple = dict_find(received, 2);
  if (cmd_tuple != NULL) {
    //Got phone battery
    int battery = cmd_tuple->value->int8;
    snprintf(phone_battery_text, sizeof(phone_battery_text), "%d%% charged", battery);

    menu_layer_reload_data(menu_layer);
  }
}

void sendActionCommand() {
  DictionaryIterator *dict;
  if (app_message_outbox_begin(&dict) == APP_MSG_OK) {
    dict_write_int8(dict, 3, 1);
    app_message_outbox_send();
  }
}


//Menu Layer
static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->section) {
    //First section
    case 0:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Bluetooth", bluetooth_text, NULL);
          break;

        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Battery", battery_text, NULL);
          break;
      }
      break;

    //Second section
    case 1:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Phone Battery", phone_battery_text, NULL);
          break;

        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Phone Action", "Toggle", NULL);
          break;
      }
      break;
  }
}

static void draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Pebble");
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Phone");
      break;
  }
}

static int16_t header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static uint16_t num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 2;
}
 
static uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
  return 2;
}
 
static void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Select click");

  if (cell_index->section == 1) {
    if (cell_index->row == 0) {
      sendBattery();
    } else if (cell_index->row == 1) {
      sendActionCommand();
    }
  }
}


//Setting up
static void init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "test starting");

  //Window
  window = window_create();

  //Create it - 12 is approx height of the top bar
  menu_layer = menu_layer_create(GRect(0, 0, 144, 168 - 16));

  menu_layer_set_click_config_onto_window(menu_layer, window);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) num_sections_callback,
    .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
    .get_header_height = (MenuLayerGetHeaderHeightCallback) header_height_callback,
    .draw_header = (MenuLayerDrawHeaderCallback) draw_header_callback,
    .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
    .select_click = (MenuLayerSelectCallback) select_click_callback
  });

  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));

  //Battery/Bluetooth status
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  //"Peek" at both statuses
  BatteryChargeState state = battery_state_service_peek();
  handle_battery(state);

  bool bluetooth_connected = bluetooth_connection_service_peek();
  handle_bluetooth(bluetooth_connected);


  window_stack_push(window, true);

  //Setup AppMessage
  app_message_register_inbox_received(in_rcv_handler);
  app_message_open(32, 32);
  
  sendBattery();
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
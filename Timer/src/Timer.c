#include <pebble.h>
#include "elements.h"
#include "stopwatch.h"

void draw_row_title(GContext *ctx, Layer *cell_layer, char *title) {
    GRect draw_rect = (GRect){.origin = GPoint(10, 8), .size = layer_get_bounds(cell_layer).size};

    graphics_draw_text(ctx,
                       title,
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       draw_rect,
                       GTextOverflowModeWordWrap,
                       GTextAlignmentLeft,
                       NULL);
}

//Menu Layer
static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    graphics_context_set_text_color(ctx, GColorWhite);

    switch(cell_index->row) {
        case 0:
            draw_row_title(ctx, cell_layer, "Stopwatch");
            break;

        case 1:
            draw_row_title(ctx, cell_layer, "Timer");
            break;

        case 2:
            draw_row_title(ctx, cell_layer, "Options");
            break;
    }
}

static int16_t cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 50;
}

static int16_t separator_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 0;
}
 
static uint16_t num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return 3;
}
 
static void select_click_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    if (cell_index->row == 0) {
        stopwatch_init_and_start();
    }
}


static void init(void) {
    window = window_create();
    window_set_fullscreen(window, false);
    window_set_background_color(window, GColorBlack);

    //Menu
    menu_layer = menu_layer_create(GRect(0, 1, 144, 150));

    menu_layer_set_click_config_onto_window(menu_layer, window);

    MenuLayerCallbacks callbacks = {
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) num_rows_callback,
        .get_cell_height = (MenuLayerGetCellHeightCallback) cell_height_callback,
        .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
        .get_separator_height = (MenuLayerGetSeparatorHeightCallback) separator_height_callback,
        .select_click = (MenuLayerSelectCallback) select_click_callback
    };
    menu_layer_set_callbacks(menu_layer, NULL, callbacks);

    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_layer));


    window_stack_push(window, true);
}

static void deinit(void) {
    stopwatch_deinit();
    menu_layer_destroy(menu_layer);
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

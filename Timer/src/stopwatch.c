#include <pebble.h>

static Window *window;
static TextLayer *interval_text_layer;

static int total_seconds = 0;
static bool running = false;

static char secs_text[4] = "0";
static char mins_text[4] = "0";
static char hours_text[4] = "0";

static TextLayer *secs_layer;
static TextLayer *mins_layer;
static TextLayer *hours_layer;

//remain_seconds flushed every minute
static int remain_seconds = 0;
static int minutes = 0;
static int hours = 0;

//Lap
static void stopwatch_up_click_callback(ClickRecognizerRef recognizer, void *context) {

}

//Pause
static void stopwatch_select_click_callback(ClickRecognizerRef recognizer, void *context) {
    running = !running;
}

//Restart
static void stopwatch_down_click_callback(ClickRecognizerRef recognizer, void *context) {
    total_seconds = 0;
    remain_seconds = 0;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, stopwatch_up_click_callback);
  window_single_click_subscribe(BUTTON_ID_SELECT, stopwatch_select_click_callback);
  window_single_click_subscribe(BUTTON_ID_DOWN, stopwatch_down_click_callback);
}

static void update_time(void) {
    if (running) {
        total_seconds++;
        remain_seconds++;

        if (remain_seconds == 60) {
            remain_seconds = 0;
            minutes++;

            //Every minute update minutes text
            snprintf(mins_text, sizeof(mins_text), "%d", minutes);
            text_layer_set_text(mins_layer, mins_text);

            if (layer_get_hidden(text_layer_get_layer(mins_layer)) == true) {
                layer_set_hidden(text_layer_get_layer(mins_layer), false);
                layer_set_frame(text_layer_get_layer(interval_text_layer), GRect(75, 8, 50, 72));
            }
        }
        if (minutes == 60) {
            minutes = 0;
            hours++;

            //Every hour update hours text
            snprintf(hours_text, sizeof(hours_text), "%d", hours);
            text_layer_set_text(hours_layer, hours_text);

            if (layer_get_hidden(text_layer_get_layer(hours_layer)) == true) {
                layer_set_hidden(text_layer_get_layer(hours_layer), false);
                layer_set_frame(text_layer_get_layer(interval_text_layer), GRect(75, 8, 50, 168-16));
            }
        }

        //Every second, update seconds text
        snprintf(secs_text, sizeof(secs_text), "%d", remain_seconds);
        text_layer_set_text(secs_layer, secs_text);
    }
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
    update_time();
}

static void window_disappeared(Window *window) {
    tick_timer_service_unsubscribe();
    text_layer_destroy(interval_text_layer);
    text_layer_destroy(secs_layer);
    text_layer_destroy(mins_layer);
    text_layer_destroy(hours_layer);
}

void setup_text_layer(TextLayer *text_layer, GFont font, GTextAlignment alignment) {
    text_layer_set_font(text_layer, font);
    text_layer_set_text_color(text_layer, GColorWhite);
    text_layer_set_text_alignment(text_layer, alignment);
    text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
    text_layer_set_background_color(text_layer, GColorClear);
}

void setup_number_text_layer(TextLayer *text_layer) {
    setup_text_layer(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GTextAlignmentCenter);
}

void stopwatch_init_and_start(void) {
    window = window_create();
    window_set_fullscreen(window, false);
    window_set_background_color(window, GColorBlack);
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .disappear = window_disappeared
    });

    Layer *root_layer = window_get_root_layer(window);

    //Text
    interval_text_layer = text_layer_create(GRect(75, 8, 50, 36));
    setup_text_layer(interval_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentCenter);
    text_layer_set_text(interval_text_layer, "sec\n\nmin\n\nhour");
    layer_add_child(root_layer, text_layer_get_layer(interval_text_layer));

    //Seconds
    secs_layer = text_layer_create(GRect(14, -2, 54, 168-16));
    setup_number_text_layer(secs_layer);
    text_layer_set_text(secs_layer, secs_text);
    layer_add_child(root_layer, text_layer_get_layer(secs_layer));

    //Minutes
    mins_layer = text_layer_create(GRect(14, 44, 54, 168-16));
    setup_number_text_layer(mins_layer);
    text_layer_set_text(mins_layer, mins_text);
    layer_set_hidden(text_layer_get_layer(mins_layer), true);
    layer_add_child(root_layer, text_layer_get_layer(mins_layer));

    //Hours
    hours_layer = text_layer_create(GRect(14, 92, 54, 168-16));
    setup_number_text_layer(hours_layer);
    text_layer_set_text(hours_layer, hours_text);
    layer_set_hidden(text_layer_get_layer(hours_layer), true);
    layer_add_child(root_layer, text_layer_get_layer(hours_layer));

    window_stack_push(window, true);


    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

    running = true;
}

void stopwatch_deinit(void) {
    window_destroy(window);
}
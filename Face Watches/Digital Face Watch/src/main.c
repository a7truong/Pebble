#include <pebble.h>
#include <string.h>
  
Window *s_main_window;
static Layer *s_battery_layer;
static TextLayer *s_bluetooth_layer;
static TextLayer *s_cover_layer;
static TextLayer *s_percentage_layer;
static TextLayer *s_time_hour_layer;
static TextLayer *s_time_min_layer;
static TextLayer *s_ampm_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_layer;
static TextLayer *s_line_layer1;

static int s_battery_level;
bool s_charging;

static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_bluetooth_layer, GColorGreen);
    #else
      text_layer_set_text_color(s_bluetooth_layer, GColorWhite);
    #endif
    text_layer_set_text(s_bluetooth_layer, "Connected");
  } else {
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_bluetooth_layer, GColorRed);
    #else
      text_layer_set_text_color(s_bluetooth_layer, GColorWhite);
    #endif
    text_layer_set_text(s_bluetooth_layer, "Disconnected");
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

void append(char *s, char c) {
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 60.0F) * 20.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
  
  // adds the % sign at the end for the battery level indicator
  static char percent[5];
  snprintf(percent, 5, "%d", s_battery_level);
  char per = '%';
  append(percent, per);
  if (s_battery_level >= 21) {
     #ifdef PBL_COLOR
      text_layer_set_text_color(s_percentage_layer, GColorGreen);
    #else
      text_layer_set_text_color(s_percentage_layer, GColorWhite);
    #endif
    text_layer_set_text(s_percentage_layer, percent);
  } else {
    #ifdef PBL_COLOR
      text_layer_set_text_color(s_percentage_layer, GColorRed);
    #else
      text_layer_set_text_color(s_percentage_layer, GColorWhite);
    #endif
    text_layer_set_text(s_percentage_layer, percent);
    if (s_battery_level == 20) {
      vibes_double_pulse();
    }
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  // Create a long-lived buffer
  static char clock_h[] = "00";
  static char clock_m[] = "00";
  static char day[] = "MMMMDDDDYYYY";
  static char date[] = "MMMMDDDDYYYY";
  static char ampm[] = "ss   ampm";
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == false) {
    // Use 24 hour format
    strftime(clock_h, sizeof("00"), "%H", tick_time);
    strftime(clock_m, sizeof("00"), "%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(clock_h, sizeof("00"), "%I", tick_time);
    strftime(clock_m, sizeof("00"), "%M", tick_time);
    strftime(ampm, sizeof("ampm"), "%P", tick_time);
  }
  strftime(day, sizeof("MMMMDDDDYYYY"), "%A", tick_time);
  strftime(date, sizeof("MMMMDDDDYYYY"), "%B %d", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_hour_layer, clock_h);
  text_layer_set_text(s_time_min_layer, clock_m);
  text_layer_set_text(s_ampm_layer, ampm);
  text_layer_set_text(s_day_layer, day);
  text_layer_set_text(s_date_layer, date);
}

// constantly updates time
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  s_battery_layer = layer_create(GRect(42, 0, 40, 28));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  
  s_percentage_layer = text_layer_create(GRect(0, 0, 40, 30));
  text_layer_set_background_color(s_percentage_layer, GColorBlack);
  text_layer_set_font(s_percentage_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TOP_BANNER_12)));
  text_layer_set_text_alignment(s_percentage_layer, GTextAlignmentCenter);
  
  s_cover_layer = text_layer_create(GRect(80, 15, 72, 30));
  text_layer_set_background_color(s_cover_layer, GColorBlack);
  
  s_time_hour_layer = text_layer_create(GRect(0, 15, 80, 87));
  text_layer_set_background_color(s_time_hour_layer, GColorBlack);
  text_layer_set_text_color(s_time_hour_layer, GColorWhite);
  text_layer_set_font(s_time_hour_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_51)));
  text_layer_set_text_alignment(s_time_hour_layer, GTextAlignmentRight);
  
  s_bluetooth_layer = text_layer_create(GRect(65, 0, 90, 30));
  text_layer_set_background_color(s_bluetooth_layer, GColorBlack);
  text_layer_set_font(s_bluetooth_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TOP_BANNER_12)));
  text_layer_set_text_alignment(s_bluetooth_layer, GTextAlignmentCenter);

  s_time_min_layer = text_layer_create(GRect(80, 30, 72, 72));
  text_layer_set_background_color(s_time_min_layer, GColorBlack);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_time_min_layer, GColorRed);
  #else
    text_layer_set_text_color(s_time_min_layer, GColorWhite);
  #endif
  text_layer_set_font(s_time_min_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_35)));
  text_layer_set_text_alignment(s_time_min_layer, GTextAlignmentLeft);
 
  s_ampm_layer = text_layer_create(GRect(45, 65, 145, 32));
  text_layer_set_background_color(s_ampm_layer, GColorBlack);
  text_layer_set_text_color(s_ampm_layer, GColorWhite);  
  text_layer_set_font(s_ampm_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITIAL_TEXT_24)));
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentCenter);

  s_day_layer = text_layer_create(GRect(8, 100, 144, 35));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorBlack);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  text_layer_set_font(s_day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITIAL_TEXT_24)));
  
  s_date_layer = text_layer_create(GRect(-8, 133, 144, 35));
  text_layer_set_background_color(s_date_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_date_layer, GColorRed);
  #else
    text_layer_set_text_color(s_date_layer, GColorBlack);
  #endif
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
  text_layer_set_font(s_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITIAL_TEXT_24)));
  
  s_line_layer1 = text_layer_create(GRect(12, 133, 120, 2));
  text_layer_set_background_color(s_line_layer1, GColorBlack);
  text_layer_set_text_color(s_line_layer1, GColorBlack);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_percentage_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_cover_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_hour_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_min_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_ampm_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_line_layer1));
  
  update_time();
  
  // Show current connection state
  bt_handler(bluetooth_connection_service_peek());
}

static void main_window_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_percentage_layer);
  text_layer_destroy(s_bluetooth_layer);
  text_layer_destroy(s_cover_layer);
  text_layer_destroy(s_time_hour_layer);
  text_layer_destroy(s_time_min_layer);
  text_layer_destroy(s_ampm_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
}

void deinit(void) {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

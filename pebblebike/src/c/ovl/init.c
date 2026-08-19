#include "pebble.h"
#include "../config.h"
#include "../pebblebike.h"
#include "init.h"
#include "../communication.h"
#include "../graph.h"
#include "../screens.h"
#include "../screen_config.h"
#include "../screen_data.h"
#ifdef ENABLE_FUNCTION_LIVE
  #include "../screen_live.h"
#endif
#include "../screen_map.h"
#include "../buttons.h"
#ifdef ENABLE_LOCALIZE
  #include "../localize.h"
#endif
#include "../heartrate.h"

// forward declarations
void topbar_layer_init(Window* window);
void screen_data_layer_init(Window* window);

void _ovl_init() {
  config_load();

#ifdef ENABLE_LOCALIZE
  locale_init();
#endif
  heartrate_init();
  s_gpsdata.heartrate = 255; // no data at startup
  bg_color_data_main = BG_COLOR_DATA_MAIN;

#ifdef ENABLE_NAVIGATION_FULL
  for (uint16_t i = 0; i < NAV_NB_POINTS_STORAGE - 1; i++) {
    s_gpsdata.nav_xpos[i] = s_gpsdata.nav_ypos[i] = INT16_MAX;
  }
#endif

#ifdef ENABLE_DEBUG_FIELDS_SIZE
  strcpy(s_data.speed, "188.8");
  strcpy(s_data.distance, "88.8");
  strcpy(s_data.avgspeed, "888.8");
  strcpy(s_data.altitude, "888.8");
  strcpy(s_data.ascent, "1342");
  strcpy(s_data.ascentrate, "548");
  strcpy(s_data.slope, "5");
  strcpy(s_data.accuracy, "9");
  strcpy(s_data.bearing, "270");
  strcpy(s_data.elapsedtime, "1:05:28");
  strcpy(s_data.maxspeed, "25.3");
  strcpy(s_data.heartrate, "128");
  strcpy(s_data.cadence, "90");
#endif
#ifdef ENABLE_DEMO
  strcpy(s_data.maxspeed, "26.1");
  strcpy(s_data.distance, "2.0");
  strcpy(s_data.avgspeed, "14.0");
  strcpy(s_data.altitude, "1139");
  strcpy(s_data.accuracy, "4");
  strcpy(s_data.steps, "7548");
  strcpy(s_data.elapsedtime, "1:15:28");
  strcpy(s_data.heartrate, "154");
  s_gpsdata.heartrate = 154;
  heartrate_new_data(s_gpsdata.heartrate);
  s_data.live = 1;
  s_data.state = STATE_START;

  s_gpsdata.nav_distance_to_destination100 = 12100;
  s_gpsdata.nav_next_distance1000 = 1345;
  s_gpsdata.nav_error1000 = 55;
  snprintf(s_data.nav_next_distance,   sizeof(s_data.nav_next_distance),   "%d",   s_gpsdata.nav_next_distance1000);
#else
  strcpy(s_data.speed, "0.0");
  strcpy(s_data.distance, "-");
  strcpy(s_data.avgspeed, "-");
  strcpy(s_data.altitude, "-");
  strcpy(s_data.ascent, "-");
  strcpy(s_data.ascentrate, "-");
  strcpy(s_data.slope, "-");
  strcpy(s_data.accuracy, "-");
  strcpy(s_data.bearing, "-");
  strcpy(s_data.elapsedtime, "00:00:00");
  strcpy(s_data.maxspeed, "-");
  strcpy(s_data.heartrate, "-");
  strcpy(s_data.cadence, "-");
  strcpy(s_data.nav_next_distance, "-");
  strcpy(s_data.nav_distance_to_destination, "-");
  strcpy(s_data.nav_ttd, "-");
  strcpy(s_data.nav_eta, "-");
#endif
  //strcpy(s_data.lat, "-");
  //strcpy(s_data.lon, "-");
  //strcpy(s_data.nbascent, "-");

  s_data.phone_battery_level = -1;

#ifdef PBL_HEALTH
  s_gpsdata.received_external_hr = false;
  s_gpsdata.received_internal_hr = false;
#endif

#ifdef PBL_PLATFORM_CHALK
  font_roboto_bold_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_16));
#endif
#ifdef PBL_PLATFORM_EMERY
  font_roboto_bold_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_20));
  font_roboto_bold_33 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_33));
  font_roboto_bold_38 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_38));
  font_roboto_bold_72 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_72));
#endif
  font_roboto_bold_62 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_62));


  // set default unit of measure
  change_units(UNITS_IMPERIAL, true);

  buttons_init();

  s_data.window = window_create();
  window_set_background_color(s_data.window, BG_COLOR_WINDOW);
#ifdef PBL_SDK_2
  window_set_fullscreen(s_data.window, true);
#endif
  topbar_layer_init(s_data.window);

  screen_data_layer_init(s_data.window);
  //screen_altitude_layer_init(s_data.window);
#ifdef ENABLE_FUNCTION_LIVE
  screen_live_layer_init(s_data.window);
#endif
  screen_map_init();

  #ifdef PRODUCTION
    #ifndef ENABLE_DEMO
      screen_reset_instant_data();
    #endif
  #endif

  action_bar_init(s_data.window);
  //menu_init();

  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  //app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);

  communication_init();

  screen_data_update_config(true);
  //screen_altitude_update_config();
  graph_init();

  window_stack_push(s_data.window, true /* Animated */);

  send_version(true);
}

//////////////////////////////////////////////////////////////
// from screens.c
//////////////////////////////////////////////////////////////
void topbar_layer_init(Window* window) {
//  int16_t w = SCREEN_W - MENU_WIDTH;

  s_data.topbar_layer.layer = layer_create(GRect(0, 0, SCREEN_W, TOPBAR_HEIGHT));
  layer_set_update_proc(s_data.topbar_layer.layer , topbar_layer_update_callback);
  layer_add_child(window_get_root_layer(window), s_data.topbar_layer.layer);

  // time (centered in top bar)
  s_data.topbar_layer.field_center_layer.data_layer = text_layer_create(GRect(PAGE_OFFSET_X, PBL_IF_ROUND_ELSE(6, PBL_IF_EMERY_ELSE(-11, -8)), PAGE_W, PBL_IF_ROUND_ELSE(19, PBL_IF_EMERY_ELSE(34, 25))));
  text_layer_set_background_color(s_data.topbar_layer.field_center_layer.data_layer, COLOR_TOP_BAR);
  set_layer_attr_full(s_data.topbar_layer.field_center_layer.data_layer, s_data.time, FONT_TOPBAR, GTextAlignmentCenter, COLOR_TOP_BAR, BG_COLOR_TOP_BAR, window_get_root_layer(window));

  // bluetooth icon
  s_data.topbar_layer.bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  GRect frame = gbitmap_get_bounds(s_data.topbar_layer.bluetooth_image);
#ifdef PBL_ROUND
  frame.origin.x += 52;
  frame.origin.y += 8;
#endif
  s_data.topbar_layer.bluetooth_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(s_data.topbar_layer.bluetooth_layer, s_data.topbar_layer.bluetooth_image);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_data.topbar_layer.bluetooth_layer), !bluetooth_connection_service_peek());

  // accuracy (1/3, right)
  s_data.topbar_layer.accuracy_layer = text_layer_create(GRect(PAGE_W - PBL_IF_ROUND_ELSE(18, 20) - PBL_IF_ROUND_ELSE(15, 0), PBL_IF_ROUND_ELSE(6, PBL_IF_EMERY_ELSE(-11, -8)), PBL_IF_ROUND_ELSE(18, 20), PBL_IF_ROUND_ELSE(19, PBL_IF_EMERY_ELSE(34, 25))));
  set_layer_attr_full(s_data.topbar_layer.accuracy_layer, s_data.accuracy, FONT_TOPBAR, GTextAlignmentRight, COLOR_TOP_BAR, BG_COLOR_TOP_BAR, window_get_root_layer(window));
  layer_set_hidden(text_layer_get_layer(s_data.topbar_layer.accuracy_layer), s_data.state == STATE_STOP);
}

//////////////////////////////////////////////////////////////
// from screen_data.c
//////////////////////////////////////////////////////////////
void screen_data_layer_init(Window* window) {
  config_init();

  s_data.page_data = layer_create(PAGE_GRECT);
  layer_add_child(window_get_root_layer(window), s_data.page_data);
  Layer *window_layer = window_get_root_layer(window);

  // BEGIN bottom left "distance"

  s_data.screenData_layer.field_bottom_left.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_DATA_BOTTOM_DATA_H + PBL_IF_EMERY_ELSE(27, 20), PAGE_W / 2 - 4, PBL_IF_EMERY_ELSE(24, 18)));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_left.unit_layer, s_data.unitsDistance, FONT_UNIT, PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_bottom_left.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + 1, PAGE_DATA_BOTTOM_DATA_H - PBL_IF_EMERY_ELSE(7, 5), PAGE_W / 2 - 4, PBL_IF_EMERY_ELSE(40, 30)));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_left.data_layer, s_data.distance, FONT_FIELD, PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_data);

  // END bottom left

  // BEGIN bottom right "avg"
  s_data.screenData_layer.field_bottom_right.unit_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2 + PBL_IF_ROUND_ELSE(4, 0), PAGE_DATA_BOTTOM_DATA_H + PBL_IF_EMERY_ELSE(27, 20), PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), PBL_IF_EMERY_ELSE(24, 18)));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_right.unit_layer, s_data.unitsSpeed, FONT_UNIT, PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_bottom_right.data_layer = text_layer_create(GRect(PAGE_OFFSET_X + PAGE_W / 2  + PBL_IF_ROUND_ELSE(4, 0), PAGE_DATA_BOTTOM_DATA_H - PBL_IF_EMERY_ELSE(7, 5), PAGE_W / 2 - 2*PBL_IF_ROUND_ELSE(4, 0), PBL_IF_EMERY_ELSE(40, 30)));
  set_layer_attr_full(s_data.screenData_layer.field_bottom_right.data_layer, s_data.avgspeed, FONT_FIELD, PBL_IF_ROUND_ELSE(GTextAlignmentLeft, GTextAlignmentCenter), COLOR_DATA, BG_COLOR_DATA, s_data.page_data);


  // END bottom right
  line_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(s_data.page_data, line_layer);

  // BEGIN top2 "speed"

  s_data.screenData_layer.field_top2.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(19, 3), PAGE_DATA_TOP_DATA_H - PBL_IF_EMERY_ELSE(23, 17), SCREEN_W - 2*PBL_IF_ROUND_ELSE(19, 3), PBL_IF_EMERY_ELSE(24, 18)));
  set_layer_attr_full(s_data.screenData_layer.field_top2.unit_layer, s_data.unitsSpeed, FONT_UNIT, GTextAlignmentRight, COLOR_UNITS, BG_COLOR_UNITS, s_data.page_data);

  s_data.screenData_layer.field_top2.data_layer = text_layer_create(GRect(0, PAGE_DATA_TOP_DATA_H - PBL_IF_EMERY_ELSE(57, PBL_IF_ROUND_ELSE(34, 42)), SCREEN_W, PBL_IF_EMERY_ELSE(40, 30)));
  set_layer_attr_full(s_data.screenData_layer.field_top2.data_layer, s_data.avgspeed, FONT_FIELD, GTextAlignmentCenter, COLOR_DATA, BG_COLOR_DATA, s_data.page_data);

  // END top2


  // BEGIN top "speed"

  s_data.screenData_layer.field_top.unit_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(10, 3), PAGE_DATA_MIDDLE_DATA_H + PBL_IF_EMERY_ELSE(26, 19), SCREEN_W - 2*PBL_IF_ROUND_ELSE(10, 3), PBL_IF_EMERY_ELSE(30, 22)));
  set_layer_attr_full(s_data.screenData_layer.field_top.unit_layer, s_data.unitsSpeedOrHeartRate, FONT_UNIT, GTextAlignmentRight, COLOR_DATA_UNITS, BG_COLOR_DATA_UNITS, s_data.page_data);

  s_data.screenData_layer.field_top.data_layer = text_layer_create(GRect(-20, PAGE_DATA_MIDDLE_DATA_H - PBL_IF_EMERY_ELSE(57, 42), SCREEN_W + 40, PBL_IF_EMERY_ELSE(110, 80)));
  set_layer_attr_full(s_data.screenData_layer.field_top.data_layer, s_data.speed, FONT_BIG, GTextAlignmentCenter, COLOR_DATA_DATA, BG_COLOR_DATA_DATA, s_data.page_data);

  // END top


  layer_set_hidden(s_data.page_data, false);
}

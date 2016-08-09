#include "pebble.h"
#include "config.h"
#include "pebblebike.h"
#include "communication.h"
#include "screens.h"
#include "screen_map.h"
#include "screen_live.h"
#include "screen_config.h"
#include "graph.h"
#include "heartrate.h"
#include "navigation.h"
#include "screen_data.h"

enum {
  BYTE_SETTINGS = 0,
  BYTE_ACCURACY = 1,
  BYTE_DISTANCE1 = 2,
  BYTE_DISTANCE2 = 3,
  BYTE_TIME1 = 4,
  BYTE_TIME2 = 5,
  BYTE_ALTITUDE1 = 6,
  BYTE_ALTITUDE2 = 7,
  BYTE_ASCENT1 = 8,
  BYTE_ASCENT2 = 9,
  BYTE_ASCENTRATE1 = 10,
  BYTE_ASCENTRATE2 = 11,
  BYTE_SLOPE = 12,
  BYTE_XPOS1 = 13,
  BYTE_XPOS2 = 14,
  BYTE_YPOS1 = 15,
  BYTE_YPOS2 = 16,
  BYTE_SPEED1 = 17,
  BYTE_SPEED2 = 18,
  BYTE_BEARING = 19,
  BYTE_HEARTRATE = 20,
  BYTE_MAXSPEED1 = 21,
  BYTE_MAXSPEED2 = 22,
  BYTE_CADENCE = 23,
};

enum {
  NAV_BYTE_DISTANCE1 = 0,
  NAV_BYTE_DISTANCE2 = 1,
  NAV_BYTE_DTD1 = 2,
  NAV_BYTE_DTD2 = 3,
  NAV_BYTE_BEARING = 4,
  NAV_BYTE_ERROR = 5,
  NAV_BYTE_POINTS_XPOS1 = 6,
  NAV_BYTE_POINTS_XPOS2 = 7,
  NAV_BYTE_POINTS_YPOS1 = 8,
  NAV_BYTE_POINTS_YPOS2 = 9,
};

int nb_sync_error_callback = 0;
int nb_tuple_live = 0, nb_tuple_altitude = 0, nb_tuple_state = 0;
static AppTimer *reset_data_timer;
static AppTimer *version_data_timer;

void communication_init() {
  app_message_register_inbox_received(communication_in_received_callback);
#ifdef ENABLE_DEBUG
  app_message_register_inbox_dropped(communication_in_dropped_callback);
#endif
  app_message_open(/* size_inbound */ 124, /* size_outbound */ 256);
}
void communication_deinit() {
  if (reset_data_timer) {
    app_timer_cancel(reset_data_timer);
  }
  if (version_data_timer) {
    app_timer_cancel(version_data_timer);
  }
}

void send_cmd(uint8_t cmd) {
    if (!bluetooth_connection_service_peek()) {
      vibes_double_pulse();
    }

    Tuplet value = TupletInteger(CMD_BUTTON_PRESS, cmd);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL)
        return;

    dict_write_tuplet(iter, &value);
    dict_write_end(iter);

    app_message_outbox_send();
}
static void version_data_timer_callback(void *data) {
  version_data_timer = NULL;
  send_version(false);
}
void send_version(bool first_time) {
    Tuplet tuplet_version_pebble = TupletInteger(MSG_VERSION_PEBBLE, VERSION_PEBBLE);
    Tuplet tuplet_config = TupletBytes(MSG_CONFIG, (uint8_t*) &config, sizeof(config));

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL)
        return;

    dict_write_tuplet(iter, &tuplet_version_pebble);
    dict_write_tuplet(iter, &tuplet_config);
    dict_write_end(iter);

    app_message_outbox_send();

    if (first_time) {
      // Register a timer to resend the pebble version (and receive the android version and data)
      // in some unknown conditions, first message sent by the android app is never received on the pebble side
      // (conflict with other 3rd party pebble apps?)
      version_data_timer = app_timer_register(5000, version_data_timer_callback, NULL);
    }
}

#ifdef ENABLE_DEBUG
void communication_in_dropped_callback(AppMessageResult reason, void *context) {
  // incoming message dropped

    if (reason != APP_MSG_OK) {
        //vibes_short_pulse();
        nb_sync_error_callback++;
    }

    char debug2[16];

    switch (reason) {
      case APP_MSG_OK:
        // All good, operation was successful.
        strncpy(debug2, "OK", 16);
        break;
      case APP_MSG_SEND_TIMEOUT:
        // The other end did not confirm receiving the sent data with an (n)ack in time.
        strncpy(debug2, "NOC", 16);
        break;
      case APP_MSG_SEND_REJECTED:
        // The other end rejected the sent data, with a "nack" reply.
        strncpy(debug2, "NAC", 16);
        break;
      case APP_MSG_NOT_CONNECTED:
        // The other end was not connected.
        strncpy(debug2, "NCO", 16);
        break;
      case APP_MSG_APP_NOT_RUNNING:
        // The local application was not running.
        strncpy(debug2, "NOR", 16);
        break;
      case APP_MSG_INVALID_ARGS:
        // The function was called with invalid arguments.
        strncpy(debug2, "INV", 16);
        break;
      case APP_MSG_BUSY:
        // There are pending (in or outbound) messages that need to be processed first before new ones can be received or sent.
        strncpy(debug2, "BUS", 16);
        break;
      case APP_MSG_BUFFER_OVERFLOW:
        // The buffer was too small to contain the incoming message.
        strncpy(debug2, "OVE", 16);
        break;
      case APP_MSG_ALREADY_RELEASED:
        // The resource had already been released.
        strncpy(debug2, "ALR", 16);
        break;
      case APP_MSG_CALLBACK_ALREADY_REGISTERED:
        // The callback node was already registered, or its ListNode has not been initialized.
        strncpy(debug2, "AL2", 16);
        break;
      case APP_MSG_CALLBACK_NOT_REGISTERED:
        // The callback could not be deregistered, because it had not been registered before.
        strncpy(debug2, "NOR", 16);
        break;
      case APP_MSG_OUT_OF_MEMORY:
        // The support library did not have sufficient application memory to perform the requested operation. 
        strncpy(debug2, "OUT", 16);
        break;
      case APP_MSG_CLOSED:
        // App message was closed
        strncpy(debug2, "CLO", 16);
        break;
      case APP_MSG_INTERNAL_ERROR:
        // An internal OS error prevented APP_MSG from completing an operation.
        strncpy(debug2, "INT", 16);
        break;
      case APP_MSG_INVALID_STATE:
        strncpy(debug2, "INS", 16);
        break;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "#%d\napp_msg_err:\n%d - %s",
      nb_sync_error_callback,
      reason, debug2
    );
}
#endif

static void reset_data_timer_callback(void *data) {
  reset_data_timer = NULL;

  screen_reset_instant_data();
}

#define GET_DATA(var, index) \
var = tuple->value->data[index]

#define GET_DATA_INT8(var, index) \
if (tuple->value->data[index] >= 128) { \
  var = -1 * (tuple->value->data[index] - 128); \
} else { \
  var = tuple->value->data[index]; \
}

int16_t getDataInt16(Tuple *tuple, uint8_t index) {
  if (tuple->value->data[index + 1] >= 128) {
    return -1 * (tuple->value->data[index] + 256 * (tuple->value->data[index + 1] - 128));
  } else {
    return tuple->value->data[index] + 256 * tuple->value->data[index + 1];
  }
}
#define GET_DATA_INT16(var, index) \
var = getDataInt16(tuple, index)

#define GET_DATA_UINT16(var, index) \
var = (tuple->value->data[index] + 256 * tuple->value->data[index + 1])


void communication_in_received_callback(DictionaryIterator *iter, void *context) {
    Tuple *tuple = dict_read_first(iter);
#define SIZE_OF_A_FRIEND 9
    //char friend[100];
    //int8_t live_max_name = -1;
    uint16_t time0;
#ifdef ENABLE_NAVIGATION_FULL
    static uint16_t avg5_time = 0;
    static int32_t avg5_distance100 = 0;
    static int32_t avg5_avgspeed100 = 0;
#endif
    int16_t xpos = 0, ypos = 0;

    while (tuple) {
        switch (tuple->key) {
#ifdef ENABLE_FUNCTION_LIVE
        case MSG_LIVE_NAME0:
            //vibes_short_pulse();
            strncpy(s_live.friends[0].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME1:
            strncpy(s_live.friends[1].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME2:
            strncpy(s_live.friends[2].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME3:
            strncpy(s_live.friends[3].name, tuple->value->cstring, 10);
            break;
        case MSG_LIVE_NAME4:
            strncpy(s_live.friends[4].name, tuple->value->cstring, 10);
            break;

        case MSG_LIVE_SHORT:
            nb_tuple_live++;

            GET_DATA(s_live.nb, 0);
            if (s_live.nb > NUM_LIVE_FRIENDS) {
                s_live.nb = NUM_LIVE_FRIENDS;
            }

            for (int i = 0; i < s_live.nb; i++) {
              GET_DATA_INT16(s_live.friends[i].xpos, 1 + i * SIZE_OF_A_FRIEND + 0);
              GET_DATA_INT16(s_live.friends[i].ypos, 1 + i * SIZE_OF_A_FRIEND + 2);
              GET_DATA_UINT16(s_live.friends[i].distance, 1 + i * SIZE_OF_A_FRIEND + 4) * 10; // in m
              GET_DATA(s_live.friends[i].bearing, 1 + i * SIZE_OF_A_FRIEND + 6) * 360 / 256;
              GET_DATA_UINT16(s_live.friends[i].lastviewed, 1 + i * SIZE_OF_A_FRIEND + 7); // in seconds
            }
            screen_live_menu_update();
            if (s_data.page_number == PAGE_MAP) {
                layer_mark_dirty(s_data.page_map);
            }

            break;
#endif
        case MSG_LOCATION_DATA:
        case MSG_LOCATION_DATA_V2:
        case MSG_LOCATION_DATA_V3:
            LOG_DEBUG("MSG_LOCATION_DATA %ld", tuple->key);
            nb_tuple_altitude++;
            if (tuple->key == MSG_LOCATION_DATA) {
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_LOCATION_DATA");
                change_units((tuple->value->data[BYTE_SETTINGS] & 0b00000001) >> 0, false);
                change_state((tuple->value->data[BYTE_SETTINGS] & 0b00000010) >> 1);
                s_data.debug = (tuple->value->data[BYTE_SETTINGS] & 0b00000100) >> 2;
                s_data.live = (tuple->value->data[BYTE_SETTINGS] & 0b00001000) >> 3;
                s_data.refresh_code = (tuple->value->data[BYTE_SETTINGS] & 0b00110000) >> 4;
            }
            if (tuple->key >= MSG_LOCATION_DATA_V2) {
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_LOCATION_DATA_v2");
                change_units((tuple->value->data[BYTE_SETTINGS] & 0b00000111) >> 0, false);
                change_state((tuple->value->data[BYTE_SETTINGS] & 0b00001000) >> 3);
                s_data.debug = (tuple->value->data[BYTE_SETTINGS] & 0b00010000) >> 4;
                s_data.live = (tuple->value->data[BYTE_SETTINGS] & 0b00100000) >> 5;
                s_data.refresh_code = (tuple->value->data[BYTE_SETTINGS] & 0b11000000) >> 6;
            }
            // if (tuple->key == MSG_LOCATION_DATA_V2) {
            //   APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_LOCATION_DATA_v2");
            // }
            // if (tuple->key == MSG_LOCATION_DATA_V3) {
            //   APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_LOCATION_DATA_v3");
            // }

            GET_DATA(s_gpsdata.accuracy, BYTE_ACCURACY);
            GET_DATA_UINT16(s_gpsdata.distance100, BYTE_DISTANCE1); // in 0.01km or 0.01miles
            time0 = s_gpsdata.time;
            GET_DATA_UINT16(s_gpsdata.time, BYTE_TIME1);
            if (s_gpsdata.time != 0) {
              if (s_gpsdata.units == UNITS_RUNNING_IMPERIAL || s_gpsdata.units == UNITS_RUNNING_METRIC) {
                // pace: min per mile_or_km
                if (s_gpsdata.distance100 > 0) {
                    s_gpsdata.avgspeed100 = 100 * 100 / 60 * s_gpsdata.time / s_gpsdata.distance100; // min per mile_or_km
                  } else {
                    s_gpsdata.avgspeed100 = 0;
                  }
                } else {
                  s_gpsdata.avgspeed100 = 3600 * s_gpsdata.distance100 / s_gpsdata.time; // 0.01km/h or 0.01mph
                }
#ifdef ENABLE_NAVIGATION_FULL
              if ((avg5_time == 0 && s_gpsdata.time > 0)|| s_gpsdata.time > avg5_time + 2 * 30) {
                LOG_INFO("avg5 time:%d>%d dist:%ld>%ld", avg5_time, s_gpsdata.time, avg5_distance100, s_gpsdata.distance100);
                avg5_avgspeed100 = 3600 * (s_gpsdata.distance100 - avg5_distance100) / (s_gpsdata.time - avg5_time); // 0.01km/h or 0.01mph
                avg5_distance100 = s_gpsdata.distance100;
                avg5_time = s_gpsdata.time;
              }
#endif
            } else {
                s_gpsdata.avgspeed100 = 0;
            }
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "dist=%ld, time=%d, avg=%ld", s_gpsdata.distance100, s_gpsdata.time, s_gpsdata.avgspeed100);
            GET_DATA_UINT16(s_gpsdata.speed100, BYTE_SPEED1) * 10;
            graph_add_data(&graph_speeds, s_gpsdata.speed100/10);
            GET_DATA_UINT16(s_gpsdata.maxspeed100, BYTE_MAXSPEED1) * 10;
            GET_DATA_UINT16(s_gpsdata.altitude, BYTE_ALTITUDE1);
            if (s_gpsdata.altitude != 0) {
              graph_add_data(&graph_altitudes, s_gpsdata.altitude);
            }
            GET_DATA_INT16(s_gpsdata.ascent, BYTE_ASCENT1);
            GET_DATA_INT16(s_gpsdata.ascentrate, BYTE_ASCENTRATE1);
            graph_add_data(&graph_ascentrates, s_gpsdata.ascentrate);
            GET_DATA_INT8(s_gpsdata.slope, BYTE_SLOPE)

            GET_DATA_INT16(xpos, BYTE_XPOS1);
            GET_DATA_INT16(ypos, BYTE_YPOS1);

            if ((xpos == 0 && ypos == 0) || (time0 > s_gpsdata.time)) {
                // ignore old values (can happen if gps is stopped/restarted)
                #ifdef ENABLE_DEBUG
                  if (s_data.debug) {
                    
                        APP_LOG(APP_LOG_LEVEL_DEBUG, "==> time0=%d t=%d xpos=%d ypos=%d", time0, s_gpsdata.time, xpos, ypos);      
                        //vibes_short_pulse();
                  }
                #endif
                xpos = s_gpsdata.xpos;
                ypos = s_gpsdata.ypos;
            }
            s_gpsdata.xpos = xpos;
            s_gpsdata.ypos = ypos;

            if (tuple->value->data[BYTE_BEARING] != 0) {
              GET_DATA(s_gpsdata.bearing, BYTE_BEARING) * 360 / 256;
            }
            GET_DATA(s_gpsdata.heartrate, BYTE_HEARTRATE);
            if (tuple->key >= MSG_LOCATION_DATA_V3) {
              GET_DATA(s_gpsdata.cadence, BYTE_CADENCE);
            } else {
              GET_DATA(s_gpsdata.cadence, BYTE_HEARTRATE); // no specific field until MSG_LOCATION_DATA_V3
            }


            snprintf(s_data.accuracy,   sizeof(s_data.accuracy),   "%d",   s_gpsdata.accuracy);
            snprintf(s_data.distance,   sizeof(s_data.distance),   "%ld.%ld", s_gpsdata.distance100 / 100, s_gpsdata.distance100 % 100 / 10);
            copy_speed(s_data.speed,    sizeof(s_data.speed),    s_gpsdata.speed100);
            copy_speed(s_data.avgspeed, sizeof(s_data.avgspeed), s_gpsdata.avgspeed100);
            copy_speed(s_data.maxspeed, sizeof(s_data.maxspeed), s_gpsdata.maxspeed100);
            snprintf(s_data.altitude,   sizeof(s_data.altitude),   "%u",   s_gpsdata.altitude);
            snprintf(s_data.ascent,     sizeof(s_data.ascent),     "%d",   s_gpsdata.ascent);
            snprintf(s_data.ascentrate, sizeof(s_data.ascentrate), "%d",   s_gpsdata.ascentrate);
            ///@todo(nav) snprintf(s_data.slope,      sizeof(s_data.slope),      "%d",   s_gpsdata.slope);
            snprintf(s_data.bearing,    sizeof(s_data.bearing),    "%d",   s_gpsdata.bearing);
            if (s_gpsdata.heartrate != 255) {
              snprintf(s_data.heartrate,  sizeof(s_data.heartrate),  "%d",   s_gpsdata.heartrate);
              if (s_gpsdata.heartrate > 0) {
                graph_add_data(&graph_heartrates, s_gpsdata.heartrate);
                heartrate_new_data(s_gpsdata.heartrate);
              }
            } else {
              strcpy(s_data.heartrate, "-");
            }
///@todo(nav)
//            if (s_gpsdata.cadence != 255) {
//              snprintf(s_data.cadence,  sizeof(s_data.cadence),  "%d",   s_gpsdata.cadence);
//            } else {
//              strcpy(s_data.cadence, "-");
//            }
            if (s_gpsdata.time / 3600 > 0) {
              snprintf(s_data.elapsedtime,sizeof(s_data.elapsedtime),"%d:%.2d:%.2d", s_gpsdata.time / 3600, (s_gpsdata.time / 60) % 60, s_gpsdata.time % 60);
            } else {
              snprintf(s_data.elapsedtime,sizeof(s_data.elapsedtime),"%d:%.2d", (s_gpsdata.time / 60) % 60, s_gpsdata.time % 60);
            }
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "t:%d => %s", s_gpsdata.time, s_data.elapsedtime);


            if (config_screen == CONFIG_SCREEN_DISABLED) {
              // config not in progress
              screen_data_update_config(true);

              // reset data (instant speed...) after X if no data is received
              if (reset_data_timer) {
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "app_timer_cancel()");
                app_timer_cancel(reset_data_timer);
              }
              // s_data.refresh_code == 3 => _refresh_interval [5;+inf
              reset_data_timer = app_timer_register(s_data.refresh_code == 3 ? 60000 : 20000, reset_data_timer_callback, NULL);

              screen_map_update_location();

              if (s_data.data_subpage != SUBPAGE_UNDEF) {
                  layer_mark_dirty(s_data.page_data);
              }
#ifdef ENABLE_FUNCTION_LIVE
              if (s_data.page_number == PAGE_LIVE_TRACKING) {
                  layer_mark_dirty((Layer *) s_data.page_live_tracking);
              }
#endif
            }
            break;
        case MSG_SENSOR_TEMPERATURE:
            s_gpsdata.temperature10 = tuple->value->int16;
            ///@todo(nav) snprintf(s_data.temperature,   sizeof(s_data.temperature),   "%d.%d", s_gpsdata.temperature10 / 10, s_gpsdata.temperature10 % 10);
            break;

        case STATE_CHANGED:
            nb_tuple_state++;
            //vibes_short_pulse();
            change_state(tuple->value->uint8);
            break;

        case MSG_VERSION_ANDROID:
          if (version_data_timer) {
            app_timer_cancel(version_data_timer);
          }
          s_data.android_version = tuple->value->int32;
          LOG_INFO("android_version=%ld", s_data.android_version);
          break;

        case MSG_BATTERY_LEVEL:
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG_BATTERY_LEVEL:%ld", tuple->value->int32);
          s_data.phone_battery_level = tuple->value->int32;
          break;

        case MSG_HR_MAX:
          GET_DATA(heartrate_max, 0);
          GET_DATA(heartrate_zones_notification_mode, 1);
          LOG_INFO("heartrate_max=%d mode=%d", heartrate_max, heartrate_zones_notification_mode);
          break;

        case MSG_NAVIGATION:
          GET_DATA_UINT16(s_gpsdata.nav_next_distance1000, NAV_BYTE_DISTANCE1);
          GET_DATA_UINT16(s_gpsdata.nav_distance_to_destination100, NAV_BYTE_DTD1);
          GET_DATA(s_gpsdata.nav_bearing, NAV_BYTE_BEARING) * 360 / 256;
          GET_DATA(s_gpsdata.nav_error1000, NAV_BYTE_ERROR) * 10;
          uint32_t ttd = s_gpsdata.avgspeed100 > 0 ? 3600 * s_gpsdata.nav_distance_to_destination100 / s_gpsdata.avgspeed100 : 0;
#ifdef ENABLE_NAVIGATION_FULL
          if (avg5_avgspeed100 > 0) {
            ttd = 3600 * s_gpsdata.nav_distance_to_destination100 / avg5_avgspeed100;
          }
#endif
          LOG_INFO("MSG_NAVIGATION nextd:%d dtd:%d bearing:%d err:%d", s_gpsdata.nav_next_distance1000, s_gpsdata.nav_distance_to_destination100, s_gpsdata.nav_bearing, s_gpsdata.nav_error1000);
          LOG_INFO("MSG_NAVIGATION ttd:%ld time:%d dist:%ld avg:%ld", ttd, s_gpsdata.time, s_gpsdata.distance100, s_gpsdata.avgspeed100);
          ///@todo(nav)
          snprintf(s_data.cadence,   sizeof(s_data.cadence),   "%d",   s_gpsdata.nav_next_distance1000);
          snprintf(s_data.slope,   sizeof(s_data.slope),   "%d.%d",   s_gpsdata.nav_distance_to_destination100 / 100, s_gpsdata.nav_distance_to_destination100 % 100 / 10);
          snprintf(s_data.temperature,sizeof(s_data.temperature),"%ld:%.2ld", ttd / 3600, (ttd / 60) % 60);

          for (uint8_t i = 0; i < NAV_NB_POINTS; i++) {
            GET_DATA_INT16(s_gpsdata.nav_xpos[i], NAV_BYTE_POINTS_XPOS1 + i * 4);
            GET_DATA_INT16(s_gpsdata.nav_ypos[i], NAV_BYTE_POINTS_YPOS1 + i * 4);
            //LOG_INFO("%d: xpos:%d ypos:%d", i, s_gpsdata.nav_xpos[i], s_gpsdata.nav_ypos[i]);
          }
          nav_add_data();
          break;

        }
        tuple = dict_read_next(iter);
    }
}

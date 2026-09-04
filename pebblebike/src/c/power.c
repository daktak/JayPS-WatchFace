#include "pebble.h"
#include "pebblebike.h"
#include "power.h"
#ifdef ENABLE_LOCALIZE
  #include "localize.h"
#endif

char power_zone[40];
#ifdef PBL_COLOR
GColor power_color;
GColor power_zones_color[NB_POWER_ZONES + 1];
#endif

static uint8_t zone = 1;
static time_t time_prev = 0;
static uint8_t zone_prev = 0;
static time_t zone_time_ini = 0;

uint16_t ftp = PB_IF_DEMO_ELSE(250, 0);

#ifdef ENABLE_DEMO
  uint16_t power_zones_duration[NB_POWER_ZONES + 1] = { 0, 350, 512, 2100, 344, 242, 180, 90 };
#else
  uint16_t power_zones_duration[NB_POWER_ZONES + 1] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
char power_zones_name[NB_POWER_ZONES + 1][15] = { "", "", "", "", "", "", "", ""};

void power_init() {
#ifdef PBL_COLOR
  power_zones_color[0] = BG_COLOR_DATA_MAIN;
  power_zones_color[1] = GColorLightGray;
  power_zones_color[2] = GColorIslamicGreen;
  power_zones_color[3] = GColorYellow;
  power_zones_color[4] = GColorOrange;
  power_zones_color[5] = GColorRed;
  power_zones_color[6] = GColorDarkCandyAppleRed;
  power_zones_color[7] = GColorDukeBlue;
#endif
  strcpy(power_zones_name[1], _("Active Recov"));
  strcpy(power_zones_name[2], _("Endurance"));
  strcpy(power_zones_name[3], _("Tempo"));
  strcpy(power_zones_name[4], _("Threshold"));
  strcpy(power_zones_name[5], _("VO2 Max"));
  strcpy(power_zones_name[6], _("Anaerobic"));
  strcpy(power_zones_name[7], _("Neuromuscular"));
}

uint16_t power_zones_min_w(uint8_t z) {
  if (ftp == 0) return 0;
  switch(z) {
    case 1: return 0;
    case 2: return (uint16_t)(ftp * 56 / 100);
    case 3: return (uint16_t)(ftp * 76 / 100);
    case 4: return (uint16_t)(ftp * 91 / 100);
    case 5: return (uint16_t)(ftp * 106 / 100);
    case 6: return (uint16_t)(ftp * 121 / 100);
    case 7: return (uint16_t)(ftp * 151 / 100);
    default: return 0;
  }
}

uint8_t power_zone_for(uint16_t watts) {
  if (ftp == 0) return 1;
  uint16_t pct = (uint32_t)watts * 100 / ftp;
  if (pct < 56) return 1;
  if (pct < 76) return 2;
  if (pct < 91) return 3;
  if (pct < 106) return 4;
  if (pct < 121) return 5;
  if (pct < 151) return 6;
  return 7;
}

void power_new_data(uint16_t watts) {
  if (ftp == 0 || watts == 0) return;
  zone = power_zone_for(watts);
  uint16_t delta = time_prev != 0 ? time(NULL) - time_prev : 0;
  if (zone != zone_prev) {
    power_zones_duration[zone_prev] += delta;
    zone_prev = zone;
    zone_time_ini = time(NULL);
  }
  time_prev = time(NULL);
  power_zones_duration[zone] += delta;
#ifdef PBL_COLOR
  power_color = power_zones_color[zone];
#endif
  char buf[10];
  if (power_zones_duration[zone] < 60) snprintf(buf, sizeof(buf), "%02d\"", power_zones_duration[zone]);
  else snprintf(buf, sizeof(buf), "%d'%02d\"", power_zones_duration[zone] / 60, power_zones_duration[zone] % 60);
  snprintf(power_zone, sizeof(power_zone), "%d - %s - %s", zone, power_zones_name[zone], buf);
  LOG_DEBUG("p=%d pct=%d z=%d %d/%d/%d/%d/%d/%d/%d %s", watts, (int)watts*100/ftp, zone, power_zones_duration[1], power_zones_duration[2], power_zones_duration[3], power_zones_duration[4], power_zones_duration[5], power_zones_duration[6], power_zones_duration[7], power_zone);
}

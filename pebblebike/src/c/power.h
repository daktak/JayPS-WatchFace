#ifndef POWER_H
#define POWER_H
#include <stdint.h>

#define NB_POWER_ZONES 7

void power_init();
void power_new_data(uint16_t watts);
uint16_t power_zones_min_w(uint8_t zone);
uint8_t power_zone_for(uint16_t watts);

extern uint16_t ftp;
extern char power_zone[40];
extern uint16_t power_zones_duration[NB_POWER_ZONES + 1];
extern char power_zones_name[NB_POWER_ZONES + 1][15];
#ifdef PBL_COLOR
extern GColor power_color;
extern GColor power_zones_color[NB_POWER_ZONES + 1];
#endif

#endif

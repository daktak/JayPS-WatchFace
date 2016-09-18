#ifndef SCREENS_H
#define SCREENS_H

extern ActionBarLayer *action_bar;

void update_screens();
//void field_layer_init(Layer* parent, FieldLayer* field_layer, int16_t x, int16_t y, int16_t w, int16_t h, char* title_text, char* data_text, char* unit_text);
void field_layer_deinit(FieldLayer* field_layer);
void topbar_layer_update_callback(Layer *me, GContext* ctx);
void topbar_layer_deinit();
void topbar_toggle_bluetooth_icon(bool connected);
void action_bar_init(Window* window);
void action_bar_deinit();
void set_layer_attr_full(TextLayer *textlayer, const char *text, GFont font, GTextAlignment text_alignment, GColor color, GColor bg_color, Layer *ParentLayer);
void screen_reset_instant_data();
void copy_speed(char *speed, int8_t size, int32_t speed100);
void screen_map_update_location();

#endif // SCREENS_H

#include "control_window.h"
#define STRENGTH_KEY 2

static Window *s_main_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_plus_bitmap, *s_bolt_bitmap, *s_minus_bitmap;

static DictionaryIterator *out_iter;
static char stren_str[24];
static int current_strength;
extern bool js_ready;
typedef struct ClaySettings
{
  int maxStrength;
  bool configured;
} ClaySettings;


extern ClaySettings settings;

static void update_strength()
{
  snprintf(stren_str, sizeof(stren_str), "%d%%", current_strength);
  text_layer_set_text(s_label_layer, stren_str);
}

static void up_click_handler()
{
  if (current_strength < settings.maxStrength){
    current_strength += 1;
    update_strength();
  
  }
}

static void select_click_handler()
{
  if (!js_ready)
  {
    return;
  }

  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK)
  {
    dict_write_int(out_iter, MESSAGE_KEY_shock_str, &current_strength, sizeof(int), true);
    result = app_message_outbox_send();
    if (result != APP_MSG_OK)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  }
  else
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

static void down_click_handler()
{
  if (current_strength > 0)
  {
    current_strength += -1;
    update_strength();
  }
}

static void click_config_provider(void *context)
{
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 125, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 125, down_click_handler);
}

static void window_load(Window *window)
{
  persist_read_data(STRENGTH_KEY, &current_strength, sizeof(current_strength));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const GEdgeInsets label_insets = {.top = (168 / 2) - (42 / 2) - 6, .right = ACTION_BAR_WIDTH, .left = ACTION_BAR_WIDTH / 2};
  s_label_layer = text_layer_create(grect_inset(bounds, label_insets));
  update_strength();
  text_layer_set_text_color(s_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

  s_plus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  s_bolt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BOLT);
  s_minus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUS);

  s_action_bar_layer = action_bar_layer_create();
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_plus_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_bolt_bitmap);
  action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_minus_bitmap);
  action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);
  action_bar_layer_add_to_window(s_action_bar_layer, window);
}

static void window_unload(Window *window)
{

  persist_write_data(STRENGTH_KEY, &current_strength, sizeof(current_strength));
  text_layer_destroy(s_label_layer);
  action_bar_layer_destroy(s_action_bar_layer);
  bitmap_layer_destroy(s_icon_layer);

  gbitmap_destroy(s_plus_bitmap);
  gbitmap_destroy(s_bolt_bitmap);
  gbitmap_destroy(s_minus_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

void control_window_push()
{
  if (!s_main_window)
  {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite));
    window_set_window_handlers(s_main_window, (WindowHandlers){
                                                  .load = window_load,
                                                  .unload = window_unload,
                                              });
  }
  window_stack_push(s_main_window, true);
}

#include "control_window.h"
#define STRENGTH_KEY 2
#define DURATION_KEY 3

static Window *s_main_window;
static TextLayer *s_strength_label_layer, *s_duration_label_layer;
static BitmapLayer *s_icon_layer;
static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_plus_bitmap, *s_bolt_bitmap, *s_minus_bitmap, *s_time_bitmap, *s_strength_bitmap;

static DictionaryIterator *out_iter;
static char stren_str[24];
static char dur_str[24];
static int current_strength;
static int current_duration;
extern bool js_ready;
static GFont poppins20;
typedef struct ClaySettings
{
  int maxStrength;
  int maxDuration;
  bool configured;
} ClaySettings;

extern ClaySettings settings;

typedef enum State
{
  MAIN,
  STRENGTH,
  TIME
} screenState;

static screenState state;

static void update_strength_text()
{
  snprintf(stren_str, sizeof(stren_str), "%d%%", current_strength);
  text_layer_set_text(s_strength_label_layer, stren_str);
}

static void update_duration_text()
{
  char char_time[3];
  if (current_duration >= 1000)
  {
    strcpy(char_time, "s");
    snprintf(dur_str, sizeof(dur_str), "%d.%d %s", current_duration/1000, current_duration/100 % 10, char_time);
  }
  else
  {
    strcpy(char_time, "ms");
    snprintf(dur_str, sizeof(dur_str), "%d %s", current_duration, char_time);
  };
  text_layer_set_text(s_duration_label_layer, dur_str);
}

void update_state(screenState new_state)
{
  switch (new_state)
  {
  case MAIN:
    state = new_state;
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_strength_bitmap);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_bolt_bitmap);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_time_bitmap);
    text_layer_set_text_color(s_strength_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
    text_layer_set_text_color(s_duration_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
    break;

  case STRENGTH:
    state = new_state;
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_plus_bitmap);
    action_bar_layer_clear_icon(s_action_bar_layer, BUTTON_ID_SELECT);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_minus_bitmap);
    text_layer_set_text_color(s_strength_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
    text_layer_set_text_color(s_duration_label_layer, PBL_IF_COLOR_ELSE(GColorLightGray, GColorBlack));
    break;

  case TIME:
    state = new_state;
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_plus_bitmap);
    action_bar_layer_clear_icon(s_action_bar_layer, BUTTON_ID_SELECT);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_minus_bitmap);
    text_layer_set_text_color(s_strength_label_layer, PBL_IF_COLOR_ELSE(GColorLightGray, GColorBlack));
    text_layer_set_text_color(s_duration_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
    break;

  default:
    break;
  }
}

static void up_click_handler()
{
  switch (state)
  {
  case MAIN:
    update_state(STRENGTH);
    break;
  case STRENGTH:
    if (current_strength < settings.maxStrength)
    {
      current_strength += 1;
      update_strength_text();
    };
    break;
  case TIME:
    if (current_duration < (settings.maxDuration*1000))
    {
      current_duration += 100;
      update_duration_text();
    };
    break;
  default:
    break;
  }
}

static void select_click_handler()
{
  if (state == MAIN)
  {
    if (!js_ready)
    {
      return;
    }

    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if (result == APP_MSG_OK)
    {
      dict_write_int(out_iter, MESSAGE_KEY_shock_str, &current_strength, sizeof(int), true);
      dict_write_int(out_iter, MESSAGE_KEY_shock_dur, &current_duration, sizeof(int), true);
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
}

static void down_click_handler()
{
  switch (state)
  {
  case MAIN:
    update_state(TIME);
    break;

  case STRENGTH:
    if (current_strength > 0)
    {
      current_strength += -1;
      update_strength_text();
    }
    break;

  case TIME:
    if (current_duration > 0)
    {
      current_duration += -100;
      update_duration_text();
    }
    break;

  default:
    break;
  }
}

static void back_click_handler()
{
  switch (state)
  {
  case MAIN:
  {
    window_stack_pop(true);
  }
  case STRENGTH:
  {
    update_state(MAIN);
  }
  case TIME:
  {
    update_state(MAIN);
  }
  default:
    break;
  }
}

static void click_config_provider(void *context)
{
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 125, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 125, down_click_handler);

  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void window_load(Window *window)
{
  current_duration = 300;
  current_strength = 1;
  persist_read_data(STRENGTH_KEY, &current_strength, sizeof(current_strength));
  persist_read_data(DURATION_KEY, &current_duration, sizeof(current_duration));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  poppins20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POPPINS_SEMIBOLD_FONT_24));

  const GEdgeInsets strength_label_insets = {.top = 40, .right = ACTION_BAR_WIDTH, .bottom = 0, .left = ACTION_BAR_WIDTH / 2};
  s_strength_label_layer = text_layer_create(grect_inset(bounds, strength_label_insets));
  update_strength_text();
  text_layer_set_text_color(s_strength_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_background_color(s_strength_label_layer, GColorClear);
  text_layer_set_text_alignment(s_strength_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_strength_label_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_layer, text_layer_get_layer(s_strength_label_layer));

  const GEdgeInsets duration_label_insets = {.top = (int16_t)(bounds.size.h - (30 + 40)), .right = ACTION_BAR_WIDTH, .bottom = 10, .left = ACTION_BAR_WIDTH / 2};
  s_duration_label_layer = text_layer_create(grect_inset(bounds, duration_label_insets));
  update_duration_text();
  text_layer_set_text_color(s_duration_label_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_background_color(s_duration_label_layer, GColorClear);
  text_layer_set_text_alignment(s_duration_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_duration_label_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  layer_add_child(window_layer, text_layer_get_layer(s_duration_label_layer));

  s_plus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  s_minus_bitmap = gbitmap_create_with_resource(RESOURCE_ID_MINUS);

  s_strength_bitmap = gbitmap_create_with_resource(RESOURCE_ID_STRENGTH);
  s_bolt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BOLT);
  s_time_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TIME);

  s_action_bar_layer = action_bar_layer_create();

  update_state(MAIN);

  action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);
  action_bar_layer_add_to_window(s_action_bar_layer, window);
}

static void window_unload(Window *window)
{
  persist_write_data(STRENGTH_KEY, &current_strength, sizeof(current_strength));
  persist_write_data(DURATION_KEY, &current_duration, sizeof(current_duration));

  text_layer_destroy(s_strength_label_layer);
  text_layer_destroy(s_duration_label_layer);
  action_bar_layer_destroy(s_action_bar_layer);
  bitmap_layer_destroy(s_icon_layer);

  gbitmap_destroy(s_plus_bitmap);
  gbitmap_destroy(s_bolt_bitmap);
  gbitmap_destroy(s_minus_bitmap);
  gbitmap_destroy(s_time_bitmap);
  gbitmap_destroy(s_strength_bitmap);

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

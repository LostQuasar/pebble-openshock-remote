#include <pebble.h>

#include "windows/control_window.h"
#include "windows/config_needed_window.h"
#include "windows/network_needed_window.h"

#define NUM_WINDOWS 3

static Window *s_main_window;
static MenuLayer *s_menu_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return NUM_WINDOWS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  switch (cell_index->row)
  {
  case 0:
    menu_cell_basic_draw(ctx, cell_layer, "Control UI", NULL, NULL);
    break;
  case 1:
    menu_cell_basic_draw(ctx, cell_layer, "Config Needed", NULL, NULL);
    break;
  case 2:
    menu_cell_basic_draw(ctx, cell_layer, "Network Needed", NULL, NULL);
    break;
  default:
    break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  return PBL_IF_ROUND_ELSE(
      menu_layer_is_index_selected(menu_layer, cell_index) ? MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
      44);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  switch (cell_index->row)
  {
  case 0:
    control_window_push();
    break;
  case 1:
    config_needed_window_push();
    break;
  case 2:
    network_needed_window_push();
    break;
  default:
    break;
  }
}

static void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
                                                   .get_num_rows = get_num_rows_callback,
                                                   .draw_row = draw_row_callback,
                                                   .get_cell_height = get_cell_height_callback,
                                                   .select_click = select_callback,
                                               });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window)
{
  menu_layer_destroy(s_menu_layer);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
  Tuple *shock_max_t = dict_find(iter, MESSAGE_KEY_shock_max);
  if (shock_max_t)
  {
    int shock_max = shock_max_t->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Shock Max: %d", (int)shock_max);
  }
}

static void init()
{
  const uint32_t inbox_size = 128;
  const uint32_t outbox_size = 128;
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(inbox_size, outbox_size);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){
                                                .load = window_load,
                                                .unload = window_unload,
                                            });
  window_stack_push(s_main_window, true);
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main()
{
  init();
  app_event_loop();
  deinit();
}

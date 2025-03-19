#include <pebble.h>

#include "windows/control_window.h"
#include "windows/config_needed_window.h"
#include "windows/network_needed_window.h"

#define SETTINGS_KEY 1

typedef struct ClaySettings
{
  int maxStrength;
  bool configured;
} ClaySettings;

// An instance of the struct
ClaySettings settings;
bool js_ready;
static bool connected;
// Save the settings to persistent storage
static void prv_save_settings()
{
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void prv_read_settings()
{
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// AppMessage receive handler
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context)
{

  Tuple *js_ready_t = dict_find(iter, MESSAGE_KEY_js_ready);
  if (js_ready_t){
    js_ready = true;
    return;
  }

  Tuple *shock_max_t = dict_find(iter, MESSAGE_KEY_shock_max);
  Tuple *shocker_id_t = dict_find(iter, MESSAGE_KEY_shocker_id);
  Tuple *api_key_t = dict_find(iter, MESSAGE_KEY_api_key);
  if (shock_max_t)
  {
    settings.maxStrength = shock_max_t->value->int32;
  }
  if (shocker_id_t && api_key_t)
  {
    settings.configured = true;
  }
  prv_save_settings();
}


static void init()
{
  js_ready = false;
  const uint32_t inbox_size = 128;
  const uint32_t outbox_size = 128;
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(inbox_size, outbox_size);
  prv_read_settings();
  connected = connection_service_peek_pebble_app_connection();
  if (!settings.configured)
  {
    config_needed_window_push();
  }
  else if (!connected) {
    network_needed_window_push();
  }
  else{
    control_window_push();
  }
}

static void deinit()
{
  app_message_deregister_callbacks();
}

int main()
{
  init();
  app_event_loop();
  deinit();
}

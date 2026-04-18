#include "esp_wifi.h"

// ─── WiFi init ────────────────────────────────────────────────────────────────
void wifi_init(void);

// ─── WiFi connect ─────────────────────────────────────────────────────────────
void wifi_connect(void);

// ─── WiFi event handler ───────────────────────────────────────────────────────
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);
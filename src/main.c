#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#include "gameshared.h"
#include "gpu.h"
#include "interface.h"
#include "mainLoop.h"

#include "mqtt.c"

#ifndef WIFI_SSID
#define WIFI_SSID none
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD none
#endif

/* #define MQTT_SERVER_HOST "public.mqtthq.com" */
/* #define MQTT_SERVER_PORT 8883 */

/* typedef struct MQTT_CLIENT_T_ { */
/*     ip_addr_t remote_addr; */
/*     mqtt_client_t *mqtt_client; */
/*     u8_t receiving; */
/*     u32_t received; */
/*     u32_t counter; */
/*     u32_t reconnect; */
/* } MQTT_CLIENT_T; */

/* MQTT_CLIENT_T client; */

int main() {
  stdio_init_all();

  if (cyw43_arch_init()) {
    printf("Failed while initialising cyw43 driver.");
    return -1;
  }
  cyw43_arch_enable_sta_mode();

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("Failed while connecting to wifi.\n");
    return 1;
  } else {
    printf("Connected to wifi.\n");
  }

  /* MQTT_CLIENT_T *state = mqtt_client_init(); */

  IF_Setup();
  GPU_Init();
  SetupCallbacks(&G10_Callbacks);

  DoMainLoop();
  return 0;
}

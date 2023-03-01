#include <stdio.h>
#include <stdlib.h>
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "lwip/altcp_tcp.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"

#include "gameshared.h"
#include "gpu.h"
#include "interface.h"
#include "mainLoop.h"

#ifndef WIFI_SSID
#define WIFI_SSID "none"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "none"
#endif

int main() {
  stdio_init_all();

  if (cyw43_arch_init()) {
    fprintf(stdout, "Failed while initialising cyw43 driver.");
    return -1;
  }
  cyw43_arch_enable_sta_mode();

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    fprintf(stdout, "Failed while connecting to wifi.\n");
    return 1;
  } else {
    fprintf(stdout, "Connected to wifi.\n");
  }

  IF_Setup();
  GPU_Init();

  DoMainLoop();

  cyw43_arch_deinit();
  return 0;
}

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

#include "mqtt.c"

#ifndef WIFI_SSID
#define WIFI_SSID none
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD none
#endif

#define MQTT_SERVER_HOST "public.mqtthq.com"
#define MQTT_SERVER_PORT 1883

typedef struct MQTT_CLIENT_T_ {
    ip_addr_t remote_addr;
    mqtt_client_t *mqtt_client;
    u8_t receiving;
    u32_t received;
    u32_t counter;
    u32_t reconnect;
} MQTT_CLIENT_T;

err_t mqtt_test_connect(MQTT_CLIENT_T *state);

// Perform initialisation
static MQTT_CLIENT_T* mqtt_client_init(void) {
    MQTT_CLIENT_T *state = calloc(1, sizeof(MQTT_CLIENT_T));
    if (!state) {
        fprintf(stdout, "failed to allocate state\n");
        return NULL;
    }
    state->receiving = 0;
    state->received = 0;
    return state;
}

void dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T*)callback_arg;
    fprintf(stdout, "DNS query finished with resolved addr of %s.\n", ip4addr_ntoa(ipaddr));
    state->remote_addr = *ipaddr;
}

int run_dns_lookup(MQTT_CLIENT_T *state) {
    fprintf(stdout, "Running DNS query for %s.\n", MQTT_SERVER_HOST);

    cyw43_arch_lwip_begin();
    err_t err = dns_gethostbyname(MQTT_SERVER_HOST, &(state->remote_addr), dns_found, state);
    cyw43_arch_lwip_end();

    if (err == ERR_ARG) {
        fprintf(stdout, "failed to start DNS query\n");
        return 0;
    }

    if (err == ERR_OK) {
        fprintf(stdout, "no lookup needed");
        return 1;
    }

    while (state->remote_addr.addr == 0) {
        cyw43_arch_poll();
        sleep_ms(1);
    }
    return 0;
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    /* MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg; */
    if (status != 0) {
        fprintf(stdout, "Error during connection: err %d.\n", status);
    } else {
        fprintf(stdout, "MQTT connected.\n");
    }
}

void mqtt_pub_request_cb(void *arg, err_t err) {
    MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg;
    fprintf(stdout, "mqtt_pub_request_cb: err %d\n", err);
    state->receiving = 0;
    state->received++;
}

err_t mqtt_test_publish(MQTT_CLIENT_T *state)
{
  char buffer[128];

  sprintf(buffer, "hello from picow %d / %d", state->received, state->counter);

  err_t err;
  u8_t qos = 0; /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0;
  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, "pico-to-pico", buffer, strlen(buffer), qos, retain, mqtt_pub_request_cb, state);
  cyw43_arch_lwip_end();
  if(err != ERR_OK) {
    fprintf(stdout, "Publish err: %d\n", err);
  }

  return err;
}

void mqtt_test_conn_config_cb(void *conn) {
    #if MQTT_TLS
    mbedtls_ssl_set_hostname(altcp_tls_context((struct altcp_pcb *)conn), MQTT_SERVER_HOST);
    #endif
}

err_t mqtt_test_connect(MQTT_CLIENT_T *state) {
    struct mqtt_connect_client_info_t ci;
    err_t err;

    memset(&ci, 0, sizeof(ci));

    ci.client_id = "clientId-TJNIAbBzQ69";
    ci.client_user = NULL;
    ci.client_pass = NULL;
    ci.keep_alive = 0;
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_retain = 0;
    ci.will_qos = 0;

    err = mqtt_client_connect(state->mqtt_client, &(state->remote_addr), MQTT_SERVER_PORT, mqtt_connection_cb, state, &ci);

    if (err != ERR_OK) {
        fprintf(stdout, "mqtt_connect return %d\n", err);
    }

    return err;
}

void mqtt_run_test(MQTT_CLIENT_T *state) {
    state->mqtt_client = mqtt_client_new();

    state->counter = 0;

    u32_t notReady = 5000;

    if (state->mqtt_client == NULL) {
        fprintf(stdout, "Failed to create new mqtt client\n");
        return;
    }

    if (mqtt_test_connect(state) == ERR_OK) {
        while (true) {
            cyw43_arch_poll();
            sleep_ms(1);
            if (!notReady--) {
              const int connected = mqtt_client_is_connected(state->mqtt_client);
                if (connected) {
                    cyw43_arch_lwip_begin();
                    state->receiving = 1;
                    if (mqtt_test_publish(state) == ERR_OK) {
                        fprintf(stdout, "published %d\n", state->counter);
                        state->counter++;
                    } // else ringbuffer is full and we need to wait for messages to flush.
                    cyw43_arch_lwip_end();
                } else {
                    fprintf(stdout, ".");
                }

                // MEM_STATS_DISPLAY();
                // MEMP_STATS_DISPLAY(0);
                // MEMP_STATS_DISPLAY(1);

                notReady = 5000;
            }
        }
    }
}

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

  MQTT_CLIENT_T *state = mqtt_client_init();

  run_dns_lookup(state);

  mqtt_run_test(state);

  IF_Setup();
  GPU_Init();
  SetupCallbacks(&G10_Callbacks);

  DoMainLoop();

  cyw43_arch_deinit();
  return 0;
}

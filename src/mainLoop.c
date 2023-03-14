#include "mainLoop.h"

#include "gameshared.h"
#include "gpu.h"
#include "interface.h"

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "lwip/altcp_tcp.h"
#include "lwip/apps/mqtt.h"

#include "lwip/apps/mqtt_priv.h"


#ifndef MQTT_CLIENT
#define MQTT_CLIENT "none"
#endif
#ifndef MQTT_TOPIC_IN
#define MQTT_TOPIC_IN "none"
#endif
#ifndef MQTT_TOPIC_OUT
#define MQTT_TOPIC_OUT "none"
#endif

#define MQTT_SERVER_HOST "broker.hivemq.com"
#define MQTT_SERVER_PORT 1883


static const GPU_Color _color_list[10] = {
    C_BLACK, C_BLACK, C_GRAY_DARK, C_WHITE,
    C_RED,  C_ORANGE, C_YELLOW, C_GREEN,      C_C1,   C_C2};
u8_t _selected_color_index;
GPU_Color _selected_color;


struct Draw {
  uint8_t x; // half the screen size
  uint8_t y; // half the screen size
  uint8_t c; // color index
};

#define MAX_DRAW_QUEUE 50
u8_t drawQueueI;
struct Draw drawQueue[MAX_DRAW_QUEUE];

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

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  printf("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
  printf("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */

    /* Don't trust the publisher, check zero termination */
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    for (int i = 0; i < len - sizeof(struct Draw);i+=sizeof(struct Draw) ) {
      struct Draw* draw = (struct Draw*)&data[i];
      if (draw->c == 0)
        GPU_DrawFilledSquare(_color_list[draw->c], 20, 0, GPU_X-20, GPU_Y);
      else
        GPU_DrawFilledCircle(_color_list[draw->c], ((int16_t)draw->x) * 2, ((int16_t)draw->y) * 2, 3);
    }
    if(data[len-1] == 0) {
      printf("mqtt_incoming_data_cb: %s\n", (const char *)data);
    }
  } else {
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
  printf("Subscribe result: %d\n", result);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    /* MQTT_CLIENT_T *state = (MQTT_CLIENT_T *)arg; */
    if (status != 0) {
        fprintf(stdout, "Error during connection: err %d.\n", status);
    } else {
        fprintf(stdout, "MQTT connected.\n");

  /* Setup callback for incoming publish requests */
      mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

      /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */
      mqtt_subscribe(client, MQTT_TOPIC_IN, 0, mqtt_sub_request_cb, arg);
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
  /* char buffer[128]; */

  /* sprintf(buffer, "%hd:%hd,%hd:%hd [%d/%d]", drawQueueI, drawQueue[0].x, drawQueue[0].y, state->received, state->counter); */

  err_t err;
  u8_t qos = 0; /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0;
  cyw43_arch_lwip_begin();
  err = mqtt_publish(state->mqtt_client, MQTT_TOPIC_OUT, drawQueue, sizeof(struct Draw) * drawQueueI, qos, retain, mqtt_pub_request_cb, state);
  cyw43_arch_lwip_end();
  if(err != ERR_OK) {
    fprintf(stdout, "Publish err: %d\n", err);
  } else {
    drawQueueI = 0;
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

    ci.client_id = MQTT_CLIENT;
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

static uint32_t _lastDrawTime;
static uint32_t _lastNetTime;
static uint32_t _lastPollTime;

static uint16_t lastX;
static uint16_t lastY;

void DoMainLoop() {
  MQTT_CLIENT_T *state = mqtt_client_init();
  run_dns_lookup(state);
  state->mqtt_client = mqtt_client_new();
  state->counter = 0;
  if (state->mqtt_client == NULL) {
      fprintf(stdout, "Failed to create new mqtt client\n");
      return;
  }
  if (!mqtt_test_connect(state) == ERR_OK) {
      fprintf(stdout, "Failed to create new mqtt client\n");
      return;
  }

  lastX = 0;
  lastY = 0;
  // Start queue with initial reset signal
  drawQueue[0].x = 0;
  drawQueue[0].y = 0;
  drawQueue[0].c = 0;
  drawQueueI = 1;

  mainMemory._lastRenderedTime = 0;
  mainMemory.touch_X = 0;
  mainMemory.touch_Y = 0;
  _selected_color_index = 0;
  _selected_color = _color_list[_selected_color_index];

  GPU_ClearFramebuffers();
  for (uint8_t i = 0; i < 10; i++) {
    GPU_DrawFilledSquare(_color_list[i], 0, i * 24, 20, 20);
  }
  mainMemory._lastRenderedTime = 0;

  _lastDrawTime = IF_GetCurrentTime();
  while (1) {
    sleep_ms(1);

    // Register touch events
    IF_Touchscreen();

    // Do main loop logic
    if (mainMemory.touchPressed) {
      if (mainMemory.touch_X >= 0 && mainMemory.touch_X < 24 &&
          mainMemory.touch_Y >= 0 && mainMemory.touch_Y < 240) {
        _selected_color_index = mainMemory.touch_Y / 24;
        _selected_color = _color_list[_selected_color_index];
      }
      if (mainMemory.touch_X >= 26 && mainMemory.touch_X < 360 &&
          mainMemory.touch_Y >= 0 && mainMemory.touch_Y < 240 &&
          (lastX / 2) != (mainMemory.touch_X/2) &&
          (lastY / 2) != (mainMemory.touch_Y/2)
      ) {
        if (drawQueueI < MAX_DRAW_QUEUE) {
          drawQueue[drawQueueI].x = mainMemory.touch_X / 2;
          drawQueue[drawQueueI].y = mainMemory.touch_Y / 2;
          drawQueue[drawQueueI].c = _selected_color_index;
          drawQueueI++;

          lastX = mainMemory.touch_X;
          lastY = mainMemory.touch_Y;

          if (_selected_color_index == 0)
            GPU_DrawFilledSquare(_color_list[0], 20, 0, GPU_X-20, GPU_Y);
          else
            GPU_DrawFilledCircle(_selected_color, mainMemory.touch_X, mainMemory.touch_Y, 3);
        }
      }
    }

    uint32_t new_draw_time = IF_GetCurrentTime();
    if (new_draw_time - _lastDrawTime > (ONE_SECOND / GPU_TARGET_FPS)) {
      _lastDrawTime = new_draw_time;
      GPU_Render();
    }

    uint32_t new_poll_time = IF_GetCurrentTime();
    if (new_poll_time - _lastPollTime > (ONE_SECOND / 30)) {
      _lastPollTime = new_poll_time;

      cyw43_arch_poll();
    }

    uint32_t new_net_time = IF_GetCurrentTime();
    if (new_net_time - _lastNetTime > (ONE_SECOND * 1.1) && drawQueueI > 0) {
      _lastNetTime = new_net_time;

      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
      const int connected = mqtt_client_is_connected(state->mqtt_client);
      if (connected) {
        cyw43_arch_lwip_begin();
        state->receiving = 1;
        if (mqtt_test_publish(state) == ERR_OK) {
          state->counter++;
        } // else ringbuffer is full and we need to wait for messages to flush.
        cyw43_arch_lwip_end();
      } else {
        fprintf(stdout, ".");
      }
    }
  }
}

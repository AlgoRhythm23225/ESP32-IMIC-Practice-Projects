#include "esp_http_client.h"
#include <sys/param.h> 
#include "esp_tls.h"
#include "esp_log.h"


#define MAX_HTTP_OUTPUT_BUFFER 4096
#define CONFIG_EXAMPLE_HTTP_ENDPOINT "httpforever.com"

typedef enum {
  GET,
  POST,
  PUT,
  PATCH,
  DELETE,
  HEAD
} http_state_t;

esp_err_t _http_event_handler(esp_http_client_event_t *evt);

void http_rest_with_url(http_state_t http_state);

void http_rest_with_hostname_path(void);

void http_test_task(void *pvParametters);

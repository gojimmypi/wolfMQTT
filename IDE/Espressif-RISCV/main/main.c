/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* include private my_config.h first  */
#include "my_config.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"

/* time */
#include  <lwip/apps/sntp.h>

static const char *TAG = "main";
#define TIME_ZONE "PST-8"
/* NELEMS(x) number of elements
 * To determine the number of elements in the array, we can divide the total size of
 * the array by the size of the array element
 * See https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
 **/
#define NELEMS(x)  ( (int)(sizeof(x) / sizeof((x)[0])) )
#define NTP_SERVER_LIST ( (char*[]) {                        \
                                     "pool.ntp.org",         \
                                     "time.nist.gov",        \
                                     "utcnist.colorado.edu"  \
                                     }                       \
                        )
/* #define NTP_SERVER_COUNT using NELEMS:
 *
 *  (int)(sizeof(NTP_SERVER_LIST) / sizeof(NTP_SERVER_LIST[0]))
 */
#define NTP_SERVER_COUNT NELEMS(NTP_SERVER_LIST)
char* ntpServerList[NTP_SERVER_COUNT] = NTP_SERVER_LIST;

/* our NTP server list is global info */
extern char* ntpServerList[NTP_SERVER_COUNT];

#include "awsiot.h"

int set_time(void)
{
    /* we'll also return a result code of zero */
    int res = 0;
    int i = 0; /* counter for time servers */
    time_t interim_time;

    /* ideally, we'd like to set time from network,
     * but let's set a default time, just in case */
    struct tm timeinfo = {
        .tm_year = 2022 - 1900,
        .tm_mon = 7,
        .tm_mday = 19,
        .tm_hour = 22,
        .tm_min = 46,
        .tm_sec = 10
    };
    struct timeval now;

    /* set interim static time */
    interim_time = mktime(&timeinfo);
    now = (struct timeval){ .tv_sec = interim_time };
    settimeofday(&now, NULL);

    /* set timezone */
    setenv("TZ", TIME_ZONE, 1);
    tzset();

    /* next, let's setup NTP time servers
     *
     * see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html#sntp-time-synchronization
     */
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    ESP_LOGI(TAG, "sntp_setservername:");
    for (i = 0; i < NTP_SERVER_COUNT; i++) {
        const char* thisServer = ntpServerList[i];
        if (strncmp(thisServer, "\x00", 1) == 0) {
            /* just in case we run out of NTP servers */
            break;
        }
        ESP_LOGI(TAG, "%s", thisServer);
        sntp_setservername(i, thisServer);
    }
    sntp_init();
    ESP_LOGI(TAG, "sntp_init done.");

    // static const char *TAG2 = "other";


    //esp_log_level_set("WOLFMQTT", ESP_LOG_VERBOSE);
    //ESP_LOGI("WOLFMQTT", "Hello World");
    //PRINTF("This is a test:");
    //PRINTF("This is a test: %d", 42);

   //esp_log_level_set(TAG2, ESP_LOG_VERBOSE);
   // ESP_LOGI(TAG2, "Hello World TAG2");

    return res;
}

void app_main(void)
{
    set_time();

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    main_aws();
}

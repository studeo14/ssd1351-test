
#include "ssd1306.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "touch_element/touch_button.h"

#include <stdio.h>
#define SPI_HOST SPI2_HOST
#define PIN_NUM_DC 6
#define PIN_NUM_RST 7

#define PIN_NUM_CS 5
#define PIN_NUM_MISO 35
#define PIN_NUM_MOSI 37
#define PIN_NUM_CLK 36

#define MAX_SZ 32

static const char *TAG = "SSD1351 TEST";

#define TOUCH_BUTTON 9

static touch_button_handle_t button_handle;
static const touch_pad_t button_channel = TOUCH_PAD_NUM9;
static const float channel_sense = 0.1F;

/* Button event handler task */
static void button_handler_task(void *arg)
{
    (void) arg; //Unused
    touch_elem_message_t element_message;
    while (1) {
        /* Waiting for touch element messages */
        touch_element_message_receive(&element_message, portMAX_DELAY);
        if (element_message.element_type != TOUCH_ELEM_TYPE_BUTTON) {
            continue;
        }
        /* Decode message */
        const touch_button_message_t *button_message = touch_button_get_message(&element_message);
        if (button_message->event == TOUCH_BUTTON_EVT_ON_PRESS) {
            ESP_LOGI(TAG, "Button[%ld] Press", (uint32_t)element_message.arg);
        } else if (button_message->event == TOUCH_BUTTON_EVT_ON_RELEASE) {
            ESP_LOGI(TAG, "Button[%ld] Release", (uint32_t)element_message.arg);
        } else if (button_message->event == TOUCH_BUTTON_EVT_ON_LONGPRESS) {
            ESP_LOGI(TAG, "Button[%ld] LongPress", (uint32_t)element_message.arg);
        }
    }
}

void app_main(void)
{
    uint8_t text_x = 0, text_y = 0;
    bool xmode = true, ymode = true;
    ESP_LOGI(TAG, "Start of test");
    ssd1351_128x128_spi_init(PIN_NUM_RST, PIN_NUM_CS, PIN_NUM_DC);
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_fillScreen(0x00);
    ssd1306_setMode(LCD_MODE_NORMAL);
    ssd1306_clearScreen8();
    ssd1306_setColor(RGB_COLOR8(255, 255, 0));

    touch_elem_global_config_t global_config = TOUCH_ELEM_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_element_install(&global_config));
    ESP_LOGI(TAG, "Touch element lib installed");
    touch_button_global_config_t button_global_config = TOUCH_BUTTON_GLOBAL_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(touch_button_install(&button_global_config));
    ESP_LOGI(TAG, "Touch button installed");

    touch_button_config_t button_config = {
        .channel_num = button_channel,
        .channel_sens = channel_sense
    };

    /* Create Touch buttons */
    ESP_ERROR_CHECK(touch_button_create(&button_config, &button_handle));
    /* Subscribe touch button events (On Press, On Release, On LongPress) */
    ESP_ERROR_CHECK(touch_button_subscribe_event(button_handle, TOUCH_ELEM_EVENT_ON_PRESS | TOUCH_ELEM_EVENT_ON_RELEASE | TOUCH_ELEM_EVENT_ON_LONGPRESS,
                                                    (void *)button_channel));
    /* Set EVENT as the dispatch method */
    ESP_ERROR_CHECK(touch_button_set_dispatch_method(button_handle, TOUCH_ELEM_DISP_EVENT));
    /* Set LongPress event trigger threshold time */
    ESP_ERROR_CHECK(touch_button_set_longpress(button_handle, 2000));
    /* Create a handler task to handle event messages */
    xTaskCreate(&button_handler_task, "button_handler_task", 4 * 1024, NULL, 5, NULL);
    touch_element_start();
    ESP_LOGI(TAG, "Touch buttons created");
    while(1) {
        ssd1306_printFixed8(text_x, text_y, "Hello World!", STYLE_NORMAL);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        ssd1306_printFixed8(text_x, text_y, "            ", STYLE_NORMAL);
        if (text_x >= 8*7) xmode = false;
        if (text_y >= 120) ymode = false;
        if (text_x <= 0) xmode = true;
        if (text_y <= 0) ymode = true;
        text_x += xmode ? 1: -1;
        text_y += ymode ? 1: -1;
    }
}

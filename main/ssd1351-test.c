
#include "ssd1306.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


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

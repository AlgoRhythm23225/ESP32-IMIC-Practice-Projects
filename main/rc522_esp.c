#include <stdio.h>
#include <math.h>
#include "ESP_SPI.h"
#include "rc522.h"
#include "rc522.c"
#include "driver/ledc.h"

#define TAG_ID  0x51
#define CARD_ID 0x2F

void PWM_init();
static void blink_led(int8_t led, int8_t led_state);
static void configure_led(void);

void app_main(void)
{
    configure_led();
    PWM_init();
    rfid_spi = spi_init();
    InitRc522();

    printf("--- RC522 Scanner Ready ---\n");
    uint8_t status;
    uint8_t snr[MAXRLEN]; // Mảng chứa mã ID của thẻ (UID)
    float t = 0;
    int blink_count = 0;
    uint32_t duty;

    while (1) 
    {
        // Kiểm tra xem có thẻ nào trong vùng phát sóng không
        status = PcdRequest(PICC_REQIDL, snr);

        // Read_Card_Start
        if (status == TAG_OK) 
        {
            printf("Tìm thấy thẻ! Đang đọc ID...\n");
            
            // Chống va chạm - lấy mã UID của thẻ
            status = PcdAnticoll(0x93, snr);
            
            if (status == TAG_OK) 
            {
                printf("ID của thẻ là: %02X %02X %02X %02X %02X\n", 
                        snr[0], snr[1], snr[2], snr[3], snr[4]);
            }
            blink_led(GPIO_NUM_2, 1);
        }
        else
            blink_led(GPIO_NUM_2, 0);
        // Read_Card_End

        if (snr[0] == TAG_ID) {
            // PWM_start
            float brightness = (sin(t) + 1.0) / 2.0;
            duty = brightness * 1023;
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            t += 0.025;
            // PWM_end
            
            // Blink_start
            if (blink_count >= 0 && blink_count <= 50)
                blink_led(GPIO_NUM_15, 1);
            else if (blink_count > 50 && blink_count < 100)
                blink_led(GPIO_NUM_15, 0);
                else if (blink_count >= 100)
                blink_count = 0;    
                blink_count++;    
                // Blink_end
            }
            else if (snr[0] == CARD_ID) {                
                duty = 0;
                ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
                ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
                blink_led(GPIO_NUM_15, 0);
        }         
        
        // Delay một chút để không chiếm dụng 100% CPU (nuôi Watchdog)
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void PWM_init(){
    ledc_timer_config_t timer = {
        .speed_mode         = LEDC_HIGH_SPEED_MODE,
        .timer_num          = LEDC_TIMER_0,
        .duty_resolution    = LEDC_TIMER_10_BIT,
        .freq_hz            = 5000,
        .clk_cfg            = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {
        .gpio_num   = GPIO_NUM_16,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ch);
}

static void configure_led(void)
{
    gpio_reset_pin(GPIO_NUM_15);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GPIO_NUM_2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
}

static void blink_led(int8_t led, int8_t led_state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(led, led_state);
}
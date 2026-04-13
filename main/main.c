#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/dac_oneshot.h"

const static char *TAG = "ADC_EXAMPLE";

void app_main(void)
{
    // Khởi tạo ADC unit
    adc_oneshot_unit_handle_t adc1_handle = NULL;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Cấu hình channel
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,           // 12 bits ( 0 - 4095)
        .atten = ADC_ATTEN_DB_12                    // upto ~3.3 V
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));

    // Cấu hình hiệu chuẩn
    adc_cali_handle_t cali_handle = NULL;
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    // Tạo handle hiệu chuẩn
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));

    // Dac Set up
    dac_oneshot_handle_t dac_handle;
    dac_oneshot_config_t dac_cfg = {
        .chan_id = DAC_CHAN_0
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_cfg, &dac_handle));

    while (1) {        
        int adc_raw;
        int voltage;

        // filter 
        int adc_raw_accum = 0;
        for (int i = 0; i < 10; i++) {
            int temp_raw;
            // read raw data
            ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &temp_raw));
            adc_raw_accum += temp_raw;
        }
        adc_raw = adc_raw_accum / 10;

        // Converse to mV via Calibration 
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage));

        
        // DAC
        uint8_t dac_value = (uint32_t)voltage * 255 / 3300;
        if(dac_value >= 255) {
            dac_value = 255;
        }
        ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac_handle, dac_value));

        ESP_LOGI(TAG, "Raw: %d, Voltage: %d, Dac Voltage: %d", adc_raw, voltage, dac_value);
        
        vTaskDelay(pdMS_TO_TICKS(25));

    }
}


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ESP_ADC.h"
#include "ESP_DAC.h"

const static char *TAG = "ADC_EXAMPLE";

void app_main(void)
{
    adc_init();
    dac_init();

    while (1) {   
        // adc     
        int adc_raw = adc_avg_filter_raw();
        int voltage = adc_raw_to_voltage(adc_raw);
        
        // dac
        int dac_value = voltage_to_dac(voltage);
        dac_to_output(dac_value);
 
        // print
        ESP_LOGI(TAG, "Raw: %d, Voltage: %d, Dac Voltage: %d", adc_raw, voltage, dac_value);
        
        // delay
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}


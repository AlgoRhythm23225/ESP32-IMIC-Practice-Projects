#include "ESP_ADC.h"

// Cấu hình adc
adc_oneshot_unit_handle_t adc1_handle = NULL;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE
};

// Cấu hình channel
adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,           // 12 bits ( 0 - 4095)
    .atten = ADC_ATTEN_DB_12                    // upto ~3.3 V
};

// Cấu hình hiệu chuẩn
adc_cali_handle_t cali_handle = NULL;
adc_cali_line_fitting_config_t cali_config = {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT
};

void adc_init(){
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));
}

int adc_avg_filter_raw(){
    int adc_raw;
    int adc_raw_accum = 0;
    for (int i = 0; i < 10; i++) {
        int temp_raw;
        // read raw data
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &temp_raw));
        adc_raw_accum += temp_raw;
    }
    adc_raw = adc_raw_accum / 10;

    return adc_raw;
}

int adc_raw_to_voltage(int adc_raw){
    int voltage;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage));
    
    return voltage;
}
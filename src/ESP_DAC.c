#include "ESP_DAC.h"

// DAC set up
dac_oneshot_handle_t dac_handle;
dac_oneshot_config_t dac_cfg = {
    .chan_id = DAC_CHAN_0
};

void dac_init(){
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_cfg, &dac_handle));
}

void dac_to_output(uint8_t dac_value){
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac_handle, dac_value));
}

uint8_t voltage_to_dac(int voltage){
        uint8_t dac_value = (uint32_t)voltage * 255 / 3300;
        if (dac_value >= 255) {
            dac_value = 255;
        }
        
        return dac_value;
}


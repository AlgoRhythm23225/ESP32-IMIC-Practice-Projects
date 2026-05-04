#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

extern adc_oneshot_unit_handle_t adc1_handle;
extern adc_cali_handle_t cali_handle;

void adc_init();
int adc_avg_filter_raw();
int adc_raw_to_voltage(int adc_raw);
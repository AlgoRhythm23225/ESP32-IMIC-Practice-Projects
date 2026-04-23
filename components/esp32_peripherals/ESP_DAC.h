#include "driver/dac_oneshot.h"

extern dac_oneshot_handle_t dac_handle;

void dac_init();

void dac_to_output(uint8_t dac_value);
uint8_t voltage_to_dac(int voltage);
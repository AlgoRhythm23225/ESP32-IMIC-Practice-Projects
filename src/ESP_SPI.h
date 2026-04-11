#ifndef ESP_SPI_H 
#define ESP_SPI_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

static spi_device_handle_t rfid_spi = NULL;
spi_device_handle_t spi_init();
// uint8_t rc522_read_reg(spi_device_handle_t spi, uint8_t reg);
// esp_err_t rc522_write_reg(spi_device_handle_t spi, uint8_t reg, uint8_t data);
#endif

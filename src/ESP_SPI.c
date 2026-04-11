#include "ESP_SPI.h"

spi_device_handle_t spi_init() {
    spi_bus_config_t buscfg = {
        .miso_io_num = GPIO_NUM_19,
        .mosi_io_num = GPIO_NUM_23,
        .sclk_io_num = GPIO_NUM_18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0,                          // SPI Mode 0
        .spics_io_num = GPIO_NUM_5,         // CS Pin
        .queue_size = 7,
    };

    // Cau hinh reset
    gpio_reset_pin(GPIO_NUM_22);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_22, 0);
    vTaskDelay(pdMS_TO_TICKS(50)); 
    gpio_set_level(GPIO_NUM_22, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    // spi_device_handle_t spi;
    spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    // spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
    
    spi_device_handle_t temp_handle; // Tạo biến tạm
    // spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI3_HOST, &devcfg, &temp_handle);

  
    return temp_handle;
}

// uint8_t rc522_read_reg(spi_device_handle_t spi, uint8_t reg) {
//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));

//     // RC522 Read: ((Address << 1) & 0x7E) | 0x80
//     uint8_t tx_data[2] = { ((reg << 1) & 0x7E) | 0x80, 0x00 };
//     uint8_t rx_data[2];

//     t.length = 16;
//     t.tx_buffer = tx_data;
//     t.rx_buffer = rx_data;

//     esp_err_t ret = spi_device_transmit(spi, &t);
//     if (ret != ESP_OK) return 0;

//     return rx_data[1]; // Dữ liệu trả về nằm ở byte thứ 2
// }

// esp_err_t rc522_write_reg(spi_device_handle_t spi, uint8_t reg, uint8_t data) {
//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));
    
//     // RC522 Write: (Address << 1) & 0x7E
//     uint8_t buffer[2];
//     buffer[0] = (reg << 1) & 0x7E; 
//     buffer[1] = data;

//     t.length = 16;          // 2 byte = 16 bit
//     t.tx_buffer = buffer;
    
//     return spi_device_transmit(spi, &t); 
// }
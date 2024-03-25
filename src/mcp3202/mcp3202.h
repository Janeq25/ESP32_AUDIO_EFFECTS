#pragma once

#include <stdint.h>
#include "driver/spi_master.h"


#define MCP3202_OK ESP_OK 
#define MCP3202_ERR ESP_FAIL 

typedef esp_err_t mcp3202_err_t;

typedef struct
{
    int mosi_io;
    int miso_io;
    int sclk_io;
    spi_host_device_t host; 
    int cs_io_num;  
    uint32_t clock_speed_hz;
    uint16_t reference_voltage;
    spi_device_handle_t* spi_handle;
} mcp3202_config_t;

mcp3202_err_t mcp3202_init(mcp3202_config_t* config);

mcp3202_err_t mcp3202_get_actual_freq(spi_device_handle_t spi_handle, uint32_t *frequency_hz);

mcp3202_err_t mcp3202_read_diff(spi_device_handle_t spi_handle, uint16_t *value);


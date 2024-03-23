#pragma once

#include "driver/spi_master.h"

#define MCP_ERR -1
#define MCP_OK 0

typedef int mcp3202_err;


typedef struct 
{
    int spi_clk_speed;
    int spi_mosi_io;
    int spi_miso_io;
    int spi_clk_io;
    int spi_cs_io;
    spi_host_device_t spi_host_id;
    spi_device_handle_t spi_handle;

} mcp3202_config_t;


mcp3202_err mcp3202_init(mcp3202_config_t* config);

mcp3202_err mcp3202_read_ch0(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count);

mcp3202_err mcp3202_read_ch1(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count);

mcp3202_err mcp3202_read_diff(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count);
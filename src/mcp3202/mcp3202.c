#include "mcp3202.h"

const char* TAG = "SPI";

spi_bus_config_t spi_bus_config;
spi_device_interface_config_t spi_device_config;
spi_device_handle_t spi_handle;

mcp3202_err mcp3202_init(mcp3202_config_t* config){

    int freq;

    spi_bus_config.mosi_io_num = config->spi_mosi_io;
    spi_bus_config.miso_io_num = config->spi_miso_io;
    spi_bus_config.sclk_io_num = config->spi_clk_io;
    spi_bus_config.quadwp_io_num = -1;
    spi_bus_config.quadhd_io_num = -1;
    spi_bus_config.data4_io_num = -1;
    spi_bus_config.data5_io_num = -1;
    spi_bus_config.data6_io_num = -1;
    spi_bus_config.data7_io_num = -1;
    spi_bus_config.max_transfer_sz = 3;
    spi_bus_config.flags = SPICOMMON_BUSFLAG_MASTER;
    spi_bus_config.isr_cpu_id = INTR_CPU_ID_AUTO;
    spi_bus_config.intr_flags = ESP_INTR_FLAG_LEVEL3;


    spi_device_config.command_bits = 0;
    spi_device_config.address_bits = 0;
    spi_device_config.clock_speed_hz = config->spi_clk_speed;
    spi_device_config.mode = 0;
    spi_device_config.queue_size = 1;
    spi_device_config.spics_io_num = config->spi_clk_io;
    spi_device_config.input_delay_ns = 0;
    spi_device_config.pre_cb = NULL;
    spi_device_config.post_cb = NULL;
    spi_device_config.flags = SPI_DEVICE_NO_DUMMY;

    if (spi_bus_initialize(config->spi_host_id, &spi_bus_config, 0) != ESP_OK){
        ESP_LOGI(TAG, "spi bus init error");
        return MCP_ERR;
    }

    if (spi_bus_add_device(config->spi_host_id, &spi_device_config, &spi_handle) != ESP_OK){
        ESP_LOGI(TAG, "spi device init error");
        return MCP_ERR;
    }

    if (spi_device_acquire_bus(spi_handle, portMAX_DELAY) != ESP_OK){
        ESP_LOGI(TAG, "spi acquire error");
        return MCP_ERR;
    }

    if (spi_device_get_actual_freq(spi_handle, &freq) != ESP_OK){
        ESP_LOGI(TAG, "frequency chceck error");
        return MCP_ERR;
    }

    ESP_LOGI(TAG, "set frequency of: %i", freq);


    return MCP_OK;
}

mcp3202_err mcp3202_read_ch0(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count){
    return MCP_OK;
}

mcp3202_err mcp3202_read_ch1(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count){
    return MCP_OK;
}

mcp3202_err mcp3202_read_diff(mcp3202_config_t* config, uint16_t* read_buffer, uint16_t sample_count){
    return MCP_OK;
}
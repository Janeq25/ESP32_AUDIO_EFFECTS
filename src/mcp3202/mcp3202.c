#include "mcp3202.h"


spi_device_handle_t spi_handle;

mcp3202_err_t mcp3202_init(mcp3202_config_t* config){
        spi_bus_config_t bus_cfg = {
        .mosi_io_num = config->mosi_io, // 23
        .miso_io_num = config->miso_io, // 19
        .sclk_io_num = config->sclk_io, // 18
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 3, // 24 bits.
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .isr_cpu_id = 0,
        .intr_flags = ESP_INTR_FLAG_LEVEL3};

        spi_device_interface_config_t device_cfg = {
        .command_bits = 0,
        .address_bits = 0,
        .clock_speed_hz = (int)config->clock_speed_hz,
        .mode = 0,
        .queue_size = 1,
        .spics_io_num = config->cs_io_num,
        .input_delay_ns = 0,
        .pre_cb = NULL,
        .post_cb = NULL,
        .flags = SPI_DEVICE_NO_DUMMY};

        spi_bus_initialize(config->host, &bus_cfg, 0);

        spi_bus_add_device(config->host, &device_cfg, &spi_handle);

        spi_device_acquire_bus(spi_handle, portMAX_DELAY);

    return MCP3202_OK;

}


mcp3202_err_t mcp3202_get_actual_freq(uint32_t *frequency_hz)
{
    int calculated_freq_khz;

    spi_device_get_actual_freq(spi_handle, &calculated_freq_khz);

    *frequency_hz = (uint32_t)calculated_freq_khz * 1000;

    return MCP3202_OK;
}




mcp3202_err_t mcp3202_read_diff(uint16_t *value)
{
    spi_transaction_t transaction = {
        .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
        .length = 24};


    transaction.tx_data[0] = 0b00000010;
    transaction.tx_data[1] = 0;
    transaction.tx_data[2] = 0;

    spi_device_polling_transmit(spi_handle, &transaction);

    *value = ((transaction.rx_data[1] & 15) << 8) | transaction.rx_data[2];


    return MCP3202_OK;
}


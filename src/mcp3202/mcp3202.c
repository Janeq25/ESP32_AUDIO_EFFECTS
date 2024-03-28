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
        .isr_cpu_id = INTR_CPU_ID_AUTO,
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

    // Request format (tx_data) is eight bits aligned.
    //
    // 0 0 0 0 0 1 SG/DIFF D2 _ D1 D0 X X X X X X _ X X X X X X X X
    // |--------------------|   |---------------|   |-------------|
    //
    // Where:
    //   * 0: dummy bits, must be zero.
    //   * 1: start bit.
    //   * SG/DIFF:
    //     - 0: differential conversion.
    //     - 1: single conversion.
    //   * D [0 1 2]:
    //     -  0 0 0: channel 0
    //     -  0 0 1: channel 1
    //     -  0 1 0: channel 2
    //     -  0 1 1: channel 3
    //     -  1 0 0: channel 4
    //     -  1 0 1: channel 5
    //     -  1 1 0: channel 6
    //     -  1 1 1: channel 7
    //   * X: dummy bits, any value.


    transaction.tx_data[0] = 0b00000100;
    transaction.tx_data[1] = 0;
    transaction.tx_data[2] = 0;

    spi_device_polling_transmit(spi_handle, &transaction);

    // Response format (rx_data):
    //
    // X X X X X X X X _ X X X 0 B11 B10 B9 B8 _ B7 B6 B5 B4 B3 B2 B1 B0
    // |-------------|   |-------------------|   |---------------------|
    //
    // Where:
    //   * X: dummy bits; any value.
    //   * 0: start bit.
    //   * B [0 1 2 3 4 5 6 7 8 9 10 11]: digital output code, uint16_t bits, big-endian.
    //     - B11: most significant bit.
    //     - B0: least significant bit.
    //
    // More information on section "6.1 Using the MCP3204/3208 with Microcontroller (MCU) SPI Ports"
    // of the mcp3202 datasheet.
    //
    // Result logic, taking the following sequence as example:
    //
    // 0 1 0 0 0 1 0 0 _ 1 0 1 0 0 1 0 0 _ 1 1 1 1 0 1 1 0 = 1270
    // |--- rx[0] ---|   |--- rx[1] ---|   |--- rx[2] ---|
    //
    // 1) As bits 5-7 from rx_data[1] can be any value (bit 4 is always zero),
    //    AND it with 15 (00001111) to zero them.
    //    > first_part  = 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0
    //    > second_part = 0 0 0 0 0 0 0 0 1 1 1 1 0 1 1 0
    //
    // 2) Move rx_data[1] value 8 bits to the left to open space for second_part.
    //    > first_part  = 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0
    //    > second_part = 0 0 0 0 0 0 0 0 1 1 1 1 0 1 1 0
    //
    // 3) Concat (ORing) first_part with second_part.
    //    > first_part  = 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0
    //    > second_part = 0 0 0 0 0 0 0 0 1 1 1 1 0 1 1 0
    //    > result      = 0 0 0 0 0 1 0 0 1 1 1 1 0 1 1 0

    *value = ((transaction.rx_data[1] & 15) << 8) | transaction.rx_data[2];

    return MCP3202_OK;
}


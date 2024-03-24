#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_timer.h"

#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"

#include "mcp320x/mcp320x.h"
//#include "mcp3202/mcp3202.h"

#include <math.h>

#define SAMPLERATE 44100
#define SAMPLEBLOCK 512
#define RANGE (2^16)

#define MAXDELAYLENGTH (1 * SAMPLERATE)

mcp320x_t *mcp320x_handle = NULL;
uint16_t voltage;

int16_t samples_in[SAMPLEBLOCK];
int16_t samples_out[SAMPLEBLOCK];
int pot1 =  0;
int sw1 = 0;


void inputs_setup(){
    gpio_set_pull_mode(GPIO_NUM_11, GPIO_PULLUP_ENABLE);
    gpio_set_direction(GPIO_NUM_11, GPIO_MODE_INPUT);

}

void adc_setup (){

    uint32_t freq;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_NUM_13, // 23
        .miso_io_num = GPIO_NUM_12, // 19
        .sclk_io_num = GPIO_NUM_14, // 18
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

    mcp320x_config_t mcp320x_cfg = {
        .host = SPI3_HOST,
        .device_model = MCP3204_MODEL,
        .clock_speed_hz = 2000000,
        .reference_voltage = 5000,         // 5V
        .cs_io_num = GPIO_NUM_15}; // 5

    // Bus initialization is up to the developer.
    ESP_ERROR_CHECK(spi_bus_initialize(mcp320x_cfg.host, &bus_cfg, 0));

    // Add the device to the SPI bus.
    mcp320x_handle = mcp320x_install(&mcp320x_cfg);

    // Occupy the SPI bus for multiple transactions.
    ESP_ERROR_CHECK(mcp320x_acquire(mcp320x_handle, portMAX_DELAY));

    mcp320x_get_actual_freq(mcp320x_handle, &freq);
    ESP_LOGI("ADC SETUP:", "freq = %li", freq);

}


void dac_setup(){
        i2s_config_t i2s_dac_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLERATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .dma_buf_count = 8,
        .dma_buf_len = SAMPLEBLOCK,
        .use_apll = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t dacI2SPins = {
        .mck_io_num = GPIO_NUM_0,
        .bck_io_num = GPIO_NUM_4,
        .ws_io_num = GPIO_NUM_5,
        .data_out_num = GPIO_NUM_18,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_1, &i2s_dac_config, 0, NULL);
    i2s_set_pin(I2S_NUM_1, &dacI2SPins);

}

void task_read(){
    while (1){

    for (size_t i = 0; i < SAMPLEBLOCK; i++)
    {
        // Read voltage, sampling 1000 times.
        ESP_ERROR_CHECK(mcp320x_read(mcp320x_handle,
                             MCP320X_CHANNEL_0,
                             MCP320X_READ_MODE_SINGLE,
                             1,
                             &voltage));

        samples_in[i] = (int16_t)voltage;

        //ESP_LOGI("mcp320x", "Voltage: %d mV", voltage);
    }

    

    }
}


int16_t delay_buffer[MAXDELAYLENGTH];
int32_t delay_index = 0;
size_t delay_length = SAMPLERATE * 0.5;
float decay = 0.5;

// float time;
// int16_t sine(){
//     time += (1/SAMPLERATE);
//     return (int16_t)sin(2*3.14*time);
// }


void task_write(){



    while(1){


        for (int i = 0; i < SAMPLEBLOCK; i+=1){
            samples_out[i] = samples_in[i];
            //printf("%d\n", samples_out[i]);
        }

        // for (int i = 0; i < SAMPLEBLOCK; i+=1){
        //     samples_out[i] = sine();
        //     printf("%d\n", samples_out[i]);
        // }
            


            // int32_t delayed_index = delay_index-delay_length;
            // if (delayed_index <= 0) delayed_index = delayed_index + MAXDELAYLENGTH;

            // //printf("delay_index: %li, delayed_index: %li\n", delay_index, delayed_index);

            // delay_buffer[delay_index] = ((lowpass(samples_in[i])-2048)*8) + (int16_t)(decay * delay_buffer[delay_index]);

            // samples_out[i] = delay_buffer[delayed_index];

            // delay_index++;


        // }

        size_t bytes_written = 0;
        i2s_write(I2S_NUM_1, (void *)samples_out, sizeof(samples_out), &bytes_written, portMAX_DELAY);
    }
}


void measure_important_function(void) {
    const unsigned MEASUREMENTS = 1;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {
        for (size_t i = 0; i < SAMPLEBLOCK; i++)
        {
            // Read voltage, sampling 1000 times.
            ESP_ERROR_CHECK(mcp320x_read(mcp320x_handle,
                                    MCP320X_CHANNEL_0,
                                    MCP320X_READ_MODE_SINGLE,
                                    1,
                                    &voltage));

            samples_in[i] = (int16_t)voltage;

            //ESP_LOGI("mcp320x", "Voltage: %d mV", voltage);
        }
    }

    uint64_t end = esp_timer_get_time();

    for (int i = 0; i < SAMPLEBLOCK; i++){
        printf("%i\n", samples_in[i]);
    }

    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}



TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

void app_main(){
    dac_setup();
    adc_setup();

    measure_important_function();

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 1, task_read_handle, 1);
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 1, task_write_handle, 0);

    // int rate = SAMPLERATE + 10000;

    // while(1){
    //     //printf("pot1: %i, sw1: %i\n", pot1, sw1);
        


    //     // i2s_set_sample_rates(I2S_NUM_1, RATE_OFFSET+(pot1*10));
    //     // printf("ste samplerate of: %i\n", RATE_OFFSET+pot1);
    //     vTaskDelay(pdMS_TO_TICKS(100));

    //     if (pot1 > 70){
    //         rate++;
    //     }
    //     else if(pot1 < 20){
    //         rate--;
    //     }

    //     i2s_set_sample_rates(I2S_NUM_0, rate);
    //     if (sw1 == 1) printf("rate: %i\n", rate);
    // }

    
}
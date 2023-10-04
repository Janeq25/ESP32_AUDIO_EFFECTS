

#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"

#define SAMPLERATE 48000
#define SAMPLEBLOCK 1024

int16_t samples_in[SAMPLEBLOCK];
int16_t samples_out[SAMPLEBLOCK];


void adc_setup (){


    i2s_config_t i2s_adc_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
        .sample_rate = SAMPLERATE+12000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .dma_buf_count = 4,
        .dma_buf_len = SAMPLEBLOCK,
        .use_apll = false,
        .fixed_mclk = 0
    };

    i2s_driver_install(I2S_NUM_0, &i2s_adc_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_0);

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
        .dma_buf_count = 4,
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

// void task_read(){
//     size_t bytes_read = 0;
//     int num_bytes_read = i2s_read(I2S_NUM_0, (void *) samples_in, sizeof(samples_in), &bytes_read, portMAX_DELAY);
// }

TaskHandle_t task_read_handle = NULL;

void app_main(){
    dac_setup();
    adc_setup();


    while(1){

        size_t bytes_written = 0;
        size_t bytes_read = 0;
        int num_bytes_read = i2s_read(I2S_NUM_0, (void *) samples_in, sizeof(samples_in), &bytes_read, portMAX_DELAY);

        for (uint16_t i = 0; i < SAMPLEBLOCK; i+=1){
            //printf("%i\n", samples_in[i]);
            samples_out[i] = (samples_in[i]-2048)*8;

            //printf("%li\n", samples_out[i]);
        }

        i2s_write(I2S_NUM_1, (void *)samples_out, sizeof(samples_out), &bytes_written, portMAX_DELAY);
    }
}
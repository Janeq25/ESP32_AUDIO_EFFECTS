#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"

#define SAMPLERATE 48000
#define SAMPLEBLOCK 1024

#define MAXDELAYLENGTH (1 * SAMPLERATE)

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

void task_read(){
    while (1){
        size_t bytes_read = 0;
        int num_bytes_read = i2s_read(I2S_NUM_0, (void *) samples_in, sizeof(samples_in), &bytes_read, portMAX_DELAY);
    }
}


int16_t delay_buffer[MAXDELAYLENGTH];
int32_t delay_index = 0;
size_t delay_length = SAMPLERATE * 0.5;
float decay = 0.5;

void task_write(){



    while(1){

        for (size_t i = 0; i < SAMPLEBLOCK; i+=1){

            int32_t delayed_index = delay_index-delay_length;
            if (delayed_index <= 0) delayed_index = delayed_index + MAXDELAYLENGTH;

            //printf("delay_index: %li, delayed_index: %li\n", delay_index, delayed_index);

            delay_buffer[delay_index] = ((samples_in[i]-2048)*8) + (int16_t)(decay * delay_buffer[delay_index]);

            samples_out[i] = delay_buffer[delayed_index];

            delay_index++;
            if (delay_index >= MAXDELAYLENGTH) delay_index = 0;

        }

        size_t bytes_written = 0;
        i2s_write(I2S_NUM_1, (void *)samples_out, sizeof(samples_out), &bytes_written, portMAX_DELAY);
    }
}

TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

void app_main(){
    dac_setup();
    adc_setup();

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 1, task_read_handle, 1);
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 1, task_write_handle, 0);

    
}
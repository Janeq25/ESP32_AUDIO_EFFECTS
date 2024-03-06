#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include "driver/i2s.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"

#define SAMPLERATE 44100
#define SAMPLEBLOCK 512
#define RATE_OFFSET 10000
#define RANGE (2^16)

#define MAXDELAYLENGTH (1 * SAMPLERATE)

int16_t samples_in[SAMPLEBLOCK*2];
int16_t samples_out[SAMPLEBLOCK];
int pot1 =  0;
int sw1 = 0;


void inputs_setup(){
    gpio_set_pull_mode(GPIO_NUM_11, GPIO_PULLUP_ENABLE);
    gpio_set_direction(GPIO_NUM_11, GPIO_MODE_INPUT);

}

void adc_setup (){


    i2s_config_t i2s_adc_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
        .sample_rate = (SAMPLERATE+RATE_OFFSET)*2,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .dma_buf_count = 8,
        .dma_buf_len = SAMPLEBLOCK*2,
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
        size_t bytes_read = 0;
        int num_bytes_read = i2s_read(I2S_NUM_0, (void *) samples_in, sizeof(samples_in), &bytes_read, portMAX_DELAY);

        uint64_t sum;
        int reading;

        // for (int i = 0; i < 5; i++){
        //     adc2_get_raw(ADC2_CHANNEL_8, ADC_BITWIDTH_10, &reading);
        //     sum += reading;
        // }
        // sum = (sum*100)/(5*1023);
        // pot1 = (int)sum;
        // sw1 = gpio_get_level(GPIO_NUM_11);
    }
}


int16_t delay_buffer[MAXDELAYLENGTH];
int32_t delay_index = 0;
size_t delay_length = SAMPLERATE * 0.5;
float decay = 0.5;
int16_t last_sample=0;


int16_t lowpass(int16_t input)
{
   int16_t retvalue=(input + (last_sample * 7)) >> 3;
   last_sample=retvalue;
   return retvalue;
}



void task_write(){



    while(1){


        for (int i = 0; i < SAMPLEBLOCK; i+=1){
            samples_out[i] = lowpass((samples_in[i*2]+samples_in[(i*2)+1])/2)*100;
            
        }

            


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

TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

void app_main(){
    dac_setup();
    adc_setup();

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
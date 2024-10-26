#include "esp_log.h"                                // logging library

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include "math.h"
#include "esp_task_wdt.h"

#include "defines.h"                                // project settings
#include "globals.h"                                // global variable allocations
#include "setup.h"                                  // peripheral setup functions
#include "util.h"                                   // utility functions


uint64_t ms_elapsed = 0;

int16_t generate_sine(float frequency){
    if (frequency == 0) return 0;
    int16_t out = (int16_t)(2048*sin(2*M_PI*frequency*((float)ms_elapsed/10000)));
    ms_elapsed += 1;
    return out;
}


void task_read(){                                   // task responsible for sample aquisition 

    user_inputs_setup();
                
    adc_setup();                                    // spi peripherial setup for communication with mcp3202 adc
                
    measure_read_speed();                           // get acquisition time of one sample
                
    timer_setup();                                  // timer and timer interrupt setup for acquisition with desired samplerate
                
    while (1){                                      // as the acquisition is being handled inside an interrupt, in time betwen samples user inputs (knobs position) can be acquired
        int sum1 = 0;
        int sum2 = 0;
        for (int i = 0; i < 10; i++){
            sum1 += adc1_get_raw(ADC1_CHANNEL_3);
            sum2 += adc1_get_raw(ADC1_CHANNEL_7);
        }

        pot1 = (float)sum1/40950;
        pot2 = (float)sum2/40950;

        pot1 = roundf(pot1*100)/100;
        pot2 = roundf(pot2*100)/100;

        //esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    while(1){
        size_t bytes_written = CHUNK_SIZE;
        // int32_t samples2;


        for (int i = 0; i < CHUNK_SIZE; i++){       // get chunk form queue and apply effect
            int16_t sample;


            xQueueReceive(queue, &sample, 1);
            samples[i] = (int32_t)(sample << 22);

            // printf("%i\n", sample);
            // samples[i] = generate_sine(2000);
        }


        // for (int i = 0; i < CHUNK_SIZE; i+=8){
        //     samples[i] = 32768;
        //     samples[i+1] = 55929;
        //     samples[i+2] = 65535;
        //     samples[i+3] = 55966;
        //     samples[i+4] = 32820;
        //     samples[i+5] = 9644;
        //     samples[i+6] = 1;
        //     samples[i+7] = 9534;
            
        // }




        gpio_set_level(DEBUG_PIN2, 1);

        i2s_write(I2S_NUM_1, (void *)samples, sizeof(samples), &bytes_written, portMAX_DELAY); //send prepared chunk to dac

        gpio_set_level(DEBUG_PIN2, 0);

        // printf(">sin:%f\n", sin((float)esp_timer_get_time()/1000000));

        // vTaskDelay(pdMS_TO_TICKS(64));

    }
}



void app_main(){

    queue = xQueueCreate(CHUNK_SIZE*QUEUE_SIZE, sizeof(int16_t));                               // rtos queue used for buffering acquired samples before applying effects



    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 2, task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 2, task_write_handle, 0);

}
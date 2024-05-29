#include "esp_log.h"                                // logging library

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include "math.h"
#include "esp_timer.h"

#include "defines.h"                                // project settings
#include "globals.h"                                // global variable allocations
#include "setup.h"                                  // peripheral setup functions
#include "util.h"                                   // utility functions


int16_t generate_sine(float frequency){
    return (int16_t)(2048*sin(2*M_PI*frequency*((float)esp_timer_get_time()/1000000)));
}


void task_read(){                                   // task responsible for sample aquisition 
                
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


        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    while(1){

        for (int i = 0; i < CHUNK_SIZE; i++){       // get chunk form queue and apply effect
            int16_t sample;

            if(pot2 > 0.5){
                xQueueReceive(queue, &sample, pdMS_TO_TICKS(100));
                samples[i] = sample;
            }
            else{
                samples[i] = generate_sine(1000*pot1);
            }
            // printf(">sin:%i\n", samples[i]);
            // printf("pot1:%f, pot2:%f\n", pot1, pot2);
        }

        


        size_t bytes_written = 0;
        i2s_write(I2S_NUM_1, (void *)samples, sizeof(samples), &bytes_written, portMAX_DELAY); //send prepared chunk to dac

        // printf(">sin:%f\n", sin((float)esp_timer_get_time()/1000000));
    }
}



void app_main(){

    queue = xQueueCreate(CHUNK_SIZE*QUEUE_SIZE, sizeof(int16_t));                               // rtos queue used for buffering acquired samples before applying effects

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 1, task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 1, task_write_handle, 0);

}
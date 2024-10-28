#include "esp_log.h"                                // logging library

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include "math.h"
#include "esp_task_wdt.h"

#include "defines.h"                                // project settings
#include "globals.h"                                // global variable allocations
#include "setup.h"                                  // peripheral setup functions
#include "util.h"                                   // utility functions
#include "effects.h"



void task_read(){                                   // task responsible for sample aquisition 

    user_inputs_setup();
                
    adc_setup();                                    // spi peripherial setup for communication with mcp3202 adc
                
    measure_read_speed();                           // get acquisition time of one sample
                
    timer_setup();                                  // timer and timer interrupt setup for acquisition with desired samplerate
                
    while (1){                                      // as the acquisition is being handled inside an interrupt, in time betwen samples user inputs (knobs position) can be acquired

        // pot1 = roundf(adc1_get_raw(ADC1_CHANNEL_3)/40.95)/100;
        pot2 = (roundf(adc1_get_raw(ADC1_CHANNEL_6)/40.95)/100);
        pot3 = (roundf(adc1_get_raw(ADC1_CHANNEL_7)/40.95)/100);


        pot1 = adc1_get_raw(ADC1_CHANNEL_3) >> 5;
        // pot2 = adc1_get_raw(ADC1_CHANNEL_6) >> 5;
        // pot3 = adc1_get_raw(ADC1_CHANNEL_7) >> 5;


        vTaskDelay(pdMS_TO_TICKS(100));

    }
}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    while(1){

        retrive_chunk(samples);


        for (int i = 0; i < CHUNK_SIZE; i++){


            switch (pot1/(128/EFFECT_NUM))
            {
            case 0:
                samples[i] = overdrive(samples[i]);
                break;

            case 1:
                samples[i] = delay(samples[i]);
                break;
            
            default:
                break;
            }

        }

        output_chunk(samples);

        // for (int i = 0; i < CHUNK_SIZE; i++){
        //     samples[i] = overdrive(samples[i]);
        // }

        // printf("pot1: %i, pot2: %f, pot3: %f\n", pot1, pot2, pot3);


        // for (int i = 0; i < CHUNK_SIZE; i+=8){ //24 bit sine wave
        //     samples[i]   = 8388608;
        //     samples[i+1] = 14317887;
        //     samples[i+2] = 16777213;
        //     samples[i+3] = 14327330;
        //     samples[i+4] = 8401968;
        //     samples[i+5] = 2468787;
        //     samples[i+6] = 24;
        //     samples[i+7] = 2440458;
        // }


        // gpio_set_level(DEBUG_PIN2, 1);
        // output_chunk(samples);
        // gpio_set_level(DEBUG_PIN2, 0);


    }
}



void app_main(){
    ringbuffer_handle = xRingbufferCreate(CHUNK_SIZE, RINGBUF_TYPE_NOSPLIT);

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 2, &task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 2, &task_write_handle, 0);

}
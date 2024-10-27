#include "esp_log.h"                                // logging library

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include "math.h"
#include "esp_task_wdt.h"

#include "defines.h"                                // project settings
#include "globals.h"                                // global variable allocations
#include "setup.h"                                  // peripheral setup functions
#include "util.h"                                   // utility functions

#define POW11 2048
#define POW21 2097152

void task_read(){                                   // task responsible for sample aquisition 

    user_inputs_setup();
                
    adc_setup();                                    // spi peripherial setup for communication with mcp3202 adc
                
    measure_read_speed();                           // get acquisition time of one sample
                
    timer_setup();                                  // timer and timer interrupt setup for acquisition with desired samplerate
                
    while (1){                                      // as the acquisition is being handled inside an interrupt, in time betwen samples user inputs (knobs position) can be acquired
    //     int sum1 = 0;
    //     int sum2 = 0;
    //     for (int i = 0; i < 10; i++){
    //         sum1 += adc1_get_raw(ADC1_CHANNEL_3);
    //         sum2 += adc1_get_raw(ADC1_CHANNEL_7);
    //     }

    //     pot1 = (float)sum1/40950;
    //     pot2 = (float)sum2/40950;

    //     pot1 = roundf(pot1*100)/100;
    //     pot2 = roundf(pot2*100)/100;

    }
    // vTaskDelay(pdMS_TO_TICKS(10));
}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    while(1){
        size_t bytes_written = CHUNK_SIZE;

        // size_t item_size;
        // int16_t* item = (int16_t *)xRingbufferReceiveUpTo(ringbuffer_handle, &item_size, pdMS_TO_TICKS(1000), CHUNK_SIZE);

        // if (item == NULL) printf("failed");

        // vRingbufferReturnItem(ringbuffer_handle, (void *)item);


        // int32_t samples2;


        for (int i = 0; i < CHUNK_SIZE; i++){       // get chunk form queue and apply effect?


        // Receive an item from no-split ring buffer
            size_t item_size;
            int16_t* item = (int16_t* )xRingbufferReceive(ringbuffer_handle, &item_size, pdMS_TO_TICKS(1000));

            // samples[i] = ((*item - HALF) << 24);
            samples[i] = *item;

            vRingbufferReturnItem(ringbuffer_handle, (void *)item);
        }

        for (int i = 0; i < CHUNK_SIZE; i++) {
            if (samples[i] >= POW11 + 2000) {
                samples[i] = POW11 + 2000;
            }
            else if (samples[i] <= POW11 - 2000){
                samples[i] = POW11 - 2000;
            }
            else {
                samples[i] = samples[i];
            }

            samples[i] = ((samples[i] - POW11) << 12) + POW21;

        }


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



        // for (int i = 0; i < CHUNK_SIZE; i++)
        // {
        //     samples[i] = samples[i] << 19;
        //     // printf("%lu\n", samples[i]);
        // }
    


        gpio_set_level(DEBUG_PIN2, 1);

        i2s_write(I2S_NUM_1, (void *)samples, sizeof(samples), &bytes_written, portMAX_DELAY); //send prepared chunk to dac
        // i2s_write(I2S_NUM_1, (void *)item, sizeof(item), &bytes_written, portMAX_DELAY); //send prepared chunk to dac

        gpio_set_level(DEBUG_PIN2, 0);

        // printf(">sin:%f\n", sin((float)esp_timer_get_time()/1000000));

        // vTaskDelay(pdMS_TO_TICKS(64));

    }
}



void app_main(){

    // queue = xQueueCreate(CHUNK_SIZE*QUEUE_SIZE, sizeof(int16_t));                               // rtos queue used for buffering acquired samples before applying effects

    ringbuffer_handle = xRingbufferCreate(CHUNK_SIZE, RINGBUF_TYPE_NOSPLIT);


    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 2, task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 2, task_write_handle, 0);

}
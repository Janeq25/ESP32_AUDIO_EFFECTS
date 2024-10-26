#include "esp_log.h"                                // logging library

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

#include "math.h"
// #include ""

#include "defines.h"                                // project settings
#include "globals.h"                                // global variable allocations
#include "setup.h"                                  // peripheral setup functions
#include "util.h"                                   // utility functions



RingbufHandle_t ringbuffer;





void task_read(){                                   // task responsible for sample aquisition 

    user_inputs_setup();
                
    adc_setup();                                    // spi peripherial setup for communication with mcp3202 adc
                
    measure_read_speed();                           // get acquisition time of one sample
                
    timer_setup();                                  // timer and timer interrupt setup for acquisition with desired samplerate
                
    while (1){                                      // as the acquisition is being handled inside an interrupt, in time betwen samples user inputs (knobs position) can be acquired
       
       int16_t set = 1000 + (10000 * pot1);
       
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

        // vTaskDelay(pdMS_TO_TICKS(10));

        // printf("%i, %i\n", data_in_sample_ptr, chunk_ready_flag);

        if (chunk_ready_flag == 1) {

            if (data_in_buffer_ptr == 0) {
                xRingbufferSend(ringbuffer, data_in2, sizeof(data_in1), pdMS_TO_TICKS(10));
            }
            else {
                xRingbufferSend(ringbuffer, data_in1, sizeof(data_in1), pdMS_TO_TICKS(10));

            }

            chunk_ready_flag = 0;
        }

        // printf("%i, %f\n", set, cycles);


    }


}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    while(1){
        size_t bytes_written = CHUNK_SIZE;
        int16_t set = 1000 + (20000 * pot1);


        size_t item_size;
        uint16_t *item = (uint16_t *)xRingbufferReceive(ringbuffer, &item_size, portMAX_DELAY);

        vRingbufferReturnItem(ringbuffer, (void *)item);

        /* Assign w_buf */
        // for (int i = 0; i < CHUNK_SIZE; i += 8) {
        //     samples[i]     = 0x12;
        //     samples[i + 1] = 0x34;
        //     samples[i + 2] = 0x56;
        //     samples[i + 3] = 0x78;
        //     samples[i + 4] = 0x9A;
        //     samples[i + 5] = 0xBC;
        //     samples[i + 6] = 0xDE;
        //     samples[i + 7] = 0xF0;
        // }


        for (int i = 0; i < CHUNK_SIZE; i++){       // get chunk form queue and apply effect
        //     // int16_t sample;


        //     // xQueueReceive(queue, &sample, 1);

            printf("%i\n", (int16_t)item[i]);



        //     // vTaskDelay(pdMS_TO_TICKS(1));

        }

        // vTaskDelay(1);
        // printf("%i\n", set);

        

        // size_t bytes_written = CHUNK_SIZE;
        // i2s_write(I2S_NUM_1, item, item_size, &bytes_written, portMAX_DELAY); //send prepared chunk to dac
        i2s_write(I2S_NUM_1, samples, CHUNK_SIZE, &bytes_written, portMAX_DELAY); //send prepared chunk to dac

        // printf(">sin:%f\n", sin((float)esp_timer_get_time()/1000000));


        // gpio_set_level(DEBUG_PIN1, 1);
        // gpio_set_level(DEBUG_PIN1, 0);


        
        

    }
}



void app_main(){

    // queue = xQueueCreate(CHUNK_SIZE*QUEUE_SIZE, sizeof(int16_t));                               // rtos queue used for buffering acquired samples before applying effects

    ringbuffer = xRingbufferCreate(CHUNK_SIZE, RINGBUF_TYPE_NOSPLIT);

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 2, task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 2, task_write_handle, 0);

}
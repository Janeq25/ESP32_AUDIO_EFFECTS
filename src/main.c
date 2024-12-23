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

// #include "esp_dsp.h"



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

        // printf("%f, %i\n", pot2, (uint16_t)(pot2 * MAX_BUFFER_LEN));

        // printf("R=%u, W=%u, R-W=%u, W-R=%u\n, ", buffer1.read_ptr, buffer1.write_ptr, buffer1.read_ptr-buffer1.write_ptr, buffer1.write_ptr-buffer1.read_ptr);


        if (buffer1.read_ptr > buffer1.write_ptr) printf("%i\n", buffer1.read_ptr - buffer1.write_ptr);
        if (buffer1.read_ptr <= buffer1.write_ptr) printf("%i\n", buffer1.write_ptr - buffer1.read_ptr);



    }
}


void task_write(){                                  // task responsible for applying effects and sending data to dac in chunks


    dac_setup();

    init_buffer(&buffer1);


    while(1){

        // gpio_set_level(DEBUG_PIN1, 1);

        retrive_chunk(samples);

        // gpio_set_level(DEBUG_PIN1, 0);


        gpio_set_level(DEBUG_PIN2, 1);
        

        for (int i = 0; i < CHUNK_SIZE; i++){


            switch (pot1/(128/EFFECT_NUM))
            {
            case 0:
                samples[i] = samples[i];
                break;

            case 1:
                samples[i] = overdrive(samples[i], pot2, (int)(4*pot3));
                break;
            
            case 2:
                samples[i] = delay(samples[i], (uint16_t)(pot2 * MAX_BUFFER_LEN));
                break;

            case 3:
                samples[i] = echo(samples[i], (uint16_t)(pot2 * MAX_BUFFER_LEN), pot3);
                break;

            case 4:
                samples[i] = echo_fir(samples[i], (uint16_t)(pot2 * MAX_BUFFER_LEN), pot3);
                break;

            case 5:
                samples[i] = overdrive_echo(samples[i], (fmodf(pot2, 0.2f)*5.0f), (int)(pot2*4), (uint16_t)(pot3 * MAX_BUFFER_LEN), 0.5f);
                break;

            case 6:
                samples[i] = tremolo(samples[i]);
                break;

            case 7:
                samples[i] = flanger(samples[i]);
                break;

            case 8:
                samples[i] = FIR_f(samples[i]);
                break;

            default:
                break;
            }

        }


        output_chunk(samples);

        gpio_set_level(DEBUG_PIN2, 0);
        

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
    ringbuffer_handle = xRingbufferCreate(CHUNK_SIZE*QUEUE_SIZE, RINGBUF_TYPE_NOSPLIT);

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 2, &task_read_handle, 1);        //tasks are created pinned to cores
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 2, &task_write_handle, 0);

}
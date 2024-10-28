#include "util.h"

#include "defines.h"
#include "globals.h"

#include "mcp3202/mcp3202.h"
#include "driver/i2s.h"
#include "esp_timer.h"
#include <inttypes.h>
#include <stdio.h>



void measure_read_speed(void) {
    uint16_t voltage;

    const unsigned MEASUREMENTS = 100;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {

        mcp3202_read_diff(&voltage);


    }

    uint64_t end = esp_timer_get_time();


    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}


void retrive_chunk(int16_t* samples){
    for (int i = 0; i < CHUNK_SIZE; i++){       // get chunk form queue and apply effect?


// Receive an item from no-split ring buffer
    size_t item_size;
    int16_t* item = (int16_t* )xRingbufferReceive(ringbuffer_handle, &item_size, pdMS_TO_TICKS(1000));

    samples[i] = *item - POW11; // normalize to int

    vRingbufferReturnItem(ringbuffer_handle, (void *)item);
}

}

void output_chunk(int16_t* samples){
    size_t bytes_written = CHUNK_SIZE;
    uint32_t output[CHUNK_SIZE];
    
    for (int i = 0; i < CHUNK_SIZE; i++)
    {
        
        output[i] = (uint32_t)(samples[i] << 12) + POW21;
        // output[i] = (uint32_t)(samples[i] << 12) + POW21;

    }
    

    i2s_write(I2S_NUM_1, (void *)output, sizeof(output), &bytes_written, portMAX_DELAY); //send prepared chunk to dac

}
#include "util.h"

#include "defines.h"

#include "mcp3202/mcp3202.h"
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
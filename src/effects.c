#include "effects.h"

#include "defines.h"
#include "globals.h"


int16_t delay_buf[MAXDELAYLENGTH];
uint16_t input_ptr = 0;

int16_t delay(int16_t sample_in){
    uint16_t delay_ptr = (input_ptr + (uint16_t)(pot2 * (MAXDELAYLENGTH - 1))) % MAXDELAYLENGTH;

    input_ptr = (input_ptr + 1) % MAXDELAYLENGTH;

    delay_buf[input_ptr] = sample_in +  (uint16_t)(pot3 * delay_buf[delay_ptr]);
    

    // printf("%u, %u, %u\n", delay_ptr, input_ptr, delay_ptr-input_ptr);

    return delay_buf[input_ptr];

}



int16_t overdrive(int16_t sample){
    int16_t out;
    const int scale = POW11;
    if (sample > (pot2 * scale) + 1) out = pot2 * scale;
    else if (sample < (-1 * pot2 * scale) - 1)  out = -1 * pot2 * scale;
    else out = sample;

    return out *  (1 + (1 - pot2) * 6);
}

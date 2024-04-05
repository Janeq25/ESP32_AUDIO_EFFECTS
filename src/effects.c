#include "effects.h"

#include "defines.h"
#include "globals.h"

#include <inttypes.h>

int16_t delay_buffer[MAXDELAYLENGTH];
int32_t delay_index = 0;
uint16_t delay_length = SAMPLERATE * 0.5;
float decay = 0.5;


void delay(int16_t* sample_in, int16_t* sample_out){

    int32_t delayed_index = delay_index-delay_length;
    if (delayed_index <= 0) delayed_index = delayed_index + MAXDELAYLENGTH;

    //printf("delay_index: %li, delayed_index: %li\n", delay_index, delayed_index);

    delay_buffer[delay_index] = *sample_in + (int16_t)(decay * delay_buffer[delay_index]);

    *sample_out = delay_buffer[delayed_index];

    delay_index++;
}
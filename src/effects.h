#pragma once


#include <inttypes.h>



int16_t overdrive(int16_t sample, float gain, int type);

int16_t delay(int16_t in, uint16_t length, float sustain);

int16_t FIR_f(int16_t input);

int16_t echo(int16_t in, uint16_t length, float sustain);

int16_t overdrive_echo(int16_t in, float gain, int type, uint16_t length, float sustain);

int16_t tremolo(int16_t sample_in);

int16_t flanger(int16_t sample_in);

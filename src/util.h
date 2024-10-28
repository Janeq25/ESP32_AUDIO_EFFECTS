#pragma once

#include "inttypes.h"

void measure_read_speed(void); //get speed of mcp3202 reading

void retrive_chunk(int16_t* samples);

void output_chunk(int16_t* samples);
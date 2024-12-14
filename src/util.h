#pragma once

#include "inttypes.h"
#include "math.h"

#include "defines.h"



#define DB_TO_LIN(x) (powf(10, (x)/20))
#define LIN_TO_DB(x) (20*log10f(x))

void measure_read_speed(void); //get speed of mcp3202 reading

void retrive_chunk(int16_t* samples);

void output_chunk(int16_t* samples);


typedef struct {
    int16_t buffer_data[MAX_BUFFER_LEN];
    uint16_t write_ptr;
    uint16_t read_ptr;
}buffer_s;

void init_buffer(buffer_s* buf_handle);

void set_buffer_delay(uint16_t delay_frames, buffer_s* buf_handle);

int16_t buffer_get(buffer_s* buf_handle);

void buffer_set(int16_t in, buffer_s* buf_handle);

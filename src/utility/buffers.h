#pragma once

#include "defines.h"

#include "inttypes.h"


typedef enum {IS_EMPTY, IS_FULL, IS_READ, IS_WRITTEN} input_buffer_state_t;

typedef struct {
    int16_t data[SAMPLEBLOCK];
    unsigned int pointer;
    input_buffer_state_t state;
} input_buffer_t;


typedef struct {
    input_buffer_t input_buffers_table[BUFFERS_NUMBER];
    unsigned int writing_buffer_idx;
    unsigned int reading_buffer_idx;
} input_buffers_t;

void write_to_buf(void* buffers, int input_data);

input_buffer_state_t get_reading_buffer_state(void* buffers);

int16_t* start_buffer_read(void* buffers);

void end_buffer_read(void* buffers);
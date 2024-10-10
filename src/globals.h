#pragma once

#include "defines.h"

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include <inttypes.h>

// global structures like handles and memory allocations

extern QueueHandle_t queue;

extern TaskHandle_t task_read_handle;
extern TaskHandle_t task_write_handle;

extern uint16_t data_in1[CHUNK_SIZE];
extern uint16_t data_in2[CHUNK_SIZE];
extern uint8_t chunk_ready_flag;
extern uint8_t data_in_buffer_ptr;
extern uint16_t data_in_sample_ptr;



extern int16_t samples[CHUNK_SIZE];

extern float pot1;
extern float pot2;

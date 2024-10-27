#pragma once

#include "defines.h"

#include "freertos/FreeRTOS.h"  
// #include "freertos/queue.h"
#include "freertos/ringbuf.h"

#include <inttypes.h>

// global structures like handles and memory allocations

// extern QueueHandle_t queue;

extern RingbufHandle_t ringbuffer_handle;

extern TaskHandle_t task_read_handle;
extern TaskHandle_t task_write_handle;

extern uint32_t samples[CHUNK_SIZE];

extern float pot1;
extern float pot2;

#pragma once

#include "defines.h"

#include "freertos/FreeRTOS.h"  
#include "freertos/queue.h"

#include <inttypes.h>

// global structures like handles and memory allocations

extern QueueHandle_t queue;

extern TaskHandle_t task_read_handle;
extern TaskHandle_t task_write_handle;

extern int16_t samples[CHUNK_SIZE];
#pragma once

#include "defines.h"
#include "util.h"


#include "freertos/FreeRTOS.h"  
#include "freertos/ringbuf.h"

#include <inttypes.h>



extern RingbufHandle_t ringbuffer_handle;

extern TaskHandle_t task_read_handle;
extern TaskHandle_t task_write_handle;

extern int16_t samples[CHUNK_SIZE];

extern uint16_t pot1;
extern float pot2;
extern float pot3;

extern buffer_s buffer1;



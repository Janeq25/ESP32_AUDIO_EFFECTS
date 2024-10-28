#include "globals.h"

#include "defines.h"

// QueueHandle_t queue = NULL;
RingbufHandle_t ringbuffer_handle;



TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

int16_t samples[CHUNK_SIZE];


uint16_t pot1 = 0;
float pot2 = 0;
float pot3 = 0;
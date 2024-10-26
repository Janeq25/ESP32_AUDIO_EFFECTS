#include "globals.h"

#include "defines.h"

QueueHandle_t queue = NULL;

TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

int32_t samples[CHUNK_SIZE];

float pot1 = 0;
float pot2 = 0;
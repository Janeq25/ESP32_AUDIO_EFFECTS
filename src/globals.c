#include "globals.h"

#include "defines.h"

QueueHandle_t queue = NULL;

TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

int16_t samples[CHUNK_SIZE];
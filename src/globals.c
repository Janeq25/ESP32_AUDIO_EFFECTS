#include "globals.h"

#include "defines.h"

QueueHandle_t queue = NULL;

TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;

int16_t samples[CHUNK_SIZE];


uint16_t data_in1[CHUNK_SIZE];
uint16_t data_in2[CHUNK_SIZE];
uint8_t chunk_ready_flag;
uint8_t data_in_buffer_ptr;
uint16_t data_in_sample_ptr;


float pot1 = 0;
float pot2 = 0;
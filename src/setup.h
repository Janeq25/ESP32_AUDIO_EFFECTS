#pragma once

#include "driver/gptimer.h"     //timer peripheral driver
#include "mcp3202/mcp3202.h"    //MCP2302 ADC library
#include "driver/i2s.h"         //driver used for communication with PCM5102 DAC

void user_inputs_setup();

void adc_setup();

static bool IRAM_ATTR acquire_sample_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

void timer_setup();

void dac_setup();
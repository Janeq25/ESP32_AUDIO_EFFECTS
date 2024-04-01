#include "defines.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "driver/i2s.h"
#include "driver/gptimer.h"

#include "freertos/FreeRTOS.h"

#include "mcp3202/mcp3202.h"
#include "utility/buffers.h"


TaskHandle_t task_read_handle = NULL;
TaskHandle_t task_write_handle = NULL;



input_buffers_t input_buffers = {
    .reading_buffer_idx = 0,
    .writing_buffer_idx = 1
};

int pot1 =  0;
int sw1 = 0;


void inputs_setup(){
    gpio_set_pull_mode(GPIO_NUM_11, GPIO_PULLUP_ENABLE);
    gpio_set_direction(GPIO_NUM_11, GPIO_MODE_INPUT);

}

void measure_important_function(void) {
    uint16_t voltage;

    const unsigned MEASUREMENTS = 100;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {

        mcp3202_read_diff(&voltage);

    }

    uint64_t end = esp_timer_get_time();

    printf("%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}

void adc_setup (){

    uint32_t freq;

    mcp3202_config_t mcp3202_cfg = {
        .mosi_io = GPIO_NUM_13,
        .miso_io = GPIO_NUM_12,
        .sclk_io = GPIO_NUM_14,
        .host = SPI3_HOST,
        .clock_speed_hz = 2 * 5000 * 1000,
        .reference_voltage = 5000,      
        .cs_io_num = GPIO_NUM_15
        }; 

    mcp3202_init(&mcp3202_cfg);

    mcp3202_get_actual_freq(&freq);
    ESP_LOGI("ADC SETUP:", "freq = %li", freq);

}

static bool IRAM_ATTR acquire_sample(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){

    BaseType_t high_task_awoken = pdFALSE;
    uint16_t value;

    mcp3202_read_diff(&value);

    write_to_buf(user_ctx, value);

    return high_task_awoken == pdTRUE;
}

void timer_setup(){
    int temp;

    gptimer_handle_t gptimer = NULL;

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0, // counter will reload with 0 on alarm event
        .alarm_count = 226,
        .flags.auto_reload_on_alarm = true, // enable auto-reload
    };

    gptimer_event_callbacks_t cbs = {
        .on_alarm = acquire_sample, // register user callback
    };



    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz, 1 tick = 0.1us
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, &input_buffers));

    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}


void dac_setup(){
        i2s_config_t i2s_dac_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLERATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .dma_buf_count = 8,
        .dma_buf_len = SAMPLEBLOCK,
        .use_apll = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t dacI2SPins = {
        .mck_io_num = GPIO_NUM_0,
        .bck_io_num = GPIO_NUM_4,
        .ws_io_num = GPIO_NUM_5,
        .data_out_num = GPIO_NUM_18,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_1, &i2s_dac_config, 0, NULL);
    i2s_set_pin(I2S_NUM_1, &dacI2SPins);

}



void task_read(){


    adc_setup();

    measure_important_function();

    timer_setup();

    while (1){

        vTaskDelay(pdMS_TO_TICKS(1));
        

    }
}


void task_write(){

    dac_setup();

    while(1){
        size_t bytes_written = 0;

        if (get_reading_buffer_state(&input_buffers) != IS_WRITTEN){
        
            int16_t* audio_data = start_buffer_read(&input_buffers);

            for (int i = 0; i < SAMPLEBLOCK; i++){
            //    printf("%d\n", audio_data[i]);
                audio_data[i] *= 100;
            }

            i2s_write(I2S_NUM_1, (void *)audio_data, SAMPLEBLOCK, &bytes_written, portMAX_DELAY);

            end_buffer_read(&input_buffers);

        }

        vTaskDelay(pdMS_TO_TICKS(1));


    }


}



void app_main(){

    xTaskCreatePinnedToCore(task_read, "task_read", 4096, NULL, 1, task_read_handle, 1);
    xTaskCreatePinnedToCore(task_write, "task_write", 4096, NULL, 1, task_write_handle, 0);
    
}
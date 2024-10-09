#include "setup.h"

#include "defines.h"
#include "globals.h"

#include "esp_log.h"
// #include "freertos/FreeRTOS.h"


static portMUX_TYPE my_spinlock = portMUX_INITIALIZER_UNLOCKED;

void user_inputs_setup(){

    // switch setup
    // gpio_set_pull_mode(SWITCH_PIN, GPIO_PULLUP_ENABLE);
    // gpio_set_direction(SWITCH_PIN, GPIO_MODE_INPUT);

    //debug pin setup
    gpio_set_direction(DEBUG_PIN, GPIO_MODE_OUTPUT);

}



void adc_setup (){

    uint32_t freq;

    mcp3202_config_t mcp3202_cfg = {
        .mosi_io = ADC_MOSI_IO,                                     // HSPI perypherial pins
        .miso_io = ADC_MISO_IO,
        .sclk_io = ADC_SCLK_IO,
        .host = SPI_HOST,
        .clock_speed_hz = ADC_CLK,                          // spi clock speed 2.5MHZ for acquisition time of about 11us
        .reference_voltage = 5000,      
        .cs_io_num = ADC_CS_IO
        }; 

    if (mcp3202_init(&mcp3202_cfg) != MCP3202_OK) ESP_LOGI("ADC_SETUP", "mcp init failed");

    if (mcp3202_get_actual_freq(&freq) != MCP3202_OK) ESP_LOGI("ADC_SETUP", "get freq failed");
    ESP_LOGI("ADC SETUP:", "freq = %li", freq);

}

static bool IRAM_ATTR acquire_sample_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){ //timer interrupt responsible for sample acquisition with desired samplerate
    
    taskENTER_CRITICAL_ISR(&my_spinlock);

    gpio_set_level(DEBUG_PIN, 1);

    BaseType_t high_task_awoken = pdFALSE;
    uint16_t value;

    mcp3202_read_diff(&value);                                      //differential read from adc

    xQueueSendToBackFromISR(queue, &value, high_task_awoken);       //sample is placed on the back of the queue

    gpio_set_level(DEBUG_PIN, 0);

    taskEXIT_CRITICAL_ISR(&my_spinlock);


    return high_task_awoken == pdTRUE;  
}   

void timer_setup(){ 
    int temp;   

    gptimer_handle_t gptimer = NULL;                                //general purpose timer handle

    gptimer_alarm_config_t alarm_config = {                         // alarm (interrupt) config
        .reload_count = 0,                                          // counter will reload with 0 on alarm event
        .alarm_count = ISR_CTR_VALUE,                                         // value closest to sampling period of 1/44100 = 22,6757...us with maximal possible resolution of 0.1us
        .flags.auto_reload_on_alarm = true,                         // enable auto-reload
    };

    gptimer_event_callbacks_t cbs = {
        .on_alarm = acquire_sample_isr,                             // register user callback
    };



    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 10 * 1000 * 1000,                          // 10MHz, 1 tick = 0.1us maximal possible timer resolution
    };

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, &temp));

    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

    ESP_ERROR_CHECK(gptimer_start(gptimer));
}


void dac_setup(){
        i2s_config_t i2s_dac_config = {                             // i2s peripherial setup
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLERATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .dma_buf_count = 4,
        .dma_buf_len = CHUNK_SIZE,
        .use_apll = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t dacI2SPins = {                                 
        .mck_io_num = I2S_MCK_IO,
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DATA_IO,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &i2s_dac_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &dacI2SPins));

}
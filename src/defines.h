#pragma once

// ------------------------------ main defines -----------------------

#define SAMPLERATE 44100    
#define ISR_CTR_VALUE 225
#define ADC_CLK 10000000
// #define SAMPLERATE 16000    
// #define ISR_CTR_VALUE 625
// #define ADC_CLK 10000000
#define CHUNK_SIZE 512
#define QUEUE_SIZE 4        //number of chunks in queue

#define SWITCH_PIN GPIO_NUM_11

#define DEBUG_PIN1 GPIO_NUM_22
#define DEBUG_PIN2 GPIO_NUM_23

#define ADC_MOSI_IO GPIO_NUM_13
#define ADC_MISO_IO GPIO_NUM_12
#define ADC_SCLK_IO GPIO_NUM_14
#define ADC_CS_IO   GPIO_NUM_15
#define SPI_HOST    SPI3_HOST


#define I2S_MCK_IO  GPIO_NUM_0
#define I2S_BCK_IO  GPIO_NUM_4
#define I2S_WS_IO   GPIO_NUM_5
#define I2S_DATA_IO GPIO_NUM_18

// ------------------------------ effect defines ---------------------

#define MAXDELAYLENGTH (1 * SAMPLERATE)
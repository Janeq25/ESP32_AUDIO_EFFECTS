#include "buffers.h"


void write_to_buf(void* buffers, int input_data){
    input_buffers_t* in_buf = buffers;
    input_buffer_t* writing_buffer = &(in_buf->input_buffers_table[in_buf->writing_buffer_idx]);

    //printf("writing: %d to idx: %d in buff: %d\n", input_data, writing_buffer->pointer, in_buf->writing_buffer_idx);
    
    switch(writing_buffer->state){
        case IS_EMPTY:
            writing_buffer->state = IS_WRITTEN;
            writing_buffer->data[writing_buffer->pointer] = input_data;
            writing_buffer->pointer += 1;
        break;

        case IS_WRITTEN:
            if (writing_buffer->pointer >= SAMPLEBLOCK){
                writing_buffer->pointer = 0;
                writing_buffer->state = IS_FULL;

                in_buf->writing_buffer_idx = (in_buf->writing_buffer_idx + 1) % BUFFERS_NUMBER;
                writing_buffer = &(in_buf->input_buffers_table[in_buf->writing_buffer_idx]);

                if (writing_buffer->state == IS_EMPTY){
                    writing_buffer->data[writing_buffer->pointer] = input_data;
                    writing_buffer->pointer += 1;
                }
            }
            else{
                writing_buffer->data[writing_buffer->pointer] = input_data;
                writing_buffer->pointer += 1;
            }
        break;

        case IS_READ:
            //printf("skipping sample\n");
        break;

        case IS_FULL:
            //printf("waiting for buffor to empty\n");
        break;
    }
}



input_buffer_state_t get_reading_buffer_state(void* buffers){
    input_buffers_t* in_buf = buffers;
    return in_buf->input_buffers_table[in_buf->reading_buffer_idx].state;
}

int16_t* start_buffer_read(void* buffers){
    input_buffers_t* in_buf = buffers;
    in_buf->input_buffers_table[in_buf->reading_buffer_idx].state = IS_READ;
    return in_buf->input_buffers_table[in_buf->reading_buffer_idx].data;
}

void end_buffer_read(void* buffers){
    input_buffers_t* in_buf = buffers;
    in_buf->input_buffers_table[in_buf->reading_buffer_idx].state = IS_EMPTY;
    in_buf->reading_buffer_idx = (in_buf->reading_buffer_idx + 1) % BUFFERS_NUMBER;
}
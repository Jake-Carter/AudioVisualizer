/*
 * circBuf.c
 *
 *  Created on: Apr 30, 2018
 *      Author: offba
 */

#include "circBuf.h"
//#include "stdlib.h"

CB_Status CB_initialize_buffer(CircBuf_t * buf, uint32_t length) {
    //check for valid pointer and length
    if(!buf || !length){
        return CB_NULL;
    }
    //allocate memory for buffer
    buf->buffer = (uint16_t *) malloc(length*(sizeof(int16_t)));
    if(!buf->buffer){
        return CB_NULL;
    }
    //initialize head, tail, and size of buffer
    buf->num_items = 0;
    buf->size = length;
    buf->head = buf->buffer;
    buf->tail = buf->buffer;

    return CB_NO_ERROR;
}



uint32_t CB_length(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }

    return buf->num_items;
}

CB_Status CB_clear_buffer(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //reset head, tail, count
    buf->head = buf->buffer;
    buf->tail = buf->buffer;
    buf->num_items = 0;

    return CB_NO_ERROR;
}

CB_Status CB_delete_buffer(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //free allocated memory
    free(buf->buffer);

    return CB_NO_ERROR;
}

int8_t CB_is_buffer_full(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //check if buffer is full
    if(CB_length(buf) == buf->size) {
        return CB_FULL;
    }

    return CB_NO_ERROR;
}

int8_t CB_is_buffer_empty(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //check if buffer is empty
    if(CB_length(buf) == 0) {
        return CB_EMPTY;
    }

    return CB_NO_ERROR;
}

CB_Status CB_add_item(CircBuf_t * buf, uint16_t item) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }

    *(buf->head) = item;
    if (buf->num_items == buf->size) {
        buf->num_items = buf->size;
    }
    else {
        (buf->num_items)++;
    }
    if (buf->head == buf->buffer + buf->size - 1) {
        buf->head = buf->buffer;
    }
    else {
        (buf->head)++;
    }
    if (buf->head == buf->tail) {
        if (buf->tail == (buf->buffer + buf->size)) {
            buf->tail = buf->buffer;
        }
        else {
            (buf->tail)++;
        }
    }

    return CB_NO_ERROR;
}

uint8_t CB_remove_item(CircBuf_t * buf) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //remove item from buffer
    uint16_t item = *(buf->tail);
    (buf->num_items)--;
    if (buf->tail == (buf->buffer + buf->size)) {
        buf->tail = buf->buffer;
    }
    else {
        (buf->tail)++;
    }

    return item;
}

CB_Status CB_copy_buffer(CircBuf_t * in_buf, CircBuf_t * out_buf) {
    //check for valid pointers
    if(!in_buf || !in_buf->buffer) {
        return CB_NULL;
    }
    //copy contents of buffer
    uint32_t i;
    //out_buf->buffer[out_buf->tail] = in_buf->buffer[in_buf->tail];
    //out_buf->buffer[out_buf->head] = in_buf->buffer[in_buf->head];
    for(i = 0; i < CB_length(in_buf); i++) {
        CB_add_item(out_buf, in_buf->buffer[i]);
    }

    return CB_NO_ERROR;
}

CB_Status CB_add_string(CircBuf_t * buf, uint8_t * string, uint32_t length) {
    //check for valid pointers
    if(!buf || !buf->buffer) {
        return CB_NULL;
    }
    //add string to buffer
    uint32_t i;
    for (i = 0; i < length; i++) {
        CB_add_item(buf, string[i]);
    }

    return CB_NO_ERROR;
}


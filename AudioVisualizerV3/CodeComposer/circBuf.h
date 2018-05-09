/*
 * circBuf.h
 *
 *  Created on: Apr 30, 2018
 *      Author: offba
 */

#ifndef CIRCBUF_H_
#define CIRCBUF_H_

#include "stdlib.h"
#include "stdint.h"

typedef struct {
    uint16_t * buffer;
    volatile uint16_t * head;
    volatile uint16_t * tail;
    volatile uint32_t num_items;
    volatile uint32_t size;
} CircBuf_t;

typedef enum {
    CB_NO_ERROR = 0,
    CB_FULL,
    CB_NOT_FULL,
    CB_EMPTY,
    CB_NOT_EMPTY,
    CB_NULL,
} CB_Status;


uint32_t CB_length(CircBuf_t * buf);
CB_Status CB_initialize_buffer(CircBuf_t * buf, uint32_t length);
CB_Status CB_clear_buffer(CircBuf_t * buf);
CB_Status CB_delete_buffer(CircBuf_t * buf);
int8_t CB_is_buffer_full(CircBuf_t * buf);
int8_t CB_is_buffer_empty(CircBuf_t * buf);
CB_Status CB_add_item(CircBuf_t * buf, uint16_t item);
uint8_t CB_remove_item(CircBuf_t * buf);
CB_Status CB_copy_buffer(CircBuf_t * in_buf, CircBuf_t * out_buf);
CB_Status CB_add_string(CircBuf_t * buf, uint8_t * string, uint32_t length);

#endif /* CIRCBUF_H_ */

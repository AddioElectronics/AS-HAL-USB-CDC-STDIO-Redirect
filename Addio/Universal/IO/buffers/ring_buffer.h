#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint-gcc.h>


typedef struct
{
	uint8_t* buf;			//Buffer
	uint8_t* end;			//Pointer to the end of the buffer.
	size_t size;			//Buffer size
	size_t elem_size;		//Element size
	volatile uint8_t* head;	//Pointer to Last byte
	volatile uint8_t* tail;	//Pointer to First byte
	size_t length;			//How many elements in the buffer
}ring_buffer_t;

enum
{
	RBUF_ERROR_NONE = 0,
	RBUF_ERROR,
	RBUF_ERROR_FULL,
	RBUF_ERROR_EMPTY,
	RBUF_ERROR_NULL,
	RBUF_ERROR_NO_CAPACITY,
	RBUF_ERROR_ALIGNMENT,
	RBUF_ERROR_INVALID
}ring_buffer_error_t;

typedef uint8_t ringbuffer_error_t;



ringbuffer_error_t ring_buffer_init(void* buf, size_t size, uint8_t elem_size, ring_buffer_t* out_rb);

ringbuffer_error_t ring_buffer_put(ring_buffer_t* rb, void* data);
ringbuffer_error_t ring_buffer_get(ring_buffer_t* rb, void* out_data);



uint32_t ring_buffer_write(ring_buffer_t* rb, void* data, uint32_t count);
uint32_t ring_buffer_read(ring_buffer_t* rb, void* out_data, uint32_t count);

ringbuffer_error_t ring_buffer_peek(ring_buffer_t* rb, void* out_data);
uint32_t ring_buffer_peekMany(ring_buffer_t* rb, void* out_data, uint32_t count);


void empty_ring_buffer(ring_buffer_t* rb);


static bool __attribute__((__always_inline__)) ring_buffer_full(ring_buffer_t* rb)
{
	return rb->length == rb->size;
}

static bool __attribute__((__always_inline__)) ring_buffer_empty(ring_buffer_t* rb)
{
	return rb->length == 0;
}

static size_t __attribute__((__always_inline__)) ring_buffer_get_available_space(ring_buffer_t* rb)
{
	return rb->size - rb->length;
}


#endif /* RING_BUFFER_H_ */
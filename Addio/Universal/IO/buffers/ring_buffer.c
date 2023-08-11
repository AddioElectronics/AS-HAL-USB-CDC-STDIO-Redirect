#include "ring_buffer.h"

#include <hal_atomic.h>
#include <utils_assert.h>

/*
*	Initializes a ring buffer.
*
*	/param		buf					Pointer to the array that will be used for the internal buffer.
*	/param		size				Total size of buf, in bytes.
*	/param		elem_size			How many bytes a single element uses.
*	/param		out_rb				The initialized ring buffer.
*
*	/returns	ringbuffer_error_t	Error code
*/
ringbuffer_error_t ring_buffer_init(void* buf, size_t size, uint8_t elem_size, ring_buffer_t* out_rb)
{
	ASSERT(out_rb && buf && size && elem_size);
	
	//if(out_rb == NULL || buf == NULL)
	//return RBUF_ERROR_NULL;
	//
	//if(size == 0 || elem_size == 0)
	//return RBUF_ERROR_INVALID;
	
	out_rb->buf = buf;
	out_rb->size = size;
	out_rb->elem_size = elem_size;
	out_rb->head = buf;
	out_rb->tail = buf;
	out_rb->end = buf + size;
	
	return RBUF_ERROR_NONE;
}


/*
*	Writes 1 element to the ring buffer.
*
*	/param		rb					Pointer to the ring buffer.
*	/param		out_data			Pointer to the value to copy from.
*
*	/returns	ringbuffer_error_t	Error code
*/
ringbuffer_error_t ring_buffer_put(ring_buffer_t* rb, void* data)
{
	ASSERT(rb && data);
	
	volatile hal_atomic_t flags;
	
	if(ring_buffer_full(rb))
	return RBUF_ERROR_FULL;
	
	atomic_enter_critical(&flags);
	
	//Store current head in case interrupt happens.
	uint8_t* head_snapshot = rb->head;
	
	if(rb->head += rb->elem_size == rb->end)
	rb->head = rb->buf;
	
	rb->length += rb->elem_size;
	
	atomic_leave_critical(&flags);
	
	memcpy(head_snapshot, data, rb->elem_size);
	
	return RBUF_ERROR_NONE;
}


/*
*	Read 1 element from the ring buffer.
*
*	/param		rb					Pointer to the ring buffer.
*	/param		out_data			Pointer to the value to copy to.
*
*	/returns	ringbuffer_error_t	Error code
*/
ringbuffer_error_t ring_buffer_get(ring_buffer_t* rb, void* out_data)
{
	ASSERT(rb && out_data);
	
	volatile hal_atomic_t flags;
	
	if(ring_buffer_empty(rb))
	return RBUF_ERROR_EMPTY;
	
	atomic_enter_critical(&flags);
	
	//Store current tail in case interrupt happens.
	uint8_t* tail_snapshot = rb->tail;
	
	rb->tail += rb->elem_size;
	
	if(rb->tail == rb->end)
	rb->tail = rb->buf;
	
	rb->length -= rb->elem_size;
	
	atomic_leave_critical(&flags);
	
	memcpy(out_data, tail_snapshot, rb->elem_size);
	
	return RBUF_ERROR_NONE;
}

/*
*	Write multiple elements to the ring buffer.
*
*	/param		rb			Pointer to the ring buffer.
*	/param		out_data	Pointer to the array to copy data from.
*	/param		count		How many elements to write (not bytes).
*
*	/returns				How many elements were able to be written.
*/
uint32_t ring_buffer_write(ring_buffer_t* rb, void* data, uint32_t count)
{
	ASSERT(rb && data);
	
	if(count == 0)
	{
		return 0;
	}
	
	volatile hal_atomic_t flags;

	if(ring_buffer_full(rb))
	return 0;
	
	uint32_t total_written;	//How many elements were written.
	size_t write_size = count * rb->elem_size;
	
	
	//Adjust head and count within the critical space,
	//and do the copy afterwards. This allows us to get in and out of the critical space.
	//If an interrupt happens, it will copy data after the new head.
	//When we come back, we will copy data to the snapshot.
	//Only problem is, if a read happens in an interrupt before the data is copied,
	//it may receive garbage data.
	atomic_enter_critical(&flags);
	
	size_t available_space = ring_buffer_get_available_space(rb);
	uint8_t* head_snapshot = rb->head;
	
	//Not enough room for data.
	if(write_size > available_space)
	{
		//Copy as much data as possible, and throw away the rest.
		write_size = available_space;
		//return RBUF_ERROR_NO_CAPACITY;
	}
	
	//Increase ring buffer counter.
	rb->length += write_size;
	
	//Let the calling function know how many bytes were written.
	total_written = write_size / rb->elem_size;
	
	//If the write fits between head and end, the tail is before the head.
	if(head_snapshot + write_size > rb->end)
	{
		//Write does not fit and will need to be split into 2 copies.
		
		size_t end_space;	//Stores amount of bytes between head and end.
		size_t start_size;	//Stores amount of bytes left to write after the start.
		
		end_space = rb->end - rb->head;
		start_size = write_size - end_space;
		rb->head = rb->buf + start_size;
		
		atomic_leave_critical(&flags);
		
		memcpy(head_snapshot, data, end_space);
		memcpy(rb->buf, data + end_space, start_size);
		
	}
	else
	{
		//Write fits between head and end.
		
		rb->head += write_size;
		
		if(rb->head == rb->end)
		rb->head = rb->buf;
		
		atomic_leave_critical(&flags);
		
		memcpy(head_snapshot, data, write_size);
	}
	
	
	
	return total_written;
	
}

/*
*	Read multiple elements from the ring buffer.
*
*	/param		rb			Pointer to the ring buffer.
*	/param		out_data	Pointer to the array to copy data to.
*	/param		count		How many elements to read (not bytes).
*
*	/returns				How many elements were able to be read.
*/
uint32_t ring_buffer_read(ring_buffer_t* rb, void* out_data, uint32_t count)
{
	ASSERT(rb && out_data);
	
	if(count == 0)
	{
		return 0;
	}
	
	volatile hal_atomic_t flags;
	
	if(ring_buffer_empty(rb))
	return 0;
	
	uint32_t total_read;	//How many elements were read.
	size_t read_size = count * rb->elem_size;
	
	//Adjust tail and count within the critical space,
	//and do the copy afterwards. This allows us to get in and out of the critical space.
	//If an interrupt happens, it will copy data after the new tail.
	//When we come back, we will copy data from the snapshot.
	//Only problem is, if a write happens in an interrupt before the data is copied,
	//it may overwrite our data, and out_data may receive that write.
	atomic_enter_critical(&flags);
	
	uint8_t* tail_snapshot = rb->tail;
	
	//Buffer contains less data then requested.
	if(rb->length < read_size)
	{
		//Copy all the data.
		read_size = rb->length;
	}
	
	//Decrease ringbuffer count.
	rb->length -= read_size;
	
	//Let the calling function know how many elements were read.
	total_read = read_size / rb->elem_size;
	
	//If the read fits between tail and end, the tail is after the head.
	if(tail_snapshot + read_size > rb->end)
	{
		//Read does not fit and will need to be split into 2 copies.
		
		size_t end_space;	//Stores amount of bytes between tail and end.
		size_t start_size;	//Stores amount of bytes left to read after the start.
		
		end_space = rb->end - rb->tail;
		start_size = read_size - end_space;
		rb->tail = rb->buf + start_size;
		
		atomic_leave_critical(&flags);
		
		memcpy(out_data, tail_snapshot, end_space);
		memcpy(out_data + end_space, rb->buf, start_size);
	}
	else
	{
		//Read fits between head and end.
		
		rb->tail += read_size;
		
		if(rb->tail == rb->end)
		rb->tail = rb->buf;
		
		atomic_leave_critical(&flags);
		
		memcpy(out_data, tail_snapshot, read_size);
	}
	
	
	return total_read;
}

ringbuffer_error_t ring_buffer_peek(ring_buffer_t* rb, void* out_data)
{
	ASSERT(rb && out_data);
	
	if(ring_buffer_empty(rb))
	return RBUF_ERROR_EMPTY;
	
	memcpy(out_data, rb->tail, rb->elem_size);
	
	return RBUF_ERROR_NONE;
}



uint32_t ring_buffer_peekMany(ring_buffer_t* rb, void* out_data, uint32_t count)
{
	ASSERT(rb && out_data);
	
	if(count == 0)
	{
		return 0;
	}
	
	volatile hal_atomic_t flags;
	
	if(ring_buffer_empty(rb))
	return 0;
	
	uint32_t total_read;	//How many elements were read.
	size_t read_size = count * rb->elem_size;
	
	//Adjust tail and count within the critical space,
	//and do the copy afterwards. This allows us to get in and out of the critical space.
	//If an interrupt happens, it will copy data after the new tail.
	//When we come back, we will copy data from the snapshot.
	//Only problem is, if a write happens in an interrupt before the data is copied,
	//it may overwrite our data, and out_data may receive that write.
	atomic_enter_critical(&flags);
	
	uint8_t* tail_snapshot = rb->tail;
	
	//Buffer contains less data then requested.
	if(rb->length < read_size)
	{
		//Copy all the data.
		read_size = rb->length;
	}
	
	//Let the calling function know how many elements were read.
	total_read = read_size / rb->elem_size;
	
	//If the read fits between tail and end, the tail is after the head.
	if(tail_snapshot + read_size > rb->end)
	{
		//Read does not fit and will need to be split into 2 copies.
		
		size_t end_space = rb->end - rb->tail;		//Stores amount of bytes between tail and end.
		size_t start_size = read_size - end_space;	//Stores amount of bytes left to read after the start.
		
		atomic_leave_critical(&flags);
		
		memcpy(out_data, tail_snapshot, end_space);
		memcpy(out_data + end_space, rb->buf, start_size);
	}
	else
	{
		//Read fits between head and end.
		
		
		atomic_leave_critical(&flags);
		
		memcpy(out_data, tail_snapshot, read_size);
	}
	
	
	return total_read;
}


/*
*	Empty the ring buffer.
*/
void empty_ring_buffer(ring_buffer_t* rb)
{
	volatile hal_atomic_t flags;
	atomic_enter_critical(&flags);
	rb->head = rb->buf;
	rb->tail = rb->buf;
	rb->length = 0;
	atomic_leave_critical(&flags);
}

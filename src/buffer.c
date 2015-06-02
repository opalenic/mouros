/**
 * @file
 *
 * This file contains the implementation of universal FIFO byte queues for
 * MourOS.
 *
 */

#include <stddef.h> // For NULL.

#include <mouros/buffer.h> // For the buffer functions & data types.


/**
 * Inserts a single byte into the buffer.
 *
 * @param buf Pointer to the buffer struct.
 * @param in  The byte to be inserted.
 * @return True if the byte was successfully added, false otherwise.
 */
static inline bool write_char(buffer_t *buf, uint8_t ch)
{
	if (!buf->overflow) {
		buf->data[buf->write_pos] = ch;

		// If used instead of modulo in hope of making it faster.
		// On Cortex-Mx.
		buf->write_pos++;
		if (buf->write_pos == buf->data_len) {
			buf->write_pos = 0;
		}

		if (buf->write_pos == buf->read_pos) {
			buf->overflow = true;
		}

		return true;
	} else {
		return false;
	}
}

void os_buffer_init(buffer_t *buf,
                    uint8_t *buf_data,
                    uint32_t buf_data_len,
                    void (*data_added_callback)(void))
{
	buf->data = buf_data;
	buf->data_len = buf_data_len;
	buf->read_pos = 0;
	buf->write_pos = 0;
	buf->overflow = false;
	buf->data_added = data_added_callback;
}

bool os_buffer_write_byte(buffer_t *buf, uint8_t in)
{
	bool ret = write_char(buf, in);
	if (ret && buf->data_added != NULL) {
		buf->data_added();
	}

	return ret;
}

uint32_t os_buffer_write(buffer_t *buf, uint8_t *in, uint32_t in_len)
{
	uint32_t i;
	for (i = 0; i < in_len; i++) {
		if (write_char(buf, in[i])) {
			break;
		}
	}

	if (i > 0 && buf->data_added != NULL) {
		buf->data_added();
	}

	return i;
}

bool os_buffer_read_ch(buffer_t *buf, uint8_t *out)
{
	if (buf->read_pos != buf->write_pos || buf->overflow) {

		*out = buf->data[buf->read_pos];

		// If used instead of modulo in hope of making it faster.
		// On Cortex-Mx.
		buf->read_pos++;
		if (buf->read_pos == buf->data_len) {
			buf->read_pos = 0;
		}

		buf->overflow = false;

		return true;
	} else {
		return false;
	}
}

uint32_t os_buffer_read(buffer_t *buf, uint8_t *out, uint32_t out_len)
{
	uint32_t i;
	for (i = 0; i < out_len; i++) {
		if (os_buffer_read_ch(buf, &out[i])) {
			break;
		}
	}

	return i;
}

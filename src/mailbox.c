/**
 * @file
 *
 * This file contains the implementation of universal FIFO byte queues for
 * MourOS.
 *
 */

#include <stddef.h> // For NULL.

#include <mouros/mailbox.h> // For the mailbox functions & data types.

#include <libopencm3/cm3/assert.h> // For the assert macros.

/**
 * Inserts a single message into the mailbox.
 *
 * @param mb  Pointer to the buffer struct.
 * @param msg Pointer to the message to be inserted.
 * @return True if the message was successfully added, false otherwise.
 */
static inline bool write_msg(mailbox_t *mb, const void *msg)
{
	uint32_t new_write_pos = mb->write_pos + mb->msg_size;
	if (new_write_pos == mb->msg_buf_len) {
		new_write_pos = 0;
	}

	if (new_write_pos != mb->read_pos) {
		for (uint8_t i = 0; i < mb->msg_size; i++) {
			mb->msg_buf[mb->write_pos + i] = ((uint8_t *) msg)[i];
		}

		mb->write_pos = new_write_pos;

		return true;
	} else {
		return false;
	}
}

void os_mailbox_init(mailbox_t *mb,
                     uint8_t *msg_buf,
                     uint32_t msg_buf_len,
                     uint8_t msg_size,
                     void (*data_added_callback)(void))
{
	mb->msg_buf = msg_buf;
	mb->msg_buf_len = msg_buf_len;
	mb->msg_size = msg_size;
	mb->data_added = data_added_callback;

	mb->read_pos = 0;
	mb->write_pos = 0;

	cm3_assert(msg_buf_len % msg_size == 0);
}

bool os_mailbox_write(mailbox_t *mb, void *msg)
{
	bool ret = write_msg(mb, msg);
	if (ret && mb->data_added != NULL) {
		mb->data_added();
	}

	return ret;
}

uint32_t os_mailbox_write_multiple(mailbox_t *mb,
                                   const void *msgs,
                                   uint32_t msg_num)
{
	uint32_t i;
	const uint8_t *curr_msg = msgs;
	for (i = 0; i < msg_num; i++, curr_msg += mb->msg_size) {
		if (!write_msg(mb, curr_msg)) {
			break;
		}
	}

	if (i > 0 && mb->data_added != NULL) {
		mb->data_added();
	}

	return i;
}

bool os_mailbox_read(mailbox_t *mb, void *out)
{
	if (mb->read_pos != mb->write_pos) {

		for (uint8_t i = 0; i < mb->msg_size; i++) {
			((uint8_t *) out)[i] = mb->msg_buf[mb->read_pos + i];
		}

		uint32_t new_read_pos = mb->read_pos + mb->msg_size;
		if (new_read_pos == mb->msg_buf_len) {
			new_read_pos = 0;
		}

		mb->read_pos = new_read_pos;

		return true;
	} else {
		return false;
	}
}

uint32_t os_mailbox_read_multiple(mailbox_t *mb,
                                  void *out,
                                  uint32_t out_msg_num)
{
	uint32_t i;
	uint8_t *curr_msg = out;
	for (i = 0; i < out_msg_num; i++, curr_msg += mb->msg_size) {
		if (!os_mailbox_read(mb, curr_msg)) {
			break;
		}
	}

	return i;
}

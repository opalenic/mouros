/**
 * @file
 *
 * This file contains the declarations of functions and data types for the
 * MourOS mailboxes.
 *
 */

#ifndef MOUROS_MAILBOX_H_
#define MOUROS_MAILBOX_H_

#include <stdint.h>  // For uint32_t, etc.
#include <stdbool.h> // For bool.


/**
 * A structure representing a single circular FIFO mailbox buffer.
 */
typedef struct mailbox {
	/**
	 * Pointer to the beginning of the memory segment holding the mailbox
	 * data.
	 */
	uint8_t *msg_buf;
	/**
	 * The size of the memory area pointed to by msg_buf.
	 */
	uint32_t msg_buf_len;
	/**
	 * The size of a single mailbox message.
	 */
	uint32_t msg_size;
	/**
	 * Internal pointer pointing to the beginning of the next message in
	 * msg_buf that should be read.
	 */
	uint32_t read_pos;
	/**
	 * Internal pointer pointing to the next location a message should be
	 * written to.
	 */
	uint32_t write_pos;
	/**
	 * Callback function called when new data is inserted into the mailbox.
	 */
	void (*data_added)(void);
} mailbox_t;


/**
 * Initializes the mailbox struct (mb).
 *
 * @note The message buffer size must be an integer multiple of the message
 *       size. (msg_buf_len % msg_size == 0)
 *
 * @param mb                  Pointer to the struct to be initialized.
 * @param msg_buf             Pointer to the memory area to be used to hold the
 *                            mailbox data.
 * @param num_msgs            The number of messages in msg_buf.
 * @param msg_size            The size in bytes of a single message.
 * @param data_added_callback Optional callback that gets called every time new
 *                            data is added to the mailbox. Can be NULL.
 */
void os_mailbox_init(mailbox_t *mb,
                     void *msg_buf,
                     uint32_t num_msgs,
                     uint32_t msg_size,
                     void (*data_added_callback)(void));

/**
 * Inserts a new single message into the mailbox. Calls data_added_callback if
 * the insertion was successful.
 *
 * @param mb  Pointer to the mailbox struct.
 * @param msg Pointer to the message to be inserted.
 * @return True if the message was successfully added, false otherwise.
 */
bool os_mailbox_write(mailbox_t *mb, const void *msg);

/**
 * Inserts msg_num messages into the mailbox. Calls data_added_callback if the
 * insertion was successful.
 *
 * @param mb      Pointer to the mailbox struct.
 * @param msgs    Pointer to the messages to be added.
 * @param msg_num The number of messages in msgs.
 * @return The number of bytes successfully inserted into the buffer. This
 *         should be equal to in_len if all data was inserted.
 */
uint32_t os_mailbox_write_multiple(mailbox_t *mb,
                                   const void *msgs,
                                   uint32_t msg_num);

/**
 * Reads a single message from the mailbox.
 *
 * @param mb  Pointer to the mailbox struct.
 * @param out Pointer to a place in memory to store the read message.
 * @return True if there was at least one message in the mailbox. It will be
 *         stored in the location pointed to by out. False if the mailbox was
 *         empty.
 */
bool os_mailbox_read(mailbox_t *mb, void *out);

/**
 * Reads messages from the mailbox into the location pointed to by out. Reads
 * either all the messages that are available in the mailbox, or out_msg_num
 * messages, whichever is smaller.
 *
 * @param mb          Pointer to the mailbox struct.
 * @param out         Pointer to the data array
 * @param out_msg_num The number of messages that can be stored in out.
 * @return Returns the number of read messages.
 */
uint32_t os_mailbox_read_multiple(mailbox_t *mb,
                                  void *out,
                                  uint32_t out_msg_num);


#endif /* MOUROS_MAILBOX_H_ */

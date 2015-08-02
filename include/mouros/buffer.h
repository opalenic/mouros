/**
 * @file
 *
 * This file contains the declarations of functions and data types for the byte
 * FIFO queues used in MourOS.
 *
 */

#ifndef MOUROS_BUFFER_H_
#define MOUROS_BUFFER_H_

#include <stdint.h>  // For uint32_t, etc.
#include <stdbool.h> // For bool.


/**
 * A structure representing a single circular FIFO buffer.
 */
typedef struct buffer {
	/**
	 * Pointer to the beginning of the memory segment holding the buffer
	 * data.
	 */
	uint8_t *data;
	/**
	 * The size of the memory area pointed to by data.
	 */
	uint32_t data_len;
	/**
	 * Internal pointer pointing to the next byte in data that should be
	 * read.
	 */
	uint32_t read_pos;
	/**
	 * Internal pointer pointing to the next location a new byte of data
	 * should be written.
	 */
	uint32_t write_pos;
	/**
	 * Flag stating whether the buffer has overflown.
	 */
	bool overflow;
	/**
	 * Callback function called when new data is inserted into the buffer.
	 */
	void (*data_added)(void);
} buffer_t;


/**
 * Initializes the buffer struct (buf).
 *
 * @param buf                 Pointer to the struct to be initialized.
 * @param buf_data            Pointer to the memory area to be used to hold the
 *                            buffer data.
 * @param buf_data_len        The size of buf_data.
 * @param data_added_callback Optional callback that gets called every time new
 *                            data is added to the buffer. Can be NULL.
 */
void os_buffer_init(buffer_t *buf,
                    uint8_t *buf_data,
                    uint32_t buf_data_len,
                    void (*data_added_callback)(void));

/**
 * Inserts a single byte into the buffer. Calls data_added_callback if the
 * insertion was successful.
 *
 * @param buf Pointer to the buffer struct.
 * @param in  The byte to be inserted.
 * @return True if the byte was successfully added, false otherwise.
 */
bool os_buffer_write_byte(buffer_t *buf, uint8_t in);

/**
 * Inserts the contents of data into the buffer. Calls data_added_callback if
 * the insertion was successful.
 *
 * @param buf    Pointer to the buffer struct.
 * @param in     Pointer to the data to be added.
 * @param in_len Length of data.
 * @return The number of bytes successfully inserted into the buffer. This
 *         should be equal to in_len if all data was inserted.
 */
uint32_t os_buffer_write(buffer_t *buf, const uint8_t *in, uint32_t in_len);

/**
 * Inserts the string pointed to by str into the buffer. Will not write the
 * terminating '\0' character.
 *
 * @param buf Pointer to the buffer struct.
 * @param str Pointer to the string to be added.
 * @return The number of characters succesfully inserted into the buffer. This
 *         should be equal to the string lenght minus one (the terminating '\0'
 *         character, which is not written to the buffer).
 */
uint32_t os_buffer_write_str(buffer_t *buf, const char *str);


/**
 * Reads a single byte from the buffer.
 *
 * @param buf Pointer to the buffer struct.
 * @param out Pointer to a uint8_t to store the read byte.
 * @return True if there was at least one character in the buffer. It will be
 *         stored in the location pointed to by out. False if the buffer is
 *         empty.
 */
bool os_buffer_read_ch(buffer_t *buf, uint8_t *out);

/**
 * Reads data from the buffer into the location pointed to by out. Reads either
 * all the data that's available in the buffer, or out_len bytes, whichever is
 * smaller.
 *
 * @param buf      Pointer to the buffer struct.
 * @param data     Pointer to the data array
 * @param data_len
 * @return
 */
uint32_t os_buffer_read(buffer_t *buf, uint8_t *out, uint32_t out_len);


#endif /* MOUROS_BUFFER_H_ */

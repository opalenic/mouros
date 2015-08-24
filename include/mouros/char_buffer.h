/**
 * @file
 *
 * A specialization of the MourOS mailbox for sending and receiving single chars
 * and strings.
 *
 */

#ifndef MOUROS_CHAR_BUFFER_H_
#define MOUROS_CHAR_BUFFER_H_

#include <stdbool.h> // For true, false.
#include <stdint.h> // For unit32_t, etc.

#include <mouros/mailbox.h> // For the mailbox struct & functions.

/**
 * Initializes the mailbox pointed to by mb as a single byte character buffer.
 *
 * @param mb                  Pointer to the mailbox to be used for the
 *                            character buffer.
 * @param buf                 Pointer to a char buffer to hold the data.
 * @param buf_len             Length of buf.
 * @param data_added_callback Optional callback called when data is inserted
 *                            into the buffer.
 */
void os_char_buffer_init(mailbox_t *mb,
                         char *buf,
                         uint32_t buf_len,
                         void (*data_added_callback)(void));

/**
 * Tries to write a single char to the buffer.
 *
 * @param mb Pointer to the mailbox implementing the character buffer.
 * @param ch The character to be inserted.
 * @return Insertion result. True if the character was inserted, false if it
 *         wasn't.
 */
bool os_char_buffer_write_ch(mailbox_t *mb, char ch);

/**
 * Writes a single char to the buffer. Blocks & retries until the character has
 * been successfully inserted.
 *
 * @param mb Pointer to the mailbox implementing the character buffer.
 * @param ch The character to be inserted.
 */
void os_char_buffer_write_ch_blocking(mailbox_t *mb, char ch);

/**
 * Writes a number (buf_len) of characters to the buffer. Not all characters may
 * be written if the buffer becomes full.
 *
 * @param mb      Pointer to the mailbox implementing the character buffer.
 * @param buf     Pointer to the characters to be inserted into the buffer.
 * @param buf_len The number of characters to be inserted.
 * @return The number of characters actually written.
 */
uint32_t os_char_buffer_write_buf(mailbox_t *mb,
                                  const char *buf,
                                  uint32_t buf_len);

/**
 * Writes a number (buf_len) of characters to the buffer. If the buffer becomes
 * full, the function will block and retry until all characters are inserted.
 *
 * @param mb      Pointer to the mailbox implementing the character buffer.
 * @param buf     Pointer to the characters to be inserted into the buffer.
 * @param buf_len The number of characters to be inserted.
 */
void os_char_buffer_write_buf_blocking(mailbox_t *mb,
                                       const char *buf,
                                       uint32_t buf_len);

/**
 * Writes a '\0' terminated string to the buffer. Not all characters may be
 * written if the buffer becomes full.
 *
 * @param mb  Pointer to the mailbox implementing the character buffer.
 * @param str Pointer to the string to be inserted into the buffer.
 * @return The number of characters actually written.
 */
uint32_t os_char_buffer_write_str(mailbox_t *mb, const char *str);

/**
 * Writes a '\0' terminated string to the buffer. If the buffer becomes full,
 * the function will block and retry until all characters are inserted.
 *
 * @param mb  Pointer to the mailbox implementing the character buffer.
 * @param str Pointer to the string to be inserted into the buffer.
 * @return The number of characters written.
 */
uint32_t os_char_buffer_write_str_blocking(mailbox_t *mb, const char *str);

/**
 * Tries to read a single char from the buffer.
 *
 * @param mb Pointer to the mailbox implementing the character buffer.
 * @param ch Pointer to a location in memory where the read character should be
 *           written.
 * @return True if there was a character in the buffer and if it was written to
 *         the location pointed to by ch. False if no character was available.
 */
bool os_char_buffer_read_ch(mailbox_t *mb, char *ch);

/**
 * Reads a single char from the buffer. Blocks until there is a character
 * available.
 *
 * @param mb Pointer to the mailbox implementing the character buffer.
 * @return The read character.
 */
char os_char_buffer_read_ch_blocking(mailbox_t *mb);

/**
 * Reads at most buf_len characters from the buffer. May return before buf_len
 * characters are read if the buffer becomes empty.
 *
 * @param mb      Pointer to the mailbox implementing the character buffer.
 * @param buf     Pointer to the location in memory where the read characters
 *                should be placed.
 * @param buf_len The length of buf.
 * @return The number of characters actually read.
 */
uint32_t os_char_buffer_read_buf(mailbox_t *mb,
                                 char *buf,
                                 uint32_t buf_len);

/**
 * Reads buf_len characters from the buffer. If the buffer becomes empty, the
 * function will block and retry until buf_len characters have been read.
 *
 * @param mb      Pointer to the mailbox implementing the character buffer.
 * @param buf     Pointer to the location in memory where the read characters
 *                should be placed.
 * @param buf_len The length of buf.
 */
void os_char_buffer_read_buf_blocking(mailbox_t *mb,
                                      char *buf,
                                      uint32_t buf_len);


#endif /* MOUROS_CHAR_BUFFER_H_ */

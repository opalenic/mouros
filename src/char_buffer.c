/**
 * @file
 *
 * A specialization of the MourOS mailbox for sending and receiving single chars
 * and strings.
 *
 */

#include <mouros/char_buffer.h> // The character buffer declarations.


void os_char_buffer_init(mailbox_t *mb,
                         char *buf,
                         uint32_t buf_len,
                         void (*data_added_callback)(void))
{
	os_mailbox_init(mb, (void *) buf, buf_len, 1, data_added_callback);
}

bool os_char_buffer_write_ch(mailbox_t *mb, char ch)
{
	return os_mailbox_write(mb, &ch);
}

void os_char_buffer_write_ch_blocking(mailbox_t *mb, char ch)
{
	while (!os_mailbox_write(mb, &ch));
}

uint32_t os_char_buffer_write_buf(mailbox_t *mb,
                                  const char *buf,
                                  uint32_t buf_len)
{
	return os_mailbox_write_multiple(mb, buf, buf_len);
}

void os_char_buffer_write_buf_blocking(mailbox_t *mb,
                                       const char *buf,
                                       uint32_t buf_len)
{
	uint32_t pos = 0;

	while (pos <= buf_len) {
		pos += os_mailbox_write_multiple(mb, &buf[pos], buf_len - pos);
	}
}

uint32_t os_char_buffer_write_str(mailbox_t *mb, const char *str)
{
	uint32_t num_chars = 0;
	while (str[num_chars] != '\0') {
		if (!os_mailbox_write(mb, &str[num_chars])) {
			break;
		}

		num_chars++;
	}

	return num_chars;
}

uint32_t os_char_buffer_write_str_blocking(mailbox_t *mb, const char *str)
{
	uint32_t num_chars = 0;
	while (str[num_chars] != '\0') {
		while (!os_mailbox_write(mb, &str[num_chars]));
	}

	return num_chars;
}

bool os_char_buffer_read_ch(mailbox_t *mb, char *ch)
{
	return os_mailbox_read(mb, ch);
}

char os_char_buffer_read_ch_blocking(mailbox_t *mb)
{
	char ch = '\0';
	while (!os_mailbox_read(mb, &ch));

	return ch;
}

uint32_t os_char_buffer_read_buf(mailbox_t *mb,
                                 char *buf,
                                 uint32_t buf_len)
{
	return os_mailbox_read_multiple(mb, buf, buf_len);
}

void os_char_buffer_read_buf_blocking(mailbox_t *mb,
                                      char *buf,
                                      uint32_t buf_len)
{
	uint32_t pos = 0;

	while (pos <= buf_len) {
		pos += os_mailbox_read_multiple(mb, &buf[pos], buf_len - pos);
	}
}



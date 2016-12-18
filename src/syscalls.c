/**
 * @file
 *
 * TODO Add file description.
 *
 */

#include <reent.h> // Function declarations & reent structure.
#include <errno.h> // Error codes.

#include <libopencm3/cm3/cortex.h> // For the atomic macros.

#include "diag/diag.h" // For the diag log functions.

/** Minimal implementation of environment variables. */
char *__env[1] = { 0 };
/** Minimal implementation of environment variables. */
char **environ = __env;

/**
 * Declared in linker script. Address in RAM right after the space for
 * statically allocated variables.
 */
extern char end;

/**
 * The current top of the heap.
 */
static char *heap_end = &end;

void _exit(int status);


/**
 * Called when a process exits.
 *
 * @note Since MourOS doesn't have processes, this should never return.
 *
 * @param status The return code of the process.
 */
void _exit(int status)
{
	(void) status;

	while (1);
}

/**
 * Closes the file identified by filedes.
 *
 * @note Since files aren't implemented yet, this function does nothing and
 *       always fails.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param filedes File descriptor of the file to be closed.
 * @return Returns zero on success, -1 on error.
 */
int _close_r(struct _reent *reent, int filedes)
{
	diag_syscall_close(filedes);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Starts a new process.
 *
 * @note Since we don't have processes, this function does nothing and always
 *       fails.
 *
 * @param reent Pointer to the reentrancy structure.
 * @param path  File path of the file to be executed as the new process.
 * @param argv  Argument array for the new process.
 * @param envp  Environment variable array for the new process.
 * @return On success does not return. Returns -1 on error.
 */
int _execve_r(struct _reent *reent,
              const char *path,
              char * const *argv,
              char * const *envp)
{
	diag_syscall_execve((uint32_t) path,
	                    (uint32_t) argv,
	                    (uint32_t) envp);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Manipulates the supplied file descriptor.
 *
 * @note Since files aren't implemented yet, this function does nothing and
 *       always fails.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param filedes File descriptor to be manipulated.
 * @param cmd     Operation to be performed.
 * @param arg     Argument for the operation.
 * @return Returns value depends on cmd. Always returns -1 on errror.
 */
int _fcntl_r(struct _reent *reent, int filedes, int cmd, int arg)
{
	diag_syscall_fcntl(filedes, cmd, arg);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Creates a child process.
 *
 * @note Since we don't have processes, this function does nothing and always
 *       fails.
 *
 * @param reent Pointer to the reentrancy structure.
 * @return On success the PID of the child process is returned in the parent,
 *         and zero is returned in the child. -1 is returned on error.
 */
int _fork_r(struct _reent *reent)
{
	diag_syscall_fork();

	reent->_errno = ENOSYS;
	return -1;
}

/**
 * Gets information about file. Fills the struct pointed to by buf.
 *
 * @note Since files aren't implemented yet, this function does nothing and
 *       always fails.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param filedes File descriptor of the file.
 * @param buf     Pointer to the stat structure to be filled.
 * @return Returns zero on success, -1 on error.
 */
int _fstat_r(struct _reent *reent, int filedes, struct stat *buf)
{
	diag_syscall_fstat(filedes, (uint32_t) buf);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Returns the PID of the calling process. Since we don't have processes, it
 * always returns 1.
 *
 * @param reent Pointer to the reentrancy structure.
 * @return Always returns 1.
 */
int _getpid_r(struct _reent *reent)
{
	(void) reent;

	diag_syscall_getpid();

	return 1;
}

/**
 * Checks if the file descriptor refers to a terminal.
 *
 * @note Currently always returns 1.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param filedes The file descriptor to be checked.
 * @return Returns 1 if the file descriptor refers to a terminal, 0 otherwise.
 */
int _isatty_r(struct _reent *reent, int filedes)
{
	diag_syscall_isatty(filedes);

	reent->_errno = EBADF;
	return 1;
}

/**
 * Sends a signal to a process.
 *
 * @note Since we don't have processes, this will always fail.
 *
 * @param reent Pointer to the reentrancy structure
 * @param pid   PID of the target process.
 * @param sig   Signal to be sent.
 * @return Returns zero on success, -1 otherwise.
 */
int _kill_r(struct _reent *reent, int pid, int sig)
{
	diag_syscall_kill(pid, sig);

	reent->_errno = ESRCH;
	return -1;
}

/**
 * Creates a new hard link to an existing file.
 *
 * @note Until a filesystem is implemented, this will fail.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param oldpath Filepath of the existing file.
 * @param newpath Filepath of the new link.
 * @return Returns zero on success, -1 otherwise.
 */
int _link_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
	diag_syscall_link((uint32_t ) oldpath, (uint32_t ) newpath);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Repositions read/write file offset.
 *
 * @param reent   Pointer to the reentrancy structure.
 * @param filedes File descriptor of the file in which to seek.
 * @param offset  Byte offset input.
 * @param whence  How the final offset is calculated based on the input offset
 *                parameter.
 * @return Returns the resulting offset location on success. Returns -1 on
 *         error.
 */
_off_t _lseek_r(struct _reent *reent, int filedes, _off_t offset, int whence)
{
	diag_syscall_lseek(filedes, offset, whence);

	reent->_errno = EBADF;
	return (_off_t) -1;
}

/**
 * Creates a directory.
 *
 * @param reent    Pointer to the reentrancy structure.
 * @param pathname The pathname of the new directory.
 * @param mode     The mode for the new directory.
 * @return Returns zero on success, -1 otherwise.
 */
int _mkdir_r(struct _reent *reent, const char *pathname, int mode)
{
	diag_syscall_mkdir((uint32_t) pathname, mode);

	reent->_errno = EPERM;
	return -1;
}

/**
 * Opens and possibly creates a file.
 *
 * @param reent    Pointer to the reentrancy structure.
 * @param pathname The pathname of the file to be opened/created.
 * @param flags    Option flags.
 * @param mode     File mode.
 * @return File descriptor of the new file.
 */
int _open_r(struct _reent *reent, const char *pathname, int flags, int mode)
{
	diag_syscall_open((uint32_t) pathname, flags, mode);

	reent->_errno = EACCES;
	return -1;
}

/**
 * Reads up to count bytes from the provided file descriptor.
 *
 * @param reent   The reentrancy structure.
 * @param filedes The file descriptor.
 * @param buf     Pointer to the buffer to store the read data.
 * @param count   The size of buf.
 * @return The number of bytes actually read.
 */
_ssize_t _read_r(struct _reent *reent, int filedes, void *buf, size_t count)
{
	diag_syscall_read(filedes, (uint32_t) buf, count);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Changes the name or location of file.
 *
 * @param reent   The reentrancy structure.
 * @param oldpath The current file path.
 * @param newpath The new file path.
 * @return Returns zero on success, -1 on error.
 */
int _rename_r(struct _reent *reent, const char *oldpath, const char *newpath)
{
	diag_syscall_rename((uint32_t) oldpath, (uint32_t) newpath);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Changes the data segment size.
 *
 * @param reent     The reentrancy structure.
 * @param increment The number of bytes to increase/decrease the data segment
 *                  by.
 * @return Returns a pointer to the start of the new memory or -1 on error.
 */
void *_sbrk_r(struct _reent *reent, ptrdiff_t increment)
{
	diag_syscall_sbrk((uint32_t) &end, (uint32_t) heap_end, increment);

	CM_ATOMIC_CONTEXT();

	char *main_stack_pointer = 0;
	asm("mrs %[msp_content], msp"
	    : [msp_content] "=r" (main_stack_pointer));


	if (heap_end + increment > main_stack_pointer)
	{
		reent->_errno = ENOMEM;
		return (void *) -1;
	}

	char *prev_heap_end = heap_end;

	heap_end += increment;

	return (void *) prev_heap_end;
}

/**
 * Gets the file status.
 *
 * @param reent    The reentrancy structure.
 * @param pathname The pathname of the file.
 * @param buf      Pointer to the struct to store the file status.
 * @return Returns zero on success, -1 on error.
 */
int _stat_r(struct _reent *reent, const char *pathname, struct stat *buf)
{
	diag_syscall_stat((uint32_t) pathname, (uint32_t) buf);

	reent->_errno = EBADF;
	return -1;
}

/**
 * Gets process times.
 *
 * @param reent The reentrancy structure.
 * @param buf   Pointer to the struct to hold the current process times.
 * @return Returns a number of clock ticks since an arbitrary point in the past.
 */
_CLOCK_T_ _times_r(struct _reent *reent, struct tms *buf)
{
	diag_syscall_times((uint32_t) buf);

	reent->_errno = ENOSYS;
	return (_CLOCK_T_) -1;
}

/**
 * Deletes a "name" and possibly a file it refers to.
 *
 * @param reent    The reentrancy structure.
 * @param pathname The pathname of the file to be deleted.
 * @return Returns zero on success, -1 on error.
 */
int _unlink_r(struct _reent *reent, const char *pathname)
{
	diag_syscall_unlink((uint32_t) pathname);

	reent->_errno = ENOENT;
	return -1;
}

/**
 * Suspends execution of the calling process until one of its children
 * terminates.
 *
 * @param reent  The reentrancy structure.
 * @param status Pointer to an integer storing the child process exit status.
 * @return Returns the terminated process ID on success or -1 on error.
 */
int _wait_r(struct _reent *reent, int *status)
{
	diag_syscall_wait((uint32_t) status);

	reent->_errno = ECHILD;
	return -1;
}

/**
 * Writes to the supplied file descriptor.
 *
 * @param reent   The reentrancy structure.
 * @param filedes The file descriptor to be written to.
 * @param buf     Pointer to the data to be written.
 * @param count   The size of data in buf.
 * @return Returns the number of bytes actually written to filedes or -1 on
 *         error.
 */
_ssize_t _write_r(struct _reent *reent,
                  int filedes,
                  const void *buf,
                  size_t count)
{
	diag_syscall_write(filedes, (uint32_t) buf, count);

	reent->_errno = EBADF;
	return -1;
}


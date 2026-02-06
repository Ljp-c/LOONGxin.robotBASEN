/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */

#include "bsp.h"

#if !BSP_USE_FS

/**
 *  These are the Newlib dependent reentrant version.
 */

#include <stdio.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>

#include <newlib.h>

#ifndef _NEWLIB_SYSCALLS
#define NEED_SYSCALLS
#endif

//-------------------------------------------------------------------------------------------------
// open() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
int open(const char *file, int flags, ...)
{
    errno = EIO;
    return -1;
}
#endif

int _open_r(struct _reent *ptr __attribute__((unused)), const char *buf, int oflag, int mode)
{
    errno = EIO;
    return -1;
}

//-------------------------------------------------------------------------------------------------
// close() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
int close(int fd)
{
    return 0;
}
#endif

int _close_r(struct _reent *ptr __attribute__((unused)), int fd)
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
// read() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
ssize_t read(int fd, void *buf, size_t len)
{
    return 0;
}
#endif

/**
 * fd == 0: stdin
 */
ssize_t _read_r(struct _reent *ptr __attribute__((unused)), int fd, void *buf, size_t nbytes)
{
    if (fd == 0)
    {
        if ((buf) && (nbytes > 0))
        {
	#if 1
            extern ssize_t console_gets(void *buf, size_t nbytes);

            return console_gets((char *)buf, nbytes);

	#else
            extern char console_getch(void);

            int count = 0;
            char ch, *ptr = (char *)buf;

            while (count < nbytes)
            {
                ch = console_getch();

                if (ch != -1)
                {
                	*ptr = ch;
                	ptr++;
                	count++;
                }
            }

            return (ssize_t)count;

	#endif
        }

        return 0;
    }

    errno = ENOTSUP;
    return (ssize_t)-1;
}

//-------------------------------------------------------------------------------------------------
// write() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
ssize_t write(int fd, const void *buf, size_t len)
{
    return 0;
}
#endif

/*
 * fd == 1: stdout
 */
ssize_t _write_r(struct _reent *ptr __attribute__((unused)), int fd, const void *buf, size_t nbytes)
{
    if (fd == 1)
    {
        if ((buf) && (nbytes > 0))
        {
	#if 1
            extern ssize_t console_puts(char *buf, size_t nbytes);

            return console_puts((char *)buf, nbytes);

	#else
            extern void console_putch(char ch);

            size_t count = nbytes;
            char *ptr = (char *)buf;

            while (count > 0)
            {
                console_putch(*ptr);
                ptr++;
                count--;
            }

            return (_ssize_t)nbytes;
	#endif
        }

        return 0;
    }

    errno = ENOTSUP;
    return (_ssize_t)-1;
}

//-------------------------------------------------------------------------------------------------
// lseek() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
off_t lseek(int fd, off_t offset, int whence)
{
    errno = ESPIPE;
    return (off_t)-1;
}
#endif

off_t _lseek_r(struct _reent *ptr __attribute__((unused)), int fd, off_t offset, int whence)
{
    errno = ESPIPE;
    return (off_t)-1;
}

//-------------------------------------------------------------------------------------------------
// fstat() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
int fstat(int fildes, struct stat *buf)
{
    errno = EIO;
    return -1;
}
#endif

int _fstat_r(struct _reent *ptr __attribute__((unused)), int fd, struct stat *buf)
{
    errno = EIO;
    return -1;
}

//-------------------------------------------------------------------------------------------------
// isatty() function
//-------------------------------------------------------------------------------------------------

#ifdef NEED_SYSCALLS
int isatty(int fd)
{
    return 1;
}
#endif

int _isatty_r(struct _reent *ptr __attribute__((unused)), int fd)
{
    return 1;
}

//-------------------------------------------------------------------------------------------------
// memory functions
//-------------------------------------------------------------------------------------------------

/**
 * TODO 必须实现内存相关函数
 */
extern void *malloc(size_t size);
extern void *calloc(size_t nmemb, size_t size);
extern void *realloc(void *ptr, size_t size);
extern void free(void *ptr);

void *_malloc_r(struct _reent *ignored __attribute__((unused)), size_t size)
{
    return malloc(size);
}

void *_realloc_r(struct _reent *ignored __attribute__((unused)), void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void *_calloc_r(struct _reent *ignored __attribute__((unused)), size_t elements, size_t size)
{
    return calloc(elements, size);
}

void _free_r(struct _reent *ignored __attribute__((unused)), void *ptr)
{
    return free(ptr);
}

//-------------------------------------------------------------------------------------------------

#endif // #if !BSP_USE_FS

/*
 * @@ END
 */

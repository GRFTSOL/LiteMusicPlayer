/*
 * Buffered file io for ffmpeg system
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "avformat.h"
#include <stdio.h>

/* standard file protocol */

static int file_open(URLContext *h, const char *filename, int flags)
{
	FILE *fp;
	const char *openflag;

    strstart(filename, "file:", &filename);

    if (flags & URL_WRONLY) {
		openflag = "wb";
        // access = O_CREAT | O_TRUNC | O_WRONLY;
    } else {
        // access = O_RDONLY;
		openflag = "rb";
    }
    fp = fopen(filename, openflag);
    if (!fp)
        return -ENOENT;
    h->priv_data = (void *)fp;
    return 0;
}

static int file_read(URLContext *h, unsigned char *buf, int size)
{
	FILE *fp = (FILE *)h->priv_data;
    return fread(buf, 1, size, fp);
}

static int file_write(URLContext *h, unsigned char *buf, int size)
{
	FILE *fp = (FILE *)h->priv_data;
	return fwrite(buf, 1, size, fp);
}

/* XXX: use llseek */
static offset_t file_seek(URLContext *h, offset_t pos, int whence)
{
	FILE *fp = (FILE *)h->priv_data;
	if (fseek(fp, pos, whence) != 0)
		return -1;
	return ftell(fp);
}

static int file_close(URLContext *h)
{
	FILE *fp = (FILE *)h->priv_data;
    return fclose(fp);
}

URLProtocol file_protocol = {
    "file",
    file_open,
    file_read,
    file_write,
    file_seek,
    file_close,
};

/* pipe protocol */

static int pipe_open(URLContext *h, const char *filename, int flags)
{
//     int fd;
// 
//     if (flags & URL_WRONLY) {
//         fd = 1;
//     } else {
//         fd = 0;
//     }
//     h->priv_data = (void *)fd;
    return 0;
}

static int pipe_read(URLContext *h, unsigned char *buf, int size)
{
//     int fd = (int)h->priv_data;
//     return read(fd, buf, size);
	return 0;
}

static int pipe_write(URLContext *h, unsigned char *buf, int size)
{
//     int fd = (int)h->priv_data;
//     return write(fd, buf, size);
	return 0;
}

static int pipe_close(URLContext *h)
{
    return 0;
}

URLProtocol pipe_protocol = {
    "pipe",
    pipe_open,
    pipe_read,
    pipe_write,
    NULL,
    pipe_close,
};

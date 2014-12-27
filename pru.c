/*-
 * Copyright (c) 2014 Rui Paulo <rpaulo@felyko.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <Block.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <libpru.h>
#include <pru-private.h>
#include <ti-pru.h>

int libpru_debug = 0;

pru_type_t
pru_name_to_type(const char *name)
{
	if (strcasecmp(name, "ti") == 0)
		return PRU_TYPE_TI;
	else
		return PRU_TYPE_UNKNOWN;
}

static void *
pru_handle_intr(void *arg)
{
	pru_t pru;
	int ret;

	pru = arg;
	while ((ret = pru->check_intr(pru)) > 0) {
#ifdef __BLOCKS__
		if (pru->intr_block)
			pru->intr_block(ret);
		else
#endif
		if (pru->intr_func)
			pru->intr_func(ret);
	}

	return (NULL);
}

pru_t
pru_alloc(pru_type_t type)
{
	pru_t pru;
	int saved_errno;
	char *pruenv;

	pruenv = getenv("PRU_DEBUG");
	if (pruenv)
		libpru_debug = atoi(pruenv);
	DPRINTF("type %d\n", type);
	pru = malloc(sizeof(*pru));
	if (pru == NULL)
		return NULL;
	bzero(pru, sizeof(*pru));
	pru->type = type;
	switch (pru->type) {
	case PRU_TYPE_TI:
		if (ti_initialise(pru) != 0) {
			saved_errno = errno;
			free(pru);
			errno = saved_errno;
			return NULL;
		}
		break;
	case PRU_TYPE_UNKNOWN:
		pru_free(pru);
		errno = EINVAL;
		return NULL;
	}
	/*
	 * Create a thread to handle interrupts.
	 */
	if (pthread_create(&pru->thread, NULL, pru_handle_intr, pru) != 0) {
		pru_free(pru);
		errno = EINVAL; /* XXX */
		return NULL;
	}
	DPRINTF("pru %p allocated and initialised\n", pru);

	return pru;
}

void
pru_free(pru_t pru)
{
	DPRINTF("pru %p\n", pru);
	pru->deinit(pru);
#ifdef __BLOCKS__
	if (pru->intr_block)
		Block_release(pru->intr_block);
#endif
	free(pru);
}

#ifdef __BLOCKS__
void
pru_set_handler(pru_t pru, void (^block)(int))
{
	pru->intr_block = Block_copy(block);

}
#endif

void
pru_set_handler_f(pru_t pru, void (*f)(int))
{
	DPRINTF("function %p\n", f);
	pru->intr_func = f;
}

int
pru_reset(pru_t pru, unsigned int pru_number)
{
	return pru->reset(pru, pru_number);
}

int
pru_disable(pru_t pru, unsigned int pru_number)
{
	return pru->disable(pru, pru_number);
}

int
pru_enable(pru_t pru, unsigned int pru_number)
{
	return pru->enable(pru, pru_number);
}

int
pru_upload(pru_t pru, unsigned int pru_number, const char *file)
{
	int error, saved_errno;
	int fd;
	struct stat sb;
	char *buffer;

	DPRINTF("pru %d file %s\n", pru_number, file);
	fd = open(file, O_RDONLY);
	if (fd < 0)
		return errno;
	if (fstat(fd, &sb) != 0) {
		saved_errno = errno;
		close(fd);
		return saved_errno;
	}
	buffer = mmap(0, (size_t)sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (buffer == MAP_FAILED) {
		saved_errno = errno;
		close(fd);
		return saved_errno;
	}
	error = pru->upload_buffer(pru, pru_number, buffer,
	    (size_t)sb.st_size);
	munmap(buffer, (size_t)sb.st_size);
	close(fd);

	return error;
}

int
pru_wait(pru_t pru, unsigned int pru_number)
{
	return pru->wait(pru, pru_number);
}

uint32_t
pru_read_imem(pru_t pru, unsigned int pru_number, uint32_t mem)
{
	return pru->read_imem(pru, pru_number, mem);
}

uint32_t
pru_read_reg(pru_t pru, unsigned int pru_number, uint32_t reg)
{
	return pru->read_reg(pru, pru_number, reg);
}

int
pru_write_reg(pru_t pru, unsigned int pru_number, uint32_t reg, uint32_t val)
{
	return pru->write_reg(pru, pru_number, reg, val);
}

int
pru_disassemble(pru_t pru, uint32_t opcode, char *buf, size_t len)
{
	return pru->disassemble(pru, opcode, buf, len);
}

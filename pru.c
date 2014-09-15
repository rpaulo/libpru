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

pru_type_t
pru_name_to_type(const char *name)
{
	if (strcasecmp(name, "am18xx") == 0)
		return PRU_TYPE_AM18XX;
	else if (strcasecmp(name, "am33xx") == 0)
		return PRU_TYPE_AM33XX;
	else
		return PRU_TYPE_UNKNOWN;
}

static void *
pru_handle_intr(void *arg)
{
	pru_t pru;

	pru = arg;
	while (pru->check_intr(pru) > 0) {
#ifdef __BLOCKS__
		if (pru->intr_block)
			pru->intr_block();
		else
#endif
		if (pru->intr_func)
			pru->intr_func();
	}

	return (NULL);
}

pru_t
pru_alloc(pru_type_t type)
{
	pru_t pru;
	int savederrno;

	pru = malloc(sizeof(*pru));
	if (pru == NULL)
		return NULL;
	bzero(pru, sizeof(*pru));
	pru->type = type;
	switch (pru->type) {
	case PRU_TYPE_AM18XX:
	case PRU_TYPE_AM33XX:
		if (ti_initialise(pru) != 0) {
			savederrno = errno;
			free(pru);
			errno = savederrno;
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

	return pru;
}

void
pru_free(pru_t pru)
{
	pru->deinit(pru);
#ifdef __BLOCKS__
	if (pru->intr_block)
		Block_release(pru->intr_block);
#endif
	free(pru);
}

#ifdef __BLOCKS__
void
pru_set_handler(pru_t pru, void (^block)(void))
{
	pru->intr_block = Block_copy(block);

}
#endif

void
pru_set_handler_f(pru_t pru, void (*f)(void))
{
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
	int error;
	int fd;
	struct stat sb;
	char *buffer;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return errno;
	fstat(fd, &sb);
	buffer = mmap(0, (size_t)sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (buffer == NULL) {
		close(fd);
		return errno;
	}
	error = pru->upload_buffer(pru, pru_number, buffer, (size_t)sb.st_size);
	munmap(buffer, (size_t)sb.st_size);
	close(fd);
	
	return error;
}

int
pru_wait(pru_t pru, unsigned int pru_number)
{
	return pru->wait(pru, pru_number);
}

int
pru_get_property(pru_t pru)
{
	(void)pru;
	return 0;
}


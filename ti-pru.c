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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/cdefs.h>

#include <libpru.h>
#include <pru-private.h>
#include <ti-pru.h>

static uint32_t
ti_reg_read_4(char *mem, unsigned int reg)
{
	return *(volatile uint32_t *)(void *)(mem + reg);
}

static void
ti_reg_write_4(char *mem, unsigned int reg, uint32_t value)
{
	*(volatile uint32_t *)(void *)(mem + reg) = value;
}

static int
ti_disable(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;

	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	ti_reg_write_4(pru->mem, reg, CTL_REG_DISABLE);

	return 0;
}

static int
ti_enable(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;

	if (pru_number > 2)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	ti_reg_write_4(pru->mem, reg, CTL_REG_ENABLE);

	return 0;
}

static int
ti_reset(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;

	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	ti_reg_write_4(pru->mem, reg, CTL_REG_RESET);

	return 0;
}

static int
ti_upload(pru_t pru, unsigned int pru_number, const char *buffer, size_t size)
{
	unsigned int *iram;

	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV) {
		if (size > AM18XX_IRAM_SIZE)
			return -1;
		iram = (unsigned int *)AM18XX_PRUnIRAM(pru_number);
		memset(iram, 0, AM18XX_IRAM_SIZE);
	} else {
		if (size > AM33XX_IRAM_SIZE)
			return -1;
		iram = (unsigned int *)AM33XX_PRUnIRAM(pru_number);
		memset(iram, 0, AM33XX_IRAM_SIZE);
	}
	memcpy(iram, buffer, size);

	return 0;
}

static int
ti_wait(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;
	struct timespec ts;

	/* 0.5 seconds */
	ts.tv_nsec = 500000000;
	ts.tv_sec = 0;
	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	while (ti_reg_read_4(pru->mem, reg) != 0x8000)
		nanosleep(&ts, NULL);

	return 0;
}

static int
ti_check_intr(pru_t pru)
{
	(void)pru;
	return 0;
}

int
ti_initialise(pru_t pru)
{
	size_t i;
	int fd = 0;
	char dev[64];
	size_t mmap_sizes[2] = { AM33XX_MMAP_SIZE, AM18XX_MMAP_SIZE };
	size_t mmap_size = 0;
	int saved_errno = 0;

	for (i = 0; i < 4; i++) {
		snprintf(dev, sizeof(dev), "/dev/pruss%zu", i);
		fd = open(dev, O_RDWR);
		if (errno == EPERM)
			break;
		if (fd > 0)
			break;
	}
	if (fd < 0)
		return EINVAL;
	pru->fd = fd;
	/* N.B.: The order matters. */
	for (i = 0; i < sizeof(mmap_sizes)/sizeof(mmap_sizes[0]); i++) {
		pru->mem = mmap(0, mmap_sizes[i], PROT_READ|PROT_WRITE,
		    MAP_SHARED, fd, 0);
		saved_errno = errno;
		if (pru->mem != MAP_FAILED) {
			mmap_size = mmap_sizes[i];
			break;
		}
	}
	if (pru->mem == MAP_FAILED) {
		close(pru->fd);
		errno = saved_errno;
		return -1;
	}
	/*
	 * Use the md_stor field to save the revision.
	 */
	if (ti_reg_read_4(pru->mem, AM18XX_INTC_REG) == AM18XX_REV)
		pru->md_stor[0] = AM18XX_REV;
	else if (ti_reg_read_4(pru->mem, AM33XX_INTC_REG) == AM33XX_REV)
		pru->md_stor[0] = AM33XX_REV;
	else {
		munmap(pru->mem, mmap_size);
		close(pru->fd);
		return EINVAL;
	}
	pru->disable = ti_disable;
	pru->enable = ti_enable;
	pru->reset = ti_reset;
	pru->upload_buffer = ti_upload;
	pru->wait = ti_wait;
	pru->check_intr = ti_check_intr;

	return 0;
}

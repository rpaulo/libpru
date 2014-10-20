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
#pragma once

#define	AM18XX_REV		0x4E825900
#define	AM33XX_REV		0x4E82A900

#define	AM18XX_IRAM_SIZE	0x00001000
#define AM18XX_INTC_REG		0x00004000
#define AM18XX_PRU0CTL_REG	0x00007000
#define AM18XX_PRU1CTL_REG	0x00007800
#define	AM18XX_PRUnCTL(n)	AM18XX_PRU0CTL_REG + n * 0x800
#define	AM18XX_PRU0IRAM_REG	0x00008000
#define	AM18XX_PRU1IRAM_REG	0x0000C000
#define	AM18XX_PRUnIRAM(n)	AM18XX_PRU0CTL_REG + n * AM18XX_IRAM_SIZE
#define	AM18XX_MMAP_SIZE	0x00007C00

#define	AM33XX_IRAM_SIZE	0x00002000
#define AM33XX_INTC_REG		0x00020000
#define AM33XX_PRU0CTL_REG	0x00022000
#define AM33XX_PRU1CTL_REG	0x00024000
#define	AM33XX_PRUnCTL(n)	AM33XX_PRU0CTL_REG + n * 0x2000
#define	AM33XX_PRU0IRAM_REG	0x00034000
#define	AM33XX_PRU1IRAM_REG	0x00038000
#define	AM33XX_PRUnIRAM(n)	AM33XX_PRU0IRAM_REG + n * AM33XX_IRAM_SIZE
#define	AM33XX_MMAP_SIZE	0x00040000

/* Control register */
#define	CTL_REG_RESET	        (1U << 0)	/* Clear to reset */
#define	CTL_REG_ENABLE		(1U << 1)
#define	CTL_REG_RUNSTATE	(1U << 15)

extern int ti_initialise(pru_t) __hidden;

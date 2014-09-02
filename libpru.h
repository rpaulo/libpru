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

struct pru;
typedef struct pru * pru_t;

#define	LIBPRU_VERSION "0.1"

typedef enum {
	PRU_TYPE_UNKNOWN,
	PRU_TYPE_AM18XX,
	PRU_TYPE_AM33XX,
} pru_type_t;

pru_type_t pru_name_to_type(const char *);
pru_t	pru_alloc(pru_type_t);
void	pru_free(pru_t);
#ifdef __BLOCKS__
void	pru_set_handler(pru_t, void (^handler)(void));
#endif
void	pru_set_handler_f(pru_t);
int	pru_reset(pru_t, unsigned int);
int	pru_disable(pru_t, unsigned int);
int	pru_enable(pru_t, unsigned int);
int	pru_upload(pru_t, unsigned int, const char *);
int	pru_wait(pru_t, unsigned int);
int	pru_get_property(pru_t);


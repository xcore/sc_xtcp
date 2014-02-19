// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>


/**
 * \addtogroup uip
 * {@
 */

/**
 * \defgroup uiparch Architecture specific uIP functions
 * @{
 *
 * The functions in the architecture specific module implement the IP
 * check sum and 32-bit additions.
 *
 * The IP checksum calculation is the most computationally expensive
 * operation in the TCP/IP stack and it therefore pays off to
 * implement this in efficient assembler. The purpose of the uip-arch
 * module is to let the checksum functions to be implemented in
 * architecture specific assembler.
 *
 */

/**
 * \file
 * Declarations of architecture specific functions.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2001, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: uip_arch.h,v 1.2 2006/06/07 09:15:19 adam Exp $
 *
 */

#ifndef __UIP_ARCH_H__
#define __UIP_ARCH_H__

#include <stdint.h>

/**
 * Carry out a 32-bit addition.
 *
 * Because not all architectures for which uIP is intended has native
 * 32-bit arithmetic, uIP uses an external C function for doing the
 * required 32-bit additions in the TCP protocol processing. This
 * function should add the two arguments and place the result in the
 * global variable uip_acc32.
 *
 * \note The 32-bit integer pointed to by the op32 parameter and the
 * result in the uip_acc32 variable are in network byte order (big
 * endian).
 *
 * \param op32 A pointer to a 4-byte array representing a 32-bit
 * integer in network byte order (big endian).
 *
 * \param op16 A 16-bit integer in host byte order.
 */
void uip_add32(uint8_t *op32, uint16_t op16);

/* Alternative faster checksum computation */
uint16_t chksum(uint16_t sum, const uint8_t *byte_data, uint16_t lengthInBytes);

void xtcp_copy_word(uint8_t*d, uint8_t* s);

void xtcp_swap_words(uint8_t* a, uint8_t* b);

int xtcp_compare_words(const uint8_t* a, const uint8_t* b);

void xtcp_increment_word(uint8_t* a);
/** @} */
/** @} */

#endif /* __UIP_ARCH_H__ */

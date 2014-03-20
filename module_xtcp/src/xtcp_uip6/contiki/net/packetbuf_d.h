/*****************************************************************************
*
* Filename:         packetbuf_d.h
* Author:           Christian Schlittler
* Version:          1.0
* Creation date:	18.11.2013
*
* Copyright:        Christian Schlittler, christian.schlittler@gmx.ch
*
* Project:			XMOS and 6LoWPAN
* Target:			XMOS sliceKIT with RFSlice extension
* Compiler:
*
* -----------------------------------------------------------------------------
*
* History:
*
* -----------------------------------------------------------------------------
*
* Usage:
*
**************************************************************************** */

#ifndef PACKETBUF_D_H_
#define PACKETBUF_D_H_

#include <stdint.h>
#include <xccompat.h>
#include "net/rime/rimeaddr.h"

enum{
    PACKETBUF_D_ADDR_SENDER,
};

int               packetbuf_d_set_addr(uint8_t type, NULLABLE_ARRAY_OF(const rimeaddr_t, addr));
#if !__XC__
const rimeaddr_t *packetbuf_d_addr(uint8_t type);
#endif /* __XC__ */
#endif /* PACKETBUF_D_H_ */

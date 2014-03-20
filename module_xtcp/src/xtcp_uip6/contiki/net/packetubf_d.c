/*****************************************************************************
*
* Filename:         packetubf_d.c
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
* The package buffer is based on the sources of contiki-os. The problem was, that
* contiki-os runs on a single core, shared memory cpu. But, for the XMOS implementation
* this is not true. The MAC and the IPv6 stack could be on different tiles with
* no shared memory.
* The RPL alogrithm needs access to the sender MAC address and normally this is
* done by the shared packet buffer. So, here we need to copy the address in a
* dummy packet buffer.
*
* -----------------------------------------------------------------------------
*
* Usage:
*
**************************************************************************** */

#include <string.h>
#include "net/packetbuf_d.h"

static rimeaddr_t packetbuf_d_addr_sender;

int packetbuf_d_set_addr(uint8_t type, const rimeaddr_t *addr){

    switch(type){
    case PACKETBUF_D_ADDR_SENDER:
        memcpy(&packetbuf_d_addr_sender, addr, sizeof(rimeaddr_t));
        return 1;
        break;

    default:
        return 0;
        break;
    }

}

/*----------------------------------------------------------------------------*/
const rimeaddr_t *packetbuf_d_addr(uint8_t type){
    if(type == PACKETBUF_D_ADDR_SENDER){
        return &packetbuf_d_addr_sender;
    } else {
        return NULL;
    }
}

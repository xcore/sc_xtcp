/*****************************************************************************
*
* Filename:         uip_arch.c
* Author:           Christian Schlittler
* Version:          1.0
* Creation date:	03.10.2013
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

#include <xclib.h>
#include "uip_arch.h"

#include <uip.h>

#if UIP_CONF_LOGGING
#include <print.h>
#define PRINTSTR    printstr
#define PRINTSTRLN  printstrln
#else
#define     PRINTSTR(...)
#define     PRINTSTRLN(...)
#endif

void uip_log(char msg[]){
    PRINTSTR("uIP log message: ");
    PRINTSTRLN(msg);
}

/* Useful operations on 4-byte words misaligned by 2 */
void xtcp_swap_words(uint8_t* a, uint8_t* b)
{
	short c;

	c = *(short*)(&a[0]);
	*(short*)(&a[0]) = *(short*)(&b[0]);
	*(short*)(&b[0]) = (short)c;

	c = *(short*)(&a[2]);
	*(short*)(&a[2]) = *(short*)(&b[2]);
	*(short*)(&b[2]) = (short)c;
}

void xuip_ip4addr_copy(uip_ip4addr_t *src, uip_ip4addr_t *dst){
	xtcp_swap_words(src->u8, dst->u8);
}

void xtcp_increment_word(uint8_t* a)
{
	unsigned s = ((*(short*)(&a[2])) << 16) + *(short*)(&a[0]);
	s = byterev(byterev(s)+1);
	*(short*)(&a[0]) = (short)s;
	*(short*)(&a[2]) = (short)(s >> 16);
}

__attribute__ ((noinline))
void xtcp_copy_word(uint8_t* d, uint8_t* s)
{
	*(short*)(&d[0]) = *(short*)(&s[0]);
	*(short*)(&d[2]) = *(short*)(&s[2]);
}

__attribute__ ((noinline))
int xtcp_compare_words(const uint8_t* a, const uint8_t* b)
{
	return (*(short*)(&a[0]) == *(short*)(&b[0])) &&
		   (*(short*)(&a[2]) == *(short*)(&b[2]));
}

int xuip_ipaddr_cmp(uip_ip4addr_t *a, uip_ip4addr_t *b){
	return xtcp_compare_words(a->u8, b->u8);
}

#if UIP_SLIDING_WINDOW
static int xtcp_get_word(const u8_t *a) {
  unsigned int aw = ((*(unsigned short*)(&a[2])) << 16) + *(unsigned short*)(&a[0]);
  return byterev(aw);
}
/*static void xtcp_put_word(const u8_t *a, unsigned int s) {
  *(short*)(&a[0]) = (short)s;
  *(short*)(&a[2]) = (short)(s >> 16);
  }*/

#endif

/* -----------------------------------------------------------------------------
 *
 *  Alternative faster uip_addition
 *
 * -------------------------------------------------------------------------- */
extern uint8_t uip_acc32[4];		// located in uip6.c
__attribute__ ((noinline))
void uip_add32(uint8_t *op32, uint16_t op16) {
	  unsigned int *res = (unsigned int *)uip_acc32;
	  unsigned int x = ((*(unsigned short*)(&op32[2])) << 16) + *(unsigned short*)(&op32[0]);
	  x = byterev(x);
	  *res = byterev(x + op16);
}

/* -----------------------------------------------------------------------------
 *
 *  Alternative faster checksum computation
 *
 * -------------------------------------------------------------------------- */

static int onesReduce(unsigned int sum, int carry) {
    sum = (sum & 0xffff) + (sum >> 16) + carry;
    return (sum & 0xffff) + (sum >> 16);
}

uint16_t chksum(uint16_t sum, const uint8_t *byte_data, uint16_t lengthInBytes) {
    int i;
    short* data = (short*)byte_data;
    unsigned s = sum;
    for(i = 0; i < (lengthInBytes>>1); i++) {
        s += byterev(data[i]) >> 16;
    }
    if (lengthInBytes & 1) {
        s += byte_data[2*i] << 8;
    }
    sum = onesReduce(s, 0);
    return sum;
}

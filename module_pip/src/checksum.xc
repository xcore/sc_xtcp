// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include <xclib.h>
#include <print.h>
#include "checksum.h"

static int onesReduce(unsigned int sum, int carry) {
    sum = (sum & 0xffff) + (sum >> 16) + carry;
    return (sum & 0xffff) + (sum >> 16);
}

unsigned onesAdd(unsigned int x, unsigned int y) {
    unsigned int carry, sum;
    asm("ladd %0,%1,%2,%3,%4" : "=r" (carry), "=r" (sum) : "r" (x), "r" (y), "r" (0));
    return onesReduce(sum, carry);
}

unsigned onesChecksum(unsigned int sum, unsigned short data[], int begin, int lengthInBytes) {
    int i;
    for(i = begin; i < begin + (lengthInBytes>>1); i++) {
        sum += byterev(data[i]) >> 16;
    }
    if (lengthInBytes & 1) {
        sum += (data, unsigned char[])[2*i] << 8;
    }
    sum = onesReduce(sum, 0);
    return byterev((~sum) & 0xffff) >> 16;
}

// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <checksum.h>

static void onesReduce(int sum, int carry) {
    sum = (sum & 0xffff) + (sum >> 16) + carry;
    return (sum & 0xffff) + (sum >> 16);
}

unsigned onesAdd(unsigned int x, unsigned int y) {
    unsigned int h, l;
    {h, l} = ladd(x, y, 0);
    return onesReduce(l, h);
}

unsigned onesChecksum(unsigned int sum, unsigned short data[], int begin, int end) {
    for(int i = begin; i < end; i++) {
        sum += byterev(data[i]) >> 16;
    }
    sum = onesReduce(sum, 0);
    return byterev((~sum) & 0xffff) >> 16;
}

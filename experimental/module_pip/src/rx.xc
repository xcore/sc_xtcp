// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "rx.h"
#include "config.h"

unsigned getIntUnaligned(unsigned short packet[], int offset) {
    unsigned x = (packet, unsigned char[])[offset] << 24 |
        (packet, unsigned char[])[offset+1] << 16 |
        (packet, unsigned char[])[offset+2] << 8 |
        (packet, unsigned char[])[offset+3] << 0;
    return x;
}

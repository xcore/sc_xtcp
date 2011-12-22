// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

void txInt(int offset, int x);
void txShort(int offset, int x);
void txShortZeroes(int offset, int len);
void txByte(int offset, int x);
void txData(int offset, char data[], int dataOffset, int n);
unsigned shortrev(unsigned x);

extern short txbuf[400];      // THis is not good - for checksum only. TODO.

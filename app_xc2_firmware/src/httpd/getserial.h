// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>



// Retrieves Serial Number from OTP
// - serialNum - destination of serialNum
// - returns 0 for success
#ifdef __XC__
int getSerialNum(unsigned &serialNum);
#else
int getSerialNum(unsigned *serialNum);
#endif


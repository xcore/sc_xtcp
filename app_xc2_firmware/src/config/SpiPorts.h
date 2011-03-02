// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef HAVE_SPIPORTS_H_
#define HAVE_SPIPORTS_H_
#include <xs1.h>

extern out port p_ss;
extern out port p_sclk;
extern in buffered port:8 p_miso;
extern out buffered port:8 p_mosi;
extern clock b_spi;
extern out port p_rdy;


extern int spiMasterInit(void);
extern int deviceIdSPIFlash(unsigned int& pDeviceId);

#endif /* HAVE_SPIPORTS_H_ */


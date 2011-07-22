/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    SpiPorts.h
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   
#ifndef HAVE_SPIPORTS_H_
#define HAVE_SPIPORTS_H_

extern out port p_ss;
extern out port p_sclk;
extern in buffered port:8 p_miso;
extern out buffered port:8 p_mosi;
extern clock b_spi;
extern out port p_rdy;


extern int spiMasterInit(void);
extern int deviceIdSPIFlash(unsigned int& pDeviceId);

#endif /* HAVE_SPIPORTS_H_ */


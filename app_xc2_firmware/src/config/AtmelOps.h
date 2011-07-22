/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    AtmelOps.h
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
#ifndef HAVE_ATMELOPS_H_
#define HAVE_ATMELOPS_H_

int atmel_eraseAll(void);
int atmel_programPage(unsigned int,unsigned char[]);
int atmel_readPage(unsigned int,unsigned char[]);
void atmel_endSPIFlash(void);
int atmel_getBytesInPage();
int atmel_startWrite(void);
int atmel_endWrite(void);

int atmel_eraseOne(int i);
#endif /* HAVE_ATMELOPS_H_ */

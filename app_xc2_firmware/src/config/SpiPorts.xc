/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    SpiPorts.xc
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
#include <platform.h>
#include <xclib.h>
#include <xs1.h>

on stdcore[0]: out port p_ss = XS1_PORT_1B;
on stdcore[0]: out port p_sclk = XS1_PORT_1C;
on stdcore[0]: in buffered port:8 p_miso = XS1_PORT_1A;
on stdcore[0]: out buffered port:8 p_mosi = XS1_PORT_1D;
on stdcore[0]: clock b_spi = XS1_CLKBLK_1;
on stdcore[0]: out port p_rdy = XS1_PORT_1P;

int spiMasterInit(void)
{
  // put the ports in the correct way.
  //p_ss   <: 1;   // chip select to 1.
  //p_sclk <: 0;   // clock to zero.
  //p_mosi <: 0;   // Master out to zero.

  // put the ports in the correct way.
  p_mosi:1 <: 0;   // Master out to zero.
  sync(p_mosi);

  return( 0 );
}


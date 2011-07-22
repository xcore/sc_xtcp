/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    AtmelOps.xc
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
#include <print.h>
#include <xs1.h>
#include <xclib.h>
#include "AtmelOps.h"
#include "SpiPorts.h"


// Sectors are used for protection
static const int SPI_NUM_SECTORS         = 8;
static const int SPI_NUM_BYTES_IN_SECTOR = 65536;

static const int SPI_DEVICE_ID_MASK = 0xffffffff;
static const int SPI_DEVICE_ID = 0x1f440100;

static const int SPI_BYTES_IN_PAGE = 256;

// Write in progress status register mask bit
static const int SPI_WIP_BIT_MASK = 1;

// Commands
static const int SPI_WREN      = 0x06;   // Write Enable
static const int SPI_WRDI      = 0x04;   // Write Disable
static const int SPI_RDID      = 0x9F;   // Read Device ID
static const int SPI_RDSR      = 0x05;   // Read Status Register
static const int SPI_READ      = 0x03;   // Read Data Bytes
static const int SPI_READ_FAST = 0x0B;   // Read Data Bytes Fast
static const int SPI_PP        = 0x02;   // Page Program
static const int SPI_SE        = 0xD8;   // Sector Erase
static const int SPI_SP        = 0x36;   // Sector Protect
static const int SPI_SU        = 0x39;   // Sector Unprotect


// Write enable
// Normally don't need to be explicitly called
static void spiWriteEnable()
{
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_WREN));
  p_miso :> unsigned char spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
}

// Unprotect sector at given address
// Takes byte address not sector number!
// This is because some devices have uneven sector protection layout
static void spuUnprotect(unsigned addr)
{
  unsigned char spi_null;
  spiWriteEnable();
  addr = bitrev(addr) >> 8;
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_SU));
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_miso :> spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
}

// Protect sector at given address
// Takes byte address not sector number!
// This is because some devices have uneven sector protection layout
static void spiProtect(unsigned addr)
{
  unsigned char spi_null;
  spiWriteEnable();
  addr = bitrev(addr) >> 8;
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_SP));
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_miso :> spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
}


// Erase given sector
// Sector size is SPI_NUM_BYTES_IN_SECTOR
// Note: Sector must be made unprotected first
static void spiErase(int sector)
{
  unsigned char spi_null;
  unsigned addr = bitrev(SPI_NUM_BYTES_IN_SECTOR * sector) >> 8;
  spiWriteEnable();
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_SE));
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_miso :> spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
}

// Unprotect all memory
// Some devices have uneven sector protection layout, so we have #define list
static const unsigned sectors[] =
{
  0x00000, 0x10000, 0x20000, 0x30000, 0x40000, 0x50000,
  0x60000, 0x70000, 0x78000, 0x7A000, 0x7C000
};

static void spiUnprotectAll()
{
  const int nsectors = sizeof(sectors) / sizeof(unsigned);
  for (int i = 0; i < nsectors; i++)
  {
    spuUnprotect(sectors[i]);
  }
}

static void spiUnprotectOne(int i)
{
  spuUnprotect(sectors[i]);
}

// Protect all memory
// Some devices have uneven sector protection layout, so we have a list.
static void spiProtectAll()
{
  const int nsectors = sizeof(sectors) / sizeof(unsigned);
  for (int i = 0; i < nsectors; i++)
  {
    spiProtect(sectors[i]);
  }
}


// Read status register
static void spiStatusReg(unsigned &status_reg_ref)
{
  unsigned status_reg;
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_RDSR));
  p_mosi <: 0;
  p_miso :> unsigned char spi_null;
  p_miso :> status_reg;
  status_reg_ref = byterev(bitrev(status_reg));
  sync(p_mosi);
  stop_clock(b_spi);
}

// Erase/write progress
// Returns 1 if device is still busy with last erase/write
// Will try 3 times with 1us delay between each try
// Typical use: "while(spiPollProgress())"
static int spiPollProgress()
{
  timer tmr;
  unsigned t;
  int busy = 1;
  for (int timeout = 3; busy && timeout >= 0; timeout--)
  {
    unsigned status_reg;
    spiStatusReg(status_reg);
    tmr :> t;
    busy = status_reg & SPI_WIP_BIT_MASK;
    if (busy)
      tmr when timerafter(t + 100) :> t;
  }
  return busy;
}

int atmel_eraseAll(void)
{
	// Erase whole chip
	spiUnprotectAll();
#ifdef MONITOR
  printstr("Erase sectors: \n 0...");
#endif
	for (int i = 0; i < SPI_NUM_SECTORS; i++)
	{
#ifdef MONITOR
    printstr("\r ");
    printint(i);
#endif
		spiErase(i);
		while (spiPollProgress());
	}
#ifdef MONITOR
  printstr("\n");
#endif
	spiProtectAll();
  return(0);
}

int atmel_eraseOne(int i)
{
	// Erase whole chip
  spiUnprotectAll();
#ifdef MONITOR
  printstr("Erase sectors: \n 0...");
#endif
  
  spiErase(i);
  while (spiPollProgress());
  
#ifdef MONITOR
  printstr("\n");
#endif
  spiProtectAll();
  return(0);
}

// Write disable
// Normally don't need to be explicitly called
static void spiWriteDisable()
{
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_WRDI));
  p_miso :> unsigned char spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
}


static void spiInit()
{
  // Output ports start as inputs
	// Turn MOSI around so that it is an output port
  //p_mosi:1 <: 0;
  //sync(p_mosi);

	// SCLK, MOSI and MISO are clocked off generated 50 MHz clock
	// Do not make SCLK a clock-out port yet, we don't want clock now
  set_port_clock(p_sclk, b_spi);
  set_port_clock(p_mosi, b_spi);
  set_port_clock(p_miso, b_spi);

	// MOSI is a strobed master - SS is MOSIs ready out
  set_port_strobed(p_mosi);
  set_port_master(p_mosi);
  set_port_ready_src(p_ss, p_mosi);
  set_port_mode_ready(p_ss);

	// SS is low when MOSI is ready - inversion
	// We need an extra port that has the uninverted value
  set_port_inv(p_ss);
  set_port_ready_src(p_rdy, p_mosi);
  set_port_mode_ready(p_rdy);

	// MISO is a strobed slave - SS is MISOs ready input
  set_port_strobed(p_miso);
  set_port_slave(p_miso);

  // Divider set to 1, i.e. 50 MHz
  set_clock_div(b_spi, 1);

	// Uninverted SS is ready input to MISO
  set_clock_ready_src(b_spi, p_rdy);

  // Below we will need a few clock ticks to allow emptying ports
  start_clock(b_spi);

	// To make sure SS goes high, MOSI must become unready
	// A negative edge is required for that - a sync is sufficient
  sync(p_mosi);
  
  // MISO is not capturing anymore as MOSI is not ready now
  // Remove leftover data
  clearbuf(p_miso);

	// Stop clock block - ports are initialised
	// Clock block will be started and stopped by each SPI flash function call
  stop_clock(b_spi);

  // Now connect clock output port
	// No clock will be output because clock block is stopped
  set_port_mode_clock(p_sclk);

	// Make sure we start with write disabled
  spiWriteDisable();
}


// Read device ID
int atmel_getId(unsigned int& id)
{
  unsigned device_id;

  spiInit();
  
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_RDID));
  p_mosi <: 0;
  p_miso :> unsigned char spi_null;
  p_mosi <: 0;
  p_miso :> >> device_id;
  p_mosi <: 0;
  p_miso :> >> device_id;
  p_mosi <: 0;
  p_miso :> >> device_id;
  p_miso :> >> device_id;
  id = bitrev(device_id);
  sync(p_mosi);
  stop_clock(b_spi);
    printstr("Atmel:");
    printhex(id);
    printstr("\n");
  if( (id&SPI_DEVICE_ID_MASK) == SPI_DEVICE_ID )
  {
    return(1);
  }
  return(0);
}

int atmel_programPage( unsigned int pageAddress, unsigned char pageBuffer[] )
{
  unsigned char spi_null;
  unsigned addr = bitrev(pageAddress)>>8;
  spiWriteEnable();
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_PP));
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  for (int i = 0; i < SPI_BYTES_IN_PAGE; i++)
  {
    p_mosi <: (unsigned char)(bitrev(pageBuffer[i])>>24);
    p_miso :> spi_null;
  }
  p_miso :> spi_null;
  sync(p_mosi);
  stop_clock(b_spi);
  while (spiPollProgress());

  return(0);
}

int atmel_readPage( unsigned int pageAddress, unsigned char pageBuffer[] )
{
  unsigned char tmp;
  unsigned char spi_null;
  unsigned addr = bitrev(pageAddress)>>8;
  start_clock(b_spi);
  p_mosi <: byterev(bitrev(SPI_READ_FAST));
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: >> addr;
  p_miso :> spi_null;
  p_mosi <: 0;
  p_miso :> spi_null;
  p_mosi <: 0;
  p_miso :> spi_null;
  for (int i = 0; i < SPI_BYTES_IN_PAGE- 1; i++)
  {
    p_mosi <: 0;
    p_miso :> tmp;
    pageBuffer[i] = bitrev(tmp)>>24;
  }
  p_miso :> tmp;
  pageBuffer[SPI_BYTES_IN_PAGE - 1] = bitrev(tmp)>>24;
  sync(p_mosi);
  stop_clock(b_spi);
  return(0);
}


void atmel_endSPIFlash(void)
{
}

int atmel_getBytesInPage()
{
  return( SPI_BYTES_IN_PAGE );
}

int atmel_startWrite(void)
{
  spiUnprotectAll();
  return(0);
}

int atmel_endWrite(void)
{
  spiProtectAll();
  return(0);
}


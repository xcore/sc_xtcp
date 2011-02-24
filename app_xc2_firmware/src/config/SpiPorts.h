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


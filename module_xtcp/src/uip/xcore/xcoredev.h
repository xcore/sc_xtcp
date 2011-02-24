#ifndef __XCOREDEV_H__
#define __XCOREDEV_H__

#include <xccompat.h>

void xcoredev_init(chanend mac_rx, chanend mac_tx);
unsigned int xcoredev_read(chanend mac_rx, int n);
void xcoredev_send(chanend mac_tx);

#endif /* __XCOREDEV_H__ */

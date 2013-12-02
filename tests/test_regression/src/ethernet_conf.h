// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

// The maximum ip payload size we can use is 500 bytes, so
// there is no point accepted larger packets, this allows
// us to have more buffers in total
#define MAX_ETHERNET_PACKET_SIZE (1514)

#define NUM_MII_RX_BUF 6
#define NUM_MII_TX_BUF 4

#define MAX_ETHERNET_CLIENTS   (1)





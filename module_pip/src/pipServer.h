// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

void pipServer(clock clk_smi,
               out port ?p_mii_resetn,
               smi_interface_t &smi,
               mii_interface_t &m,
               chanend appIn, chanend appOut, chanend server);
